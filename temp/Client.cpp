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

const ConfigFileParsing::location_t *
Client::_longestPrefixLocationMatch(const std::string &reqURI) const
{
	const ConfigFileParsing::location_t *location = 0;
	std::size_t locationLen = 0;

	ConfigFileParsing::location_iterator_t it = _server.locations.begin();
	for (; it != _server.locations.end(); ++it)
	{
		const std::string &loc = it->location;
		std::size_t len = loc.size();

		if (reqURI.compare(0, len, loc) != 0)
			continue;

		if (loc == "/" && !reqURI.empty() && reqURI[0] == '/' && len > locationLen)
		{
			location = &(*it);
			locationLen = len;
			continue;
		}

		bool exactMatch = (reqURI.size() == len);
		bool boundaryMatch = (!exactMatch && reqURI[len] == '/');

		if ((exactMatch || boundaryMatch) && len > locationLen)
		{
			location = &(*it);
			locationLen = len;
		}
	}
	return location;
}

std::string Client::_stripQueryString(const std::string &uri) const
{
	std::string::size_type q = uri.find('?');
	if (q == std::string::npos)
		return uri;
	return uri.substr(0, q);
}

std::string Client::_extractQueryString(const std::string &uri) const
{
	std::string::size_type q = uri.find('?');
	if (q == std::string::npos)
		return "";
	return uri.substr(q + 1);
}

std::string Client::_resolvePath(const std::string &reqURI, const ConfigFileParsing::location_t *location) const
{
	std::string suffix = reqURI.substr(location->location.size());
	if (!suffix.empty() && suffix[0] != '/')
		suffix.insert(suffix.begin(), '/');

	if (!location->root.empty() &&
		location->root[location->root.size() - 1] == '/' &&
		!suffix.empty() && suffix[0] == '/')
		return (location->root + suffix.substr(1));

	if (!location->root.empty() &&
		location->root[location->root.size() - 1] != '/' &&
		!suffix.empty() && suffix[0] != '/')
		return (location->root + "/" + suffix);

	return (location->root + suffix);
}

std::string Client::_requestMediaType() const
{
	const std::map<std::string, std::string> &ct = _req.getContentType();
	std::map<std::string, std::string>::const_iterator it = ct.begin();
	for (; it != ct.end(); ++it)
	{
		if (it->first.find('/') != std::string::npos)
			return it->first;
	}
	return "";
}

std::string Client::_methodToString() const
{
	switch (_req.getMethod())
	{
		case GET: return "GET";
		case POST: return "POST";
		case DELETE: return "DELETE";
	}
	return "GET";
}

bool Client::_startCGI(const ConfigFileParsing::location_t *location)
{
	std::string requestPath = _stripQueryString(_req.getRequestTarget());
	std::string scriptPath = _resolvePath(requestPath, location);

	std::vector<std::string> argv;
	argv.push_back(location->cgi_location); // interpreter path from config
	argv.push_back(scriptPath);

	std::map<std::string, std::string> env;
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["REQUEST_METHOD"] = _methodToString();
	env["SCRIPT_NAME"] = requestPath;
	env["QUERY_STRING"] = _extractQueryString(_req.getRequestTarget());
	env["SERVER_PROTOCOL"] = _req.getHTTPVersion();
	env["CONTENT_LENGTH"] = (_req.getContentLength() > 0) ? static_cast<std::ostringstream&>(std::ostringstream() << _req.getContentLength()).str() : "";
	env["CONTENT_TYPE"] = _requestMediaType();

	// Useful extras
	env["REDIRECT_STATUS"] = "200";
	env["REQUEST_URI"] = _req.getRequestTarget();

	if (!_cgi.init(argv, env, location->root))
		return false;

	_pipeReadFd = _cgi.getReadFd();
	_pipeWriteFd = _cgi.getWriteFd();
	_cgiBodyOffset = 0;
	_cgiRawOutput.clear();
	_isCGI = true;

	const std::string &body = _req.getBody();
	if (!body.empty())
		_state = WRITE_PIPE;
	else
	{
		_cgi.closeWriteFd();
		_pipeWriteFd = -1;
		_state = READ_PIPE;
	}
	return true;
}

void Client::_finalizeCGIResponse()
{
	// Minimal: send CGI stdout as body (can be replaced by full CGI-header parser later)
	HTTPResponse resp;
	resp.setStatusCode(HTTPResponse::OK);
	resp.setHeader("Content-Type", "text/plain");
	resp.setBody(_cgiRawOutput);
	_outBuffer = resp.toString();

	_state = SEND_RESPONSE;
	_isCGI = false;
}

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

			const ConfigFileParsing::location_t *location =
				_longestPrefixLocationMatch(_req.getRequestTarget());

			if (location && location->is_cgi)
			{
				if (!_startCGI(location))
				{
					_state = CLOSE_CONNECTION;
					return -1;
				}
			}
			else
			{
				ResponseBuilder response(_req, _server);
				_outBuffer = response.str();
				_state = SEND_RESPONSE;
			}
		}
		return static_cast<int>(n);
	}

	if (_state == READ_PIPE && fd == _pipeReadFd)
	{
		char buff[4096];
		ssize_t n = _cgi.readOutput(buff, sizeof(buff));

		if (n > 0)
		{
			_cgiRawOutput.append(buff, static_cast<std::size_t>(n));
			return static_cast<int>(n);
		}

		// EOF => CGI done writing
		if (n == 0)
		{
			_cgi.closeReadFd();
			_pipeReadFd = -1;
			_cgi.pollChild(); // non-blocking reap
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