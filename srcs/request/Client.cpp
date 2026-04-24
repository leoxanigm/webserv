#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>

#include "../../includes/parsing/HTTPRequestParser.hpp"
#include "../../includes/request/Client.hpp"
#include "../../includes/response/HTTPError.hpp"
#include "../../includes/response/ResponseBuilder.hpp"
#include "../../includes/response/HTTPResponse.hpp"
#include "../../includes/utils.hpp"

Client::Client(int connFd, const ConfigFileParsing::server_t &server)
	: _server(server),
	  _lastActivity(utils::nowSeconds()),
	  _connFd(connFd),
	  _pipeReadFd(-1),
	  _pipeWriteFd(-1),
	  _inBuffer(""),
	  _outBuffer(""),
	  _sentSize(0),
	  _state(RECV_REQUEST),
	  _isCGI(false),
	  _requestBodyOffset(0),
	  _cgiRawOutput("")
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

		bool hasContentType = false;
		std::istringstream headerStream(headers);
		std::string line;
		while (std::getline(headerStream, line))
		{
			if (!line.empty() && line[line.size() - 1] == '\r')
				line.erase(line.size() - 1);

			std::string::size_type colon = line.find(':');
			if (colon == std::string::npos)
				continue;

			std::string key = line.substr(0, colon);
			std::string value = line.substr(colon + 1);
			std::string::size_type start = value.find_first_not_of(" \t");

			if (start != std::string::npos)
				value = value.substr(start);

			std::string lkey = key;
			for (std::size_t i = 0; i < lkey.size(); ++i)
				lkey[i] = static_cast<char>(::tolower(static_cast<unsigned char>(lkey[i])));

			if (lkey == "content-length") // we generate content length in response builder
				continue;

			if (lkey == "status")
			{
				int code = std::atoi(value.c_str());
				if (code >= 200 && code <= 505)
					resp.setStatusCode(code);
				continue;
			}

			resp.setHeader(key, value);
			if (lkey == "content-type")
				hasContentType = true;
		}
		if (!hasContentType)
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

void Client::_registerActivity()
{
	_lastActivity = utils::nowSeconds();
}

bool Client::hasTimedOut() const
{
	return ((utils::nowSeconds() - _lastActivity) >= _server.client_timeout);
}

int Client::handleRead(int fd)
{
	if (_state != RECV_REQUEST && _state != READ_PIPE)
		return (0);

	_registerActivity();

	if (_state == RECV_REQUEST && fd == _connFd)
	{
		char buff[4096];
		ssize_t n = recv(fd, buff, sizeof(buff), 0);

		if (n <= 0)
		{
			closePipeReadFd();
			closePipeWriteFd();
			_state = CLOSE_CONNECTION;
			return (-1);
		}

		_inBuffer.append(buff, static_cast<std::size_t>(n));

		if (_inBuffer.size() > static_cast<std::size_t>(_server.max_body_size))
		{
			_inBuffer.clear();

			ResponseBuilder errResp(_req, _server);
			errResp.buildErrorRes(HTTPError::PAYLOAD_TOO_LARGE);

			_outBuffer = errResp.str();
			_state = SEND_RESPONSE;
			return (1);
		}

		_req = HTTPRequest(_inBuffer);

		if (_req.isComplete())
		{
			_inBuffer.clear();

			ResponseBuilder response(_req, _server);

			if (!_req.isValid())
			{
				ResponseBuilder errResp(_req, _server);
				errResp.buildErrorRes(_req.getError());

				_outBuffer = errResp.str();
				_state = SEND_RESPONSE;
				return (1);
			}

			if (response.isCGI())
			{
				
				if (_cgi.startCGI(response.getLocation(), _req))
				{
					_pipeReadFd = _cgi.getReadFd();
					_pipeWriteFd = _cgi.getWriteFd();
					_requestBodyOffset = 0;
					_cgiRawOutput.clear();
					_isCGI = true;

					const std::string &body = _req.getBody();
					if (!body.empty())
						_state = WRITE_PIPE;
					else
						_state = READ_PIPE;
				}
				else
				{
					ResponseBuilder errResp(_req, _server);
					errResp.buildErrorRes(HTTPError::INTERNAL_SERVER_ERROR);

					_outBuffer = errResp.str();
					_state = SEND_RESPONSE;
					return (1);
				}
			}
			else
			{
				_outBuffer = response.str();
				_state = SEND_RESPONSE;
			}
		}
		return (static_cast<int>(n));
	}

	if (_state == READ_PIPE && fd == _pipeReadFd)
	{
		char buff[4096];
		ssize_t n = _cgi.readOutput(buff, sizeof(buff));

		if (n > 0)
		{
			_cgiRawOutput.append(buff, static_cast<std::size_t>(n));

			if (_cgiRawOutput.size() > static_cast<std::size_t>(_server.max_body_size))
			{
				_cgiRawOutput.clear();

				ResponseBuilder errResp(_req, _server);
				errResp.buildErrorRes(HTTPError::INTERNAL_SERVER_ERROR);

				_outBuffer = errResp.str();
				_state = SEND_RESPONSE;
				_cgi.reapChildNonBlocking();
				_isCGI = false;
				return (1);
			}

			return (static_cast<int>(n));
		}

		// CGI done writing
		if (n == 0)
		{
			closePipeReadFd();
			_cgi.reapChildNonBlocking();
			_finalizeCGIResponse();
			return (0);
		}

		_state = CLOSE_CONNECTION;
		return (-1);
	}

	return (0);
}

int Client::handleWrite(int fd)
{
	if (_state != SEND_RESPONSE && _state != WRITE_PIPE)
		return (0);

	_registerActivity();

	if (_state == WRITE_PIPE && fd == _pipeWriteFd)
	{
		ssize_t n = _cgi.writeInput(
			_req.getBody().c_str() + _requestBodyOffset,
			_req.getBody().size() - _requestBodyOffset);

		if (n < 0)
			return (-1);

		_requestBodyOffset += n;

		if (n == 0 || _requestBodyOffset >= _req.getBody().size())
		{
			closePipeWriteFd();
			_state = READ_PIPE;
			return (0);
		}

		return (0);
	}

	if (_state == SEND_RESPONSE && fd == _connFd)
	{
		if (_outBuffer.empty())
		{
			_state = CLOSE_CONNECTION;
			return (-1);
		}

		ssize_t send_res = send(fd, _outBuffer.c_str(), _outBuffer.length(), 0);
		if (send_res <= 0)
			return (-1);

		_outBuffer.erase(0, static_cast<std::size_t>(send_res));
		if (_outBuffer.empty())
		{
			_state = CLOSE_CONNECTION;
			return (-1);
		}

		return (static_cast<int>(send_res));
	}

	return (0);
}

int Client::handleTimeout()
{
	ResponseBuilder errResp(_req, _server);

	if (_isCGI)
	{
		_cgi.terminate();
		_cgi.reapChildNonBlocking();
		errResp.buildErrorRes(HTTPError::GATEWAY_TIMEOUT);
	}
	else
		errResp.buildErrorRes(HTTPError::REQUEST_TIMEOUT);

	_cgi.closeReadFd();
	_cgi.closeWriteFd();

	_outBuffer = errResp.str();
	_state = SEND_RESPONSE;

	return (1);
}

int Client::getConnFd() const { return (_connFd); }
int Client::getPipeReadFd(void) const { return (_pipeReadFd); }
int Client::getPipeWriteFd(void) const { return (_pipeWriteFd); }

void Client::closeConnFd()
{
	if (_connFd > -1)
		close(_connFd);
	_connFd = -1;
}

void Client::closePipeReadFd()
{
	if (_pipeReadFd > -1)
		_cgi.closeReadFd();
	_pipeReadFd = -1;
}

void Client::closePipeWriteFd()
{
	if (_pipeWriteFd > -1)
		_cgi.closeWriteFd();
	_pipeWriteFd = -1;
}

Client::State Client::getState() const { return (_state); }

void Client::setState(State state) { _state = state; }
