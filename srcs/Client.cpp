#include <unistd.h>
#include <sys/socket.h>
#include <sstream>
#include <iostream>
#include <fstream>

#include "../includes/Client.hpp"
#include "../includes/HTTPError.hpp"

Client::Client(int connFd)
	: _connFd(connFd), _pipeReadFd(-1), _pipeWriteFd(-1)
{
	_inBuffer = "";
	_outBuffer = "";
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

bool Client::operator<(const Client &rhs) const
{
	return (_connFd < rhs._connFd);
}

int Client::handleRead(int fd)
{
	char buff[4096];
	ssize_t n = recv(fd, buff, sizeof(buff), 0);

	if (n > 0)
	{

		_inBuffer.append(buff, n);

		_state = SEND_RESPONSE;

		// 	if (HTTPRequestParser::requestComplete(_inBuffer))
		// 	{
		//		_state = SEND_RESPONSE;

		// 		HTTPRequestParser req(_inBuffer);

		// 		if (_useCGI)
		// 		{
		// 			// Handle CGI
		// 			// set pipeFd
		// 			// _state = SEND_PIPE;
		// 		}
		// 		else
		// 		{
		// 			// Handle static files
		// 		}
		// 	}
	}

	return (n);
}

int Client::handleWrite(int fd)
{
	// TODO:
	// For CGI, set state to READ_PIPE, then check on finish read pipe
	// and set state to SEND_RESPONSE

	std::ifstream file("./www/html/index.html");
	std::ostringstream content;

	content << file.rdbuf();

	std::string body = content.str();
	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n"
			 << "Content-Type: text/html\r\n"
			 << "Content-Length: " << body.length() << "\r\n"
			 << "\r\n"
			 << body;

	std::string arr = response.str();
	int send_res = send(fd, arr.c_str(), arr.length(), 0);
	if (send_res == -1)
		std::cerr << "ERROR: sending response: " << '\n';
	else
		std::cout << "INFO: sent " << send_res << " bytes to socket " << fd << '\n';

	_inBuffer.clear();

	// close(_pipeFd);
	_state = CLOSE_CONNECTION;

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
