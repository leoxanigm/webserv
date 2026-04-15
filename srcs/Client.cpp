#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <cerrno>

#include "../includes/Client.hpp"
#include "../includes/HTTPError.hpp"
#include "../includes/parsing/HTTPRequestParser.hpp"
#include "../includes/ResponseBuilder.hpp"
#include "../includes/HTTPResponse.hpp"

Client::Client(int connFd, const ConfigFileParsing::server_t &server)
	: _server(server), _connFd(connFd), _pipeReadFd(-1), _pipeWriteFd(-1),
	  _inBuffer(""), _outBuffer(""), _sentSize(0), _state(RECV_REQUEST),
	  _isCGI(false), _cgiBodyOffset(0), _cgiRawOutput("")
{
}

Client::~Client()
{
	if (_connFd > -1)
		close(_connFd);
	if (_pipeReadFd > -1)
		close(_pipeReadFd);
	if (_pipeWriteFd > -1)
		close(_pipeWriteFd);
}

void Client::_finalizeCGIResponse()
{
	HTTPResponse resp;
	resp.setStatusCode(HTTPResponse::OK);

	std::string::size_type pos = _cgiRawOutput.find("\r\n\r\n");
	std::size_t sepLen = 4;

	if (pos == std::string::npos)
	{
		pos = _cgiRawOutput.find("\n\n");
		sepLen = 2;
	}

	if (pos != std::string::npos)
	{
		std::string headers = _cgiRawOutput.substr(0, pos);
		std::string body = _cgiRawOutput.substr(pos + sepLen);

		if (headers.find("Content-type:") != std::string::npos ||
			headers.find("Content-Type:") != std::string::npos)
			resp.setHeader("Content-Type", "text/html");
		else
			resp.setHeader("Content-Type", "text/html");

		resp.setBody(body);
	}
	else
	{
		resp.setHeader("Content-Type", "text/html");
		resp.setBody(_cgiRawOutput);
	}

	_outBuffer = resp.toString();
	_state = SEND_RESPONSE;
	_isCGI = false;
}

// How do we handle timeout and content length more than accepted value?
// Also what if body size doesn't match content length value?
int Client::handleRead(int fd)
{
	if (_state != RECV_REQUEST && _state != READ_PIPE)
		return 0;

	if (_state == RECV_REQUEST && fd == _connFd)
	{
		char buff[4096];
		ssize_t n = recv(fd, buff, sizeof(buff), 0);

		if (n <= 0)
		{
			closePipeReadFd();
			closePipeWriteFd();
			_state = CLOSE_CONNECTION;
			return -1;
		}

		_inBuffer.append(buff, static_cast<std::size_t>(n));
		_req = HTTPRequest(_inBuffer);

		if (_req.isComplete())
		{
			_inBuffer.clear();

			ResponseBuilder response(_req, _server);

			if (response.isCGI())
			{
				if (_cgi.startCGI(response.getLocation(), _req))
				{
					_pipeReadFd = _cgi.getReadFd();
					_pipeWriteFd = _cgi.getWriteFd();
					_cgiBodyOffset = 0;
					_cgiRawOutput.clear();
					_isCGI = true;

					const std::string &body = _req.getBody();
					if (!body.empty())
						_state = WRITE_PIPE;
					else // GET request, close read right away for EOF
					{
						_cgi.closeWriteFd();
						_pipeWriteFd = -1;
						_state = READ_PIPE;
					}
				}
				else
				{
					// TODO
					// _outBuffer = _cgi.getErrResponse();
					// _state = SEND_RESPONSE;
					_state = CLOSE_CONNECTION;
					return (-1);
				}
			}
			else
			{
				_outBuffer = response.str();
				_state = SEND_RESPONSE;
			}
		}
		return static_cast<int>(n);
	}

	if (_state == READ_PIPE && fd == _pipeReadFd)
	{
		std::cout << "== handleRead, READ_PIPE" << "\n";

		char buff[4096];
		ssize_t n = _cgi.readOutput(buff, sizeof(buff));

		if (n > 0)
		{
			_cgiRawOutput.append(buff, static_cast<std::size_t>(n));
			return static_cast<int>(n);
		}

		std::cout << _cgiRawOutput << "\n";

		// EOF => CGI done writing
		if (n == 0)
		{
			_cgi.reapChildNonBlocking();
			_finalizeCGIResponse();
			return 0;
		}

		// n < 0
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return 0;

		_state = CLOSE_CONNECTION;
		return -1;
	}

	return 0;
}

int Client::handleWrite(int fd)
{
	if (_state != SEND_RESPONSE && _state != WRITE_PIPE)
		return 0;

	if (_state == WRITE_PIPE && fd == _pipeWriteFd)
	{
		std::cout << "== handleWrite, WRITE_PIPE" << "\n";

		const std::string &body = _req.getBody();
		if (_cgiBodyOffset >= body.size())
		{
			_cgi.closeWriteFd();
			_pipeWriteFd = -1;
			_state = READ_PIPE;
			return 0;
		}

		ssize_t n = _cgi.writeInput(body.c_str() + _cgiBodyOffset, body.size() - _cgiBodyOffset);
		if (n > 0)
		{
			_cgiBodyOffset += static_cast<std::size_t>(n);
			if (_cgiBodyOffset >= body.size())
			{
				_cgi.closeWriteFd();
				_pipeWriteFd = -1;
				_state = READ_PIPE;
			}
			return static_cast<int>(n);
		}

		if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
			return 0;

		_state = CLOSE_CONNECTION;
		return -1;
	}

	if (_state == SEND_RESPONSE && fd == _connFd)
	{
		if (_outBuffer.empty())
		{
			_state = CLOSE_CONNECTION;
			return 0;
		}

		ssize_t send_res = send(fd, _outBuffer.c_str(), _outBuffer.length(), 0);
		if (send_res == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return 0;
			return -1;
		}

		_outBuffer.erase(0, static_cast<std::size_t>(send_res));
		if (_outBuffer.empty())
			_state = CLOSE_CONNECTION;

		return static_cast<int>(send_res);
	}

	return 0;
}

int Client::getConnFd() const { return _connFd; }
int Client::getPipeReadFd(void) const { return _pipeReadFd; }
int Client::getPipeWriteFd(void) const { return _pipeWriteFd; }

void Client::closeConnFd()
{
	if (_connFd > -1)
		close(_connFd);
	_connFd = -1;
}

void Client::closePipeReadFd()
{
	if (_pipeReadFd > -1)
		close(_pipeReadFd);
	_pipeReadFd = -1;
}

void Client::closePipeWriteFd()
{
	if (_pipeWriteFd > -1)
		close(_pipeWriteFd);
	_pipeWriteFd = -1;
}

Client::State Client::getState() const { return _state; }

bool Client::hasActivePipeRead() const
{
	return (_pipeReadFd > -1 && (_state == READ_PIPE));
}

bool Client::hasActivePipeWrite() const
{
	return (_pipeWriteFd > -1 && (_state == WRITE_PIPE));
}