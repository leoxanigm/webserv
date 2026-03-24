#include <unistd.h>
#include <sys/socket.h>
#include <sstream>
#include <iostream>
#include <fstream>

#include "../includes/Client.hpp"
#include "../includes/HTTPError.hpp"

Client::Client(int connFd)
	: _connFd(connFd), _pipeFd(-1)
{
	_inBuffer = "";
	_outBuffer = "";
}

Client::Client(const Client &obj) : _connFd(obj._connFd)
{
	_pipeFd = obj._pipeFd;
	_inBuffer = obj._inBuffer;
	_outBuffer = obj._outBuffer;
	_state = obj._state;
}

Client::~Client()
{
	close(_pipeFd);
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

int Client::connFd() const
{
	return (_connFd);
}

/**
 * If a client object uses CGI, we have to poll read/write to pipe,
 * so we can request the object for the pipe read/write fd
 */
int Client::getPipeFd(void) const
{
	return (_pipeFd);
}

Client::State Client::getState() const
{
	return (_state);
}
