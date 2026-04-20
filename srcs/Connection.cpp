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

Connection::Connection(ConfigFileParsing::server_list_t servers) : _servers(servers)
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

	std::list<t_servers>::iterator it = _servers.begin();

	for (; it != _servers.end(); ++it)
	{
		std::cout << YELLOW << "Server " << UNDERLINE_YELLOW << it->server_name << YELLOW << " avalaible at:" << std::endl;

		std::list<t_locations> locs = it->locations;
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
	ConfigFileParsing::servers_iterator_t it = _servers.begin();
	int count = 0;
	int socketFd = -1;

	for (; it != _servers.end(); it++)
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
	int pollCount = 0;
	while (!pollStop && !_pollFds.empty()) // run until SIGINT
	{
		pollCount = poll(&_pollFds[0], _pollFds.size(), 1000);

		if (pollCount < 0)
		{
			if (errno == EINTR) // SIGINT
				continue;
			throw HTTPError(HTTPError::INTERNAL_SERVER_ERROR);
		}

		if (pollCount > 0)
			_processConnections();

		_processTimeouts();
	}
}

void Connection::_setFdPollEvent(int fd, short event)
{
	if (fd < 0)
		return;

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

bool Connection::_hasPollFd(int fd) const
{
	for (std::size_t i = 0; i < _pollFds.size(); ++i)
	{
		if (_pollFds[i].fd == fd)
			return (true);
	}
	return (false);
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

void Connection::_addClientFd(int fd, Client *client, FdType type)
{
	if (fd < 0)
		return;

	FdEntry entry;
	entry.type = type;
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

void Connection::_removeClientFd(int fd)
{
	std::map<int, FdEntry>::iterator it = _clientFds.find(fd);
	if (it == _clientFds.end())
		return;

	Client *client = it->second.client;
	FdType type = it->second.type;

	if (type == FD_CLIENT_CONN)
	{
		_removeClient(client);
	}
	else
	{
		_closeClientFd(client, type);
		_clientFds.erase(it);
	}
}

void Connection::_removeClient(Client *client)
{
	int connFd = client->getConnFd();
	int pipeReadFd = client->getPipeReadFd();
	int pipeWriteFd = client->getPipeWriteFd();

	if (connFd >= 0)
	{
		_closeClientFd(client, FD_CLIENT_CONN);
		_clientFds.erase(connFd);
		_removePollFd(connFd);
	}

	if (pipeReadFd >= 0)
	{
		_closeClientFd(client, FD_PIPE_READ);
		_clientFds.erase(pipeReadFd);
		_removePollFd(pipeReadFd);
	}

	if (pipeWriteFd >= 0)
	{
		_closeClientFd(client, FD_PIPE_WRITE);
		_clientFds.erase(pipeWriteFd);
		_removePollFd(pipeWriteFd);
	}

	_clients.remove(client);
	delete client;
}

void Connection::_removePollFd(std::size_t index)
{
	_pollFds[index] = _pollFds[_pollFds.size() - 1];
	_pollFds.pop_back();
}

void Connection::_removePollFd(int fd)
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
	int status;

	for (std::size_t i = 0; i < _pollFds.size();)
	{
		status = 0;
		fd = _pollFds[i].fd;
		revents = _pollFds[i].revents;

		if (revents != 0)
		{
			if ((revents & POLLIN) && _sockets.find(fd) != _sockets.end())
				_handleNewConnection(fd);
			else
				status = _handleClient(fd, revents);
		}

		if (status == -1)
		{
			_removePollFd(fd);
			_removeClientFd(fd);
			continue;
		}

		i++;
	}
}

void Connection::_processTimeouts()
{
	std::list<Client *>::iterator it = _clients.begin();
	Client *client;

	for (; it != _clients.end();)
	{
		client = *it;
		it++;

		if (client->hasTimedOut())
		{
			client->handleTimeout();

			if (client->getState() == Client::SEND_RESPONSE && client->getConnFd() >= 0)
				_setFdPollEvent(client->getConnFd(), POLLOUT);
			else if (client->getState() == Client::CLOSE_CONNECTION)
			{
				_removePollFd(client->getConnFd());
				_removeClient(client);
			}
		}
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

	if (clientFd == -1)
	{
		std::cerr << "ERROR: accepting new client: " << std::strerror(errno) << '\n';
	}

	if (utils::setNonBlocking(clientFd) == -1)
	{
		close(clientFd);
		std::cerr << "ERROR: setNonBlocking for socket " << socketFd << ": " << std::strerror(errno) << '\n';
		return;
	}

	_addPollFd(clientFd, POLLIN);
	_addClient(clientFd, _sockets[socketFd]);

	std::cout << "INFO: new connection on socket " << socketFd << '\n';
	// Maybe print more info using remoteIP char array?
	// Or create a Logger which will log output info, error, warning to file
}

int Connection::_handleClient(int fd, short revents)
{
	Client *client = _findClient(fd);
	if (!client)
		return (-1);

	int status = 0;

	// CGI stdout pipe
	if (fd == client->getPipeReadFd())
	{
		if (revents & (POLLIN | POLLHUP))
			status = client->handleRead(fd);

		if (client->getState() == Client::SEND_RESPONSE ||
			client->getState() == Client::CLOSE_CONNECTION)
		{
			_removePollFd(fd);
			_removeClientFd(fd);
		}

		if (client->getState() == Client::SEND_RESPONSE)
			_setFdPollEvent(client->getConnFd(), POLLOUT);

		return (status);
	}

	// CGI stdin pipe
	if (fd == client->getPipeWriteFd())
	{
		if (revents & POLLOUT)
			status = client->handleWrite(fd);

		if (revents & (POLLHUP | POLLERR))
			client->setState(Client::READ_PIPE);

		if (client->getState() == Client::READ_PIPE ||
			client->getState() == Client::CLOSE_CONNECTION)
		{
			_removePollFd(fd);
			_removeClientFd(fd);
		}

		return (status);
	}

	// Normal client socket read
	if (fd == client->getConnFd() && revents & POLLIN)
		status = client->handleRead(fd);

	// After handleRead(), CGI may have started.
	// Register the correct CGI pipe fd.
	if (client->getState() == Client::WRITE_PIPE)
	{
		int pipeWriteFd = client->getPipeWriteFd();

		if (pipeWriteFd >= 0 && !_hasPollFd(pipeWriteFd))
		{
			_addClientFd(pipeWriteFd, client, FD_PIPE_WRITE);
			_addPollFd(pipeWriteFd, POLLOUT);
		}
	}
	else if (client->getState() == Client::READ_PIPE)
	{
		int pipeWriteFd = client->getPipeWriteFd();
		int pipeReadFd = client->getPipeReadFd();

		// For GET/no-body CGI, close CGI stdin so it receives EOF
		if (pipeWriteFd >= 0)
		{
			_removePollFd(pipeWriteFd);
			_removeClientFd(pipeWriteFd);
		}

		if (pipeReadFd >= 0 && !_hasPollFd(pipeReadFd))
		{
			_addClientFd(pipeReadFd, client, FD_PIPE_READ);
			_addPollFd(pipeReadFd, POLLIN);
		}
	}

	// Normal client socket write.
	if (fd == client->getConnFd() && revents & POLLOUT)
		status = client->handleWrite(fd);

	if (client->getState() == Client::SEND_RESPONSE)
		_setFdPollEvent(client->getConnFd(), POLLOUT);

	if (client->getState() == Client::CLOSE_CONNECTION)
		status = -1;

	if (revents & (POLLHUP | POLLERR | POLLNVAL))
	{
		client->setState(Client::CLOSE_CONNECTION);
		status = -1;
	}

	return (status);
}
