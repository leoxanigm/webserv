#include <sstream>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <errno.h>

#include "../includes/Client.hpp"
#include "../includes/HTTPError.hpp"
#include "../includes/parsing/HTTPRequestParser.hpp"
#include "../includes/ResponseBuilder.hpp"
#include "../includes/cgi.hpp"

#include "../includes/tests.hpp"

Client::Client(int connFd, const ConfigFileParsing::server_t &server)
	: _server(server), _connFd(connFd)
{
	_inBuffer = "";
	_outBuffer = "";
	_pipeReadFd = -1;
	_pipeWriteFd = -1;
	_state = RECV_REQUEST;
	_sentSize = 0;
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

// How do we handle timeout and content length more than accepted value?
// Also what if body size doesn't match content length value?
// Two CGIs we can use:
//	* python cgi module (python3.10) (https://docs.python.org/3.10/library/cgi.html)
//	* php-cgi (https://php-dictionary.readthedocs.io/en/latest/dictionary/cgi.ini.html)
int Client::handleRead(int fd)
{
	if (_state != RECV_REQUEST && _state != READ_PIPE)
		return (0);

	ssize_t n = 0;

	if (_state == RECV_REQUEST)
	{
		char buff[4096];

		n = recv(fd, buff, sizeof(buff), 0);
		_inBuffer.append(buff, n);
		_req = HTTPRequest(_inBuffer);
		// display_parsed_request(_req);

		if (_req.isComplete())
		{
			_inBuffer.clear();
			_state = SEND_RESPONSE;

			ResponseBuilder response(_req, _server);

			// TODO
			// If cgi, get read/write pipe fds here

			if (!response.isCGI())
				_outBuffer = response.str();
		}

		if (n <= 0) // Connection closed or error
		{
			closePipeReadFd();
			closePipeWriteFd();
			_state = CLOSE_CONNECTION;
		}
	}

	return (n);
}

int Client::handleWrite(int fd)
{
	if (_state != SEND_RESPONSE && _state != WRITE_PIPE)
		return (0);

	if (_outBuffer.empty())
		return (0);

	// TODO:
	// For CGI, set state to READ_PIPE, then check on finish read pipe
	// and set state to SEND_RESPONSE
	// Check send_res length and remove it from begining of _outBuffer
	// when _outBuffer is empty, close all pipe fds and set _state = CLOSE_CONNECTION

	ssize_t send_res = 0;

	send_res = send(fd, _outBuffer.c_str(), _outBuffer.length(), 0);
	if (send_res == -1)
		return (-1);

	_outBuffer.erase(0, static_cast<std::size_t>(send_res));

	std::cout << "INFO: sent " << send_res << " bytes to socket " << fd << '\n';

	// close(_pipeFd);
	if (_outBuffer.empty())
		_state = CLOSE_CONNECTION;

	return (1);
}

int Client::getConnFd() const
{
	return (_connFd);
}

int Client::getPipeReadFd(void) const
{
	return (_pipeReadFd);
}

int Client::getPipeWriteFd(void) const
{
	return (_pipeWriteFd);
}

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

Client::State Client::getState() const
{
	return (_state);
}
