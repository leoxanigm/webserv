#include <sstream>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>

#include "../includes/Client.hpp"
#include "../includes/HTTPError.hpp"
#include "../includes/parsing/HTTPRequestParser.hpp"
#include "../includes/ResponseBuilder.hpp"

Client::Client(int connFd, const ConfigFileParsing::location_list_t &locations)
	: _locations(locations), _connFd(connFd)
{
	_inBuffer = "";
	_outBuffer = "";
	_pipeReadFd = -1;
	_pipeWriteFd = -1;
	_state = READ_REQUEST;
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

int Client::handleRead(int fd)
{
	// TODO:
	// loop on recv()
	// stop on EAGAIN / EWOULDBLOCK
	// if recv() returns 0, client sent all data and closed
	// Talk with Mathys on how to implement requestComplete for
	//	* normal body
	//  * form body
	//  * chunked request
	//  * file upload
	// How do we handle timeout and content length more than accepted value?
	// Also what if body size doesn't match content length value?
	// Two CGIs we can use:
	//	* python cgi module (python3.10) (https://docs.python.org/3.10/library/cgi.html)
	//	* php-cgi (https://php-dictionary.readthedocs.io/en/latest/dictionary/cgi.ini.html)

	char buff[4096];
	ssize_t n = 0;

	while ((n = recv(fd, buff, sizeof(buff), 0)) > 0)
	{
		_inBuffer.append(buff, n);

		// buff[n] = '\0';

		// std::cout << "INFO: received buffer:\n" << buff << '\n';

		std::memset(buff, 0, 4096);

		_req = HTTPRequest(_inBuffer);

		if (_req.isValid()) // Received the whole request buffer
		{
			_state = SEND_RESPONSE;

			std::cout << "INFO: received " << n << " bytes from " << fd << '\n';

			// if (_useCGI(req.getLocation()))
			// {
			// 	// Handle CGI
			// 	// set pipeFd
			// 	// _state = SEND_PIPE;
			// }
			// else
			// {
			// 	// Handle static files
			// }
		}
		else
			std::cout << _inBuffer << '\n';
	}

	if (n <= 0) // Connection closed or error
	{
		closePipeReadFd();
		closePipeWriteFd();
		_state = CLOSE_CONNECTION;
	}

	return (n);
}

int Client::handleWrite(int fd)
{
	// TODO:
	// For CGI, set state to READ_PIPE, then check on finish read pipe
	// and set state to SEND_RESPONSE
	// Check send_res length and remove it from begining of _outBuffer
	// when _outBuffer is empty, close all pipe fds and set _state = CLOSE_CONNECTION

	(void) fd; //remove
	
	int send_res = 0;

	if (_req.isValid())
	{
		// std::ifstream file("./www/html/index.html");
		// std::ostringstream content;

		// content << file.rdbuf();


		// std::string test = HTTPResponse::directoryListing(_req);

		// std::string body = content.str();
		// std::ostringstream response;
		// response << "HTTP/1.1 200 OK\r\n"
		// 		 << "Content-Type: text/html\r\n"
		// 		 << "Content-Length: " << test.length() << "\r\n"
		// 		 << "\r\n"
		// 		 << test;

		// std::string arr = response.str();
		// send_res = send(fd, arr.c_str(), arr.length(), 0);
		// if (send_res == -1)
		// 	std::cerr << "ERROR: sending response: " << '\n';
		// else
		// 	std::cout << "INFO: sent " << send_res << " bytes to socket " << fd << '\n';

		// _inBuffer.clear();

		HTTPRequest req;
		ConfigFileParsing config("default.conf");
		ResponseBuilder responseBuilder(req, config);

		// close(_pipeFd);
		_state = CLOSE_CONNECTION;
	}

	return (send_res);
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
