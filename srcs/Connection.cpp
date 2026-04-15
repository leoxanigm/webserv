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
#include "../includes/UI.hpp"

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


	// Not sure about this but I think it could show a more complete webserv
	// But I think it could be improved both on the style and the way the datas are used to be dislayed
	std::list<t_servers>			srvs = _config.getServers();
	std::list<t_servers>::iterator	it;

	for (it = srvs.begin(); it != srvs.end(); ++it)
	{
		std::cout << YELLOW << "Server " << UNDERLINE_YELLOW << it->server_name << YELLOW << " avalaible at:" << std::endl;

		std::list<t_locations>	locs = it->locations;
		std::list<t_locations>::iterator it2;
		for (it2 = locs.begin(); it2 != locs.end(); ++it2)
		{
			std::cout << UNDERLINE_YELLOW << "\thttp://" << it->server_name << ":" << it->listen_port << it2->location << std::endl;
		}
		std::cout << RESET << std::endl;
	}

	_runPoll();
}

void Connection::_initSockets()
{
	ConfigFileParsing::servers_iterator_t it = _config.getServers().begin();
	int count = 0;
	int socketFd = -1;

	for (; it != _config.getServers().end(); it++)
	{
		try
		{
			socketFd = Socket::newSocket(it->listen_interface.c_str(), it->listen_port.c_str());
			_sockets[socketFd] = &(*it);
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

void Connection::_addClient(int fd, const ConfigFileParsing::server_t *server)
{
	Client *client = new Client(fd, *server);
	_clients.push_back(client);

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
		client->closePipeWriteFd();
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

void Connection::_removePollFdByFd(int fd)
{
	for (std::size_t i = 0; i < _pollFds.size(); ++i)
	{
		if (_pollFds[i].fd == fd)
		{
			_removePollFd(i);
			return;
		}
	}
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
		{
			// std::cout << "POLLERR" << '\n';
			removeClient = true;
		}
		else
		{
			if (revents & POLLIN)
			{
				// std::cout << "POLLIN" << '\n';
				if (_sockets.find(fd) != _sockets.end()) // New connection
					_handleNewConnection(fd);
				else if (_handleClient(fd, POLLIN) == -1)
					removeClient = true;
			}

			if (!removeClient && (revents & POLLOUT))
			{
				// std::cout << "POLLOUT" << '\n';
				if (_handleClient(fd, POLLOUT) == -1)
					removeClient = true;
			}

			if (!removeClient && (revents & POLLHUP))
			{
				// std::cout << "POLLHUP" << '\n';
				client = _findClient(fd);
				if (client && client->getState() == Client::CLOSE_CONNECTION)
					removeClient = true;
			}
		}

		if (removeClient)
		{
			// std::cout << "removeClient" << '\n';
			_removePollFd(i);
			continue; // Don't skip the current index on removal
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

	std::cout << "INFO: new connection on socket " << socketFd << '\n';
	// Maybe print more info using remoteIP char array?
	// Or create a Logger which will log output info, error, warning to file
}

// int Connection::_handleClient(int fd, short event)
// {
// 	Client *client = _findClient(fd);
// 	if (!client)
// 		return (-1);

// 	int status = 0;

// 	if (event & POLLIN)
// 		status = client->handleRead(fd);

// 	if (event & POLLOUT)
// 		status = client->handleWrite(fd);

// 	if (client->getState() == Client::READ_PIPE)
// 		_setFdPollEvent(client->getPipeReadFd(), POLLIN);

// 	if (client->getState() == Client::WRITE_PIPE)
// 		_setFdPollEvent(client->getPipeWriteFd(), POLLOUT);

// 	if (client->getState() == Client::CLOSE_CONNECTION)
// 		status = -1;

// 	return (status);
// }

int Connection::_handleClient(int fd, short event)
{
	Client *client = _findClient(fd);
	if (!client)
		return (-1);

	int status = 0;

	if (event & POLLIN)
		status = client->handleRead(fd);

	if (event & POLLIN)
		status = client->handleRead(fd);

	if (fd == client->getPipeReadFd() && client->getState() == Client::SEND_RESPONSE)
	{
		client->closePipeReadFd();
		_removePollFdByFd(fd);
		return 0;
	}

	if (event & POLLOUT)
		status = client->handleWrite(fd);

	// Do we have a read CGI pipe?
	int pr = client->getPipeReadFd();
	if (pr > -1 && _clientFds.find(pr) == _clientFds.end())
	{
		_addPollFd(pr, POLLIN);
		FdEntry e;
		e.type = FD_PIPE_READ;
		e.client = client;
		_clientFds[pr] = e;
	}

	// Do we have a write CGI pipe?
	int pw = client->getPipeWriteFd();
	if (pw > -1 && _clientFds.find(pw) == _clientFds.end())
	{
		_addPollFd(pw, POLLOUT);
		FdEntry e;
		e.type = FD_PIPE_WRITE;
		e.client = client;
		_clientFds[pw] = e;
	}

	if (client->hasActivePipeRead())
		_setFdPollEvent(client->getPipeReadFd(), POLLIN);

	if (client->hasActivePipeWrite())
		_setFdPollEvent(client->getPipeWriteFd(), POLLOUT);

	if (client->getState() == Client::SEND_RESPONSE)
		_setFdPollEvent(client->getConnFd(), POLLOUT);
	else
		_setFdPollEvent(client->getConnFd(), POLLIN);

	if (client->getState() == Client::CLOSE_CONNECTION)
		status = -1;

	return status;
}
