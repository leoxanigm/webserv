#include <stdexcept>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include "../includes/Socket.hpp"
#include "../includes/parsing/HTTPRequestParser.hpp"

/**
 * Initializes a socket listener.
 * Throws on error.
 */
Socket::Socket()
{
	// Initialize listener on start
	struct pollfd pFd;
	pFd.fd = _getListenerSocket();
	pFd.events = POLLIN;
	pFd.revents = 0;

	_pollFds.push_back(pFd);

	std::cout << "webserv: waiting for connections...\n";

	_runPoll(pFd.fd);
}

Socket::Socket(const Socket &obj)
{
	_pollFds = obj._pollFds;
}

Socket &Socket::operator=(const Socket &obj)
{
	if (this != &obj)
		_pollFds = obj._pollFds;

	return *this;
}

Socket::~Socket()
{
	std::vector<struct pollfd>::iterator it;
	for (it = _pollFds.begin(); it != _pollFds.end(); it++)
		close(it->fd);
}

int Socket::_getListenerSocket(void) const
{
	int listener;
	int optVal = 1;
	int retVal;

	struct addrinfo addrInfoHints, *addrInfo, *addrInfoIt;

	std::memset(&addrInfoHints, 0, sizeof(addrInfoHints));
	addrInfoHints.ai_family = AF_UNSPEC; // ipv4 or ipv6
	addrInfoHints.ai_socktype = SOCK_STREAM;
	addrInfoHints.ai_flags = AI_PASSIVE; // get ip for me

	if ((retVal = getaddrinfo(0, "9999", &addrInfoHints, &addrInfo)) != 0)
		throw std::runtime_error(std::string(gai_strerror(retVal)));

	for (addrInfoIt = addrInfo; addrInfoIt != 0; addrInfoIt = addrInfoIt->ai_next)
	{
		listener = socket(
			addrInfoIt->ai_family,
			addrInfoIt->ai_socktype,
			addrInfoIt->ai_protocol);

		if (listener < 0)
			continue;

		// Allow bind without wait
		// https://stackoverflow.com/a/3229926
		if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) != 0)
			continue;

		if (bind(listener, addrInfo->ai_addr, addrInfo->ai_addrlen) != 0)
		{
			close(listener);
			continue;
		}

		break; // We were able to bind
	}

	if (addrInfoIt == 0)
	{
		close(listener);
		throw std::runtime_error("Error: unable to bind to a socket.");
	}

	freeaddrinfo(addrInfo);

	if (listen(listener, SOMAXCONN) != 0)
	{
		close(listener);
		throw std::runtime_error("Error: unable to listen on a socket.");
	}

	return (listener);
}

void Socket::_runPoll(int listener)
{
	while (true)
	{
		errno = 0;

		int pollCount = poll(&_pollFds[0], _pollFds.size(), -1);

		if (pollCount == -1)
			throw std::runtime_error(std::string(strerror(errno)));

		_processConnections(listener);
	}
}

void Socket::_processConnections(int listener)
{
	for (std::size_t i = 0; i < _pollFds.size(); i++)
	{
		// Someone ready to read or hangup
		if (_pollFds[i].revents & (POLLIN | POLLHUP))
		{
			if (_pollFds[i].fd == listener) // New connection
				_handleNewConnection(listener);
			else
				_handleClient(i);
		}
	}
}

void Socket::_handleNewConnection(int listener)
{
	struct sockaddr_storage remoteAddr;
	socklen_t addrLen;
	int newFd;
	// char remoteIP[INET6_ADDRSTRLEN];

	addrLen = sizeof(remoteAddr);

	errno = 0;
	newFd = accept(listener, (struct sockaddr *)&remoteAddr, &addrLen);

	if (newFd == -1)
		std::cerr << "ERROR: accepting new client: " << strerror(errno) << '\n';
	else
	{
		struct pollfd newPFd;
		newPFd.fd = newFd;
		newPFd.events = POLLIN;
		newPFd.revents = 0;
		_pollFds.push_back(newPFd);

		std::cout << "INFO: new connection on socket " << newFd << '\n';
		// Maybe print more info using remoteIP char array?
	}
}

// For testing, this should call a parser
void Socket::_handleClient(std::size_t index)
{
	char buffer[256];
	int senderFd = _pollFds[index].fd;

	errno = 0;
	int nBytes = recv(senderFd, buffer, sizeof(buffer) - 1, 0);

	if (nBytes <= 0)
	{
		if (nBytes == 0) // connection closed
			std::cout << "INFO: socket " << senderFd << " hang up\n";
		else
			std::cerr << "ERROR: receiving data: " << strerror(errno) << '\n';

		close(senderFd);

		// Remove client
		_pollFds[index] = _pollFds[_pollFds.size() - 1];
		_pollFds.pop_back();
	}
	else
	{
		buffer[nBytes] = '\0';
		
		std::string rawRequest(buffer, nBytes);
		HTTPRequest request(rawRequest);

		/*
		Example for testing :
		GET / HTTP/1.1\r\nHost: localhost:9999\r\n
		*/
		if (!request.isValid())
			std::cerr << "WARN: failed to parse HTTP request from socket "
					<< senderFd << ": " << request.getError() << '\n';
		else
			std::cout << "INFO: received from " << senderFd
					<< ": " << nBytes << " bytes. \n";

		/*
		* TESTS

		Here we have an `index.html` file to test a static webpage
		without taking in account the header yet
		*/
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
		int send_res = send(senderFd, arr.c_str(), arr.length(), 0);
		if (send_res == -1)
			std::cerr << "ERROR: sending response: " << strerror(errno) << '\n';
		else
			std::cout << "INFO: sent " << send_res << " bytes to socket " << senderFd << '\n';
	}	
}
