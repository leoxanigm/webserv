#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#include "../includes/utils.hpp"
#include "../includes/Connection.hpp"
#include "../includes/Client.hpp"
#include "../includes/HTTPError.hpp"

Connection::Connection() : _socket("localhost", "8080") // Remove
{
	struct pollfd pFd;
	pFd.fd = _socket.fd();
	pFd.events = POLLIN;
	pFd.revents = 0;

	_pollFds.push_back(pFd);

	_sigint = false;
}

// TODO:
// init with config
// Connection::Connection(const Config &config) : _config(config)
// {
// }

Connection::~Connection()
{
	std::vector<struct pollfd>::iterator it;
	for (it = _pollFds.begin(); it != _pollFds.end(); it++)
	{
		if (it->fd >= 0)
			close(it->fd);
	}
}

void Connection::openConnection(void)
{
	std::cout << "webserv: waiting for connections for "
			  << _socket.host() << ":"
			  << _socket.service() << " ...\n";

	_runPoll();
}

void Connection::closeConnection()
{
	_sigint = true;
}

void Connection::_runPoll()
{
	while (!_sigint) // run until SIGINT
	{
		errno = 0;
		int pollCount = poll(&_pollFds[0], _pollFds.size(), -1);

		if (pollCount == -1)
			throw HTTPError(HTTPError::Code::INTERNAL_SERVER_ERROR);

		_processConnections();
	}
}

void Connection::_processConnections()
{
	for (std::size_t i = 0; i < _pollFds.size(); i++)
	{
		if (_pollFds[i].revents & (POLLIN | POLLOUT | POLLHUP))
		{
			if ((_pollFds[i].revents & POLLIN) && (_pollFds[i].fd == _socket.fd()))
				_handleNewConnection();
			else
				_handleClient(i, _pollFds[i].revents);
		}
	}
}

void Connection::_handleNewConnection()
{
	struct sockaddr_storage remoteAddr;
	socklen_t addrLen;
	int newFd;
	// char remoteIP[INET6_ADDRSTRLEN];

	addrLen = sizeof(remoteAddr);

	errno = 0;
	newFd = accept(_socket.fd(), (struct sockaddr *)&remoteAddr, &addrLen);

	if (newFd == -1 || utils::setNonBlocking(newFd) == -1)
	{
		std::cerr << "ERROR: accepting new client: " << std::strerror(errno) << '\n';
		return;
	}

	// TODO:
	// add new client to _fdClientConn map
	// wait POLLIN for the new client in poll in
	// call client.handleRead() and then request pipe fd with getPipeFd();
	// if there is a pipeFd, add it to _pollFds and _fdClientPipe.

	// On events for POLLIN, POLLOUT, POLLHUP, POLLERR, call appropriate
	// client method, passing the revent fd.
	// I think that's all Connection needs to do, Client should handle
	// request and response

	struct pollfd newPFd;
	newPFd.fd = newFd;
	newPFd.events = POLLIN;
	newPFd.revents = 0;
	_pollFds.push_back(newPFd);

	std::cout << "INFO: new connection on socket " << newFd << '\n';
	// Maybe print more info using remoteIP char array?
	// Or create a Logger which will log output info, error, warning to file
}

void Connection::_handleClient(std::size_t index, short event)
{
	// TODO:
	// Change function in accordance with the comments above

	// int senderFd = _pollFds[index].fd;
	// int status = 0;

	// // if (event & (POLLIN | POLLOUT))
	// if (event & POLLIN)
	// {
	// 	if (_events.count(event) == 1)
	// 		status = _events[event](senderFd);
	// 	else
	// 		std::cerr << "Error: _handleClient: callback not found: " << event << '\n';
	// }

	// if ((event & POLLHUP) || status == -1)
	// {
	// 	close(senderFd);

	// 	// Remove client
	// 	_pollFds[index] = _pollFds[_pollFds.size() - 1];
	// 	_pollFds.pop_back();
	// }
}
