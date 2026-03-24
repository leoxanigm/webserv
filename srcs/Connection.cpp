#include <vector>
#include <list>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#include "../includes/utils.hpp"
#include "../includes/Connection.hpp"
#include "../includes/Client.hpp"
#include "../includes/HTTPError.hpp"

Connection::Connection(const ConfigFileParsing &config) : _config(config)
{
	_initSockets();
	_initPollFds();

	_sigint = false;
}

Connection::~Connection()
{
	std::vector<struct pollfd>::iterator it;
	for (it = _pollFds.begin(); it != _pollFds.end(); it++)
	{
		if (it->fd >= 0)
			close(it->fd);
	}
}

void Connection::_initSockets()
{
	// TODO: abstract server implementation and use ConfigFileParsing::const_iterator
	std::list<t_servers>::const_iterator it = _config.servers().begin();
	int count = 0;

	for (; it != _config.servers().end(); it++)
	{
		try
		{
			Socket socket(it->interface.c_str(), it->listen_port.c_str());
			_sockets.insert(std::pair<int, locations_list_t &>(socket.fd(), it->locations));
			count++;
		}
		catch (const std::exception &e)
		{
			std::cerr << "Error: starting socket on "
					  << it->interface << ":" << it->listen_port
					  << e.what() << '\n';
		}
	}

	if (count == 0)
		throw HTTPError(HTTPError::INTERNAL_SERVER_ERROR);
}

void Connection::_initPollFds()
{
	std::map<int, locations_list_t &>::const_iterator it = _sockets.begin();

	for (; it != _sockets.end(); it++)
		_addPollFd(it->first, POLLIN);
}

void Connection::_runPoll()
{
	while (!_sigint) // run until SIGINT
	{
		int pollCount = poll(&_pollFds[0], _pollFds.size(), -1);

		if (pollCount == -1)
			throw HTTPError(HTTPError::INTERNAL_SERVER_ERROR);

		_processConnections();
	}
}

void Connection::openConnection(void)
{
	std::cout << "webserv: waiting for connections.. \n";

	_runPoll();
}

void Connection::closeConnection()
{
	_sigint = true;
}

void Connection::_addClient(int fd)
{
	_clients.push_back(Client(fd));
	_clientFds[fd] = &_clients.back();
}

void Connection::_addPollFd(int fd, short event)
{
	struct pollfd newPFd;
	newPFd.fd = fd;
	newPFd.events = event;
	newPFd.revents = 0;
	_pollFds.push_back(newPFd);
}

void Connection::_setFdPollEvent(int fd, short event)
{
	std::vector<struct pollfd>::iterator it = _pollFds.begin();

	for(; it != _pollFds.end(); it++)
	{
		if (fd == it->fd)
		{
			it->events = event;
			break;
		}
	}
}

void Connection::_removeClient(int fd)
{
	std::map<int, Client *>::iterator it = _clientFds.find(fd);
	if (it == _clientFds.end())
	{
		std::cerr << "ERROR: non existant client: in _removeClient(" << fd << ")\n";
		return;
	}

	Client *client = it->second;
	_clientFds.erase(it);

	if (client->connFd() == fd) // Connection with client closed
	{
		std::list<Client>::iterator lIt = _clients.begin();
		for (; lIt != _clients.end(); lIt++)
		{
			if (&(*lIt) == client)
			{
				_clients.erase(lIt);
				break;
			}
		}
	}
}

/**
 * Close the fd at index and remove the pollfd struct from _pollFds
 */
void Connection::_removePollFd(std::size_t index)
{
	_removeClient(_pollFds[index].fd);

	close(_pollFds[index].fd);
	_pollFds[index] = _pollFds[_pollFds.size() - 1];
	_pollFds.pop_back();
}

void Connection::_processConnections()
{
	int eventFd;
	int status = 0;

	for (std::size_t i = 0; i < _pollFds.size();)
	{
		eventFd = _pollFds[i].fd;
		status = 0;

		// Ready to receive
		if (_pollFds[i].revents & POLLIN)
		{
			if (_sockets.count(eventFd) > 0) // New connection
				_handleNewConnection(eventFd);
			else
				status = _handleClient(eventFd, POLLIN);
		}

		// Ready to send
		if (_pollFds[i].revents & POLLOUT)
			status = _handleClient(eventFd, POLLOUT);

		// Client closed or connection error
		if ((_pollFds[i].revents & (POLLHUP | POLLERR)) || status == -1)
		{
			_removePollFd(i);
			continue;
		}

		i++;
	}
}

void Connection::_handleNewConnection(int socketFd)
{
	struct sockaddr_storage remoteAddr;
	socklen_t addrLen;
	int clientFd;

	addrLen = sizeof(remoteAddr);

	errno = 0;
	clientFd = accept(socketFd, (struct sockaddr *)&remoteAddr, &addrLen);

	if (clientFd == -1 || utils::setNonBlocking(clientFd) == -1)
	{
		std::cerr << "ERROR: accepting new client: " << std::strerror(errno) << '\n';
		return;
	}

	_addPollFd(clientFd, POLLIN);
	_addClient(clientFd);

	std::cout << "INFO: new connection on socket " << socketFd << '\n';
	// Maybe print more info using remoteIP char array?
	// Or create a Logger which will log output info, error, warning to file
}

int Connection::_handleClient(int fd, short event)
{
	if (_clientFds.count(fd) == 0)
		return (-1);

	Client *client = _clientFds.at(fd);
	int status = 0;

	if (event & POLLIN)
		status = client->handleRead(fd);

	if (event & POLLOUT)
		status = client->handleWrite(fd);

	if (client->getState() == Client::SEND_RESPONSE)
		_setFdPollEvent(fd, POLLOUT);

	if (client->getState() == Client::READ_PIPE)
		_setFdPollEvent(client->getPipeFd(), POLLIN);

	if (client->getState() == Client::SEND_PIPE)
		_setFdPollEvent(client->getPipeFd(), POLLOUT);

	if (client->getState() == Client::CLOSE_CONNECTION)
		status = -1;

	return (status);
}


// TODO
// make _signal static and handle SIGINT
