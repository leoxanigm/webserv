#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#include "../includes/utils.hpp"
#include "../includes/Connection.hpp"

Connection::Connection() : _socket("localhost", "8080") // Remove
{
	struct pollfd pFd;
	pFd.fd = _socket.fd();
	pFd.events = POLLIN;
	pFd.revents = 0;

	_pollFds.push_back(pFd);

	_initEvents();
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

void Connection::closeConnection(int sig)
{
	(void)sig;
	_sigint = true;
}

/**
 * Attach a callback function for POLLIN, POLLOUT poll events.
 * Function will take file descriptor as argument and should return
 * a status 0 on success or -1 on failure.
 * In case of failure, the connection will close the file descriptor.
 */
void Connection::attachEvent(short event, int (*callback)(int))
{
	if (event & (POLLIN | POLLOUT))
		_events[event] = callback;
}

/**
 * Initialize default events for connections if none area attached
 */
void Connection::_initEvents(void)
{
	// TODO:
	// Read config and set default events

	std::cout << "INFO: no default events attached\n"; // Remove
}

void Connection::_runPoll()
{
	while (!_sigint) // run until SIGINT
	{
		errno = 0;
		int pollCount = poll(&_pollFds[0], _pollFds.size(), -1);

		if (pollCount == -1)
			throw std::runtime_error("Error: " + std::string(std::strerror(errno)));

		_processConnections();
	}
}

void Connection::_processConnections()
{
	for (std::size_t i = 0; i < _pollFds.size(); i++)
	{
		if (_pollFds[i].revents & (POLLIN | POLLOUT | POLLHUP))
		{
			if (_pollFds[i].fd == _socket.fd())
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
	int senderFd = _pollFds[index].fd;
	int status = 0;

	// if (event & (POLLIN | POLLOUT))
	if (event & POLLIN)
	{
		if (_events.count(event) == 1)
			status = _events[event](senderFd);
		else
			std::cerr << "Error: _handleClient: callback not found: " << event << '\n';
	}

	if ((event & POLLHUP) || status == -1)
	{
		close(senderFd);

		// Remove client
		_pollFds[index] = _pollFds[_pollFds.size() - 1];
		_pollFds.pop_back();
	}
}
