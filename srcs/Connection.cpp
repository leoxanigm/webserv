#include <vector>
#include <list>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <csignal>

#include "../includes/utils.hpp"
#include "../includes/Connection.hpp"
#include "../includes/Client.hpp"
#include "../includes/HTTPError.hpp"

extern volatile sig_atomic_t pollStop;

Connection::Connection(const ConfigFileParsing &config) : _config(config)
{
	_initSockets();
}

Connection::~Connection()
{
	// Remove remaining clients
	while (!_clients.empty())
	{
		delete _clients.back();
		_clients.pop_back();
	}

	// Redundant fd cleanup
	std::vector<struct pollfd>::iterator it;
	for (it = _pollFds.begin(); it != _pollFds.end(); it++)
	{
		if (it->fd > -1)
			close(it->fd);
	}
}

void Connection::start()
{
	std::cout << "webserv: waiting for connections.. \n";

	_runPoll();
}

void Connection::_initSockets()
{
	// TODO: abstract server implementation and use ConfigFileParsing::const_iterator
	std::list<t_servers>::const_iterator it = _config.getServers().begin();
	int count = 0;
	int socketFd = -1;

	for (; it != _config.getServers().end(); it++)
	{
		try
		{
			socketFd = Socket::newSocket(it->listen_interface.c_str(), it->listen_port.c_str());
			_sockets[socketFd] = &(it->locations);
			_addPollFd(socketFd, POLLIN);
			count++;
		}
		catch (const std::exception &e)
		{
			std::cerr << "Error: starting socket on "
					  << it->listen_interface << ":" << it->listen_port
					  << e.what() << '\n';
		}
	}

	if (count == 0)
		throw HTTPError(HTTPError::INTERNAL_SERVER_ERROR);
}

void Connection::_runPoll()
{
	while (!pollStop && _pollFds.size() > 0) // run until SIGINT
	{
		int pollCount = poll(&_pollFds[0], _pollFds.size(), -1);

		if (pollCount < 0)
		{
			if (errno == EINTR) // SIGINT
				continue;
			throw HTTPError(HTTPError::INTERNAL_SERVER_ERROR);
		}

		if (pollCount > 0)
			_processConnections();
	}
}

void Connection::_setFdPollEvent(int fd, short event)
{
	std::vector<struct pollfd>::iterator it = _pollFds.begin();

	for (; it != _pollFds.end(); it++)
	{
		if (fd == it->fd)
		{
			it->events = event;
			break;
		}
	}
}

void Connection::_addPollFd(int fd, short event)
{
	struct pollfd newPFd;
	newPFd.fd = fd;
	newPFd.events = event;
	newPFd.revents = 0;
	_pollFds.push_back(newPFd);
}

void Connection::_addClient(int fd, const ConfigFileParsing::location_list_t *locations)
{
	Client *client = new Client(fd, *locations);
	_clients.push_back(client); // Client should be initialised with t_locations

	FdEntry entry;
	entry.type = FD_CLIENT_CONN;
	entry.client = client;
	_clientFds[fd] = entry;
}

Client *Connection::_findClient(int fd) const
{
	std::map<int, FdEntry>::const_iterator it = _clientFds.find(fd);
	if (it == _clientFds.end())
		return (0);

	return (it->second.client);
}

void Connection::_closeClientFd(Client *client, FdType type)
{
	if (type == FD_CLIENT_CONN)
		client->closeConnFd();
	if (type == FD_PIPE_READ)
		client->closePipeReadFd();
	if (type == FD_PIPE_WRITE)
		client->closeConnFd();
}

void Connection::_removeClient(int fd)
{
	std::map<int, FdEntry>::iterator it = _clientFds.find(fd);
	if (it == _clientFds.end())
		return;

	Client *client = it->second.client;
	FdType type = it->second.type;

	_closeClientFd(client, type);
	_clientFds.erase(it);

	// Connection closed, if for some reason pipe fds are not close, close them
	// now. poll will return POLLERR later and pollfd will be cleaned up on
	// next iteration.
	if (type == FD_CLIENT_CONN)
	{
		_closeClientFd(client, FD_PIPE_READ);
		_closeClientFd(client, FD_PIPE_WRITE);

		_clients.remove(client);
		delete client;
	}
}

/**
 * Close the fd at index and remove the pollfd struct from _pollFds.
 * Changes _pollFds's size
 */
void Connection::_removePollFd(std::size_t index)
{
	_removeClient(_pollFds[index].fd);

	_pollFds[index] = _pollFds[_pollFds.size() - 1];
	_pollFds.pop_back();
}

void Connection::_processConnections()
{
	int fd;
	short revents;
	bool removeClient;
	Client *client;

	for (std::size_t i = 0; i < _pollFds.size();)
	{
		fd = _pollFds[i].fd;
		revents = _pollFds[i].revents;
		removeClient = false;
		client = 0;

		if (revents & POLLERR)
			removeClient = true;
		else
		{
			if (revents & POLLIN)
			{
				if (_sockets.find(fd) != _sockets.end()) // New connection
					_handleNewConnection(fd);
				else if (_handleClient(fd, POLLIN) == -1)
					removeClient = true;
			}

			if (!removeClient && (revents & POLLOUT))
			{
				if (_handleClient(fd, POLLOUT) == -1)
					removeClient = true;
			}

			if (!removeClient && (revents & POLLHUP))
			{
				client = _findClient(fd);
				if (client && client->getState() == Client::CLOSE_CONNECTION)
					removeClient = true;
			}
		}

		if (removeClient)
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

	_addPollFd(clientFd, POLLIN | POLLOUT);
	_addClient(clientFd, _sockets[socketFd]);
	// _addClient(clientFd);

	std::cout << "INFO: new connection on socket " << socketFd << '\n';
	// Maybe print more info using remoteIP char array?
	// Or create a Logger which will log output info, error, warning to file
}

int Connection::_handleClient(int fd, short event)
{
	Client *client = _findClient(fd);
	if (!client)
		return (-1);

	int status = 0;

	if (event & POLLIN)
		status = client->handleRead(fd);

	if (event & POLLOUT)
		status = client->handleWrite(fd);

	if (client->getState() == Client::READ_PIPE)
		_setFdPollEvent(client->getPipeReadFd(), POLLIN);

	if (client->getState() == Client::SEND_PIPE)
		_setFdPollEvent(client->getPipeWriteFd(), POLLOUT);

	if (client->getState() == Client::CLOSE_CONNECTION)
		status = -1;

	return (status);
}
