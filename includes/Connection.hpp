#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>
#include <vector>
#include <map>
#include <list>
#include <poll.h>

#include "Socket.hpp"
#include "Client.hpp"
#include "./parsing/ConfigFileParsing.hpp"

/**
 * A class that handles one socket connection and its related events
 */
class Connection
{
public:
	Connection(ConfigFileParsing::server_list_t servers); // Init with list of servers
	~Connection();

	void start();

private:
	Connection();
	Connection(const Connection &obj);
	Connection &operator=(const Connection &obj);

	enum FdType
	{
		FD_CLIENT_CONN,
		FD_PIPE_READ,
		FD_PIPE_WRITE
	};

	enum timeoutEvent
	{
		TIMEOUT = 408
	};

	struct FdEntry
	{
		FdType type;
		Client *client;
	};

	ConfigFileParsing::server_list_t _servers;

	// Associative map for socket fd and corresponding config
	std::map<int, const ConfigFileParsing::server_t *> _sockets;

	std::vector<struct pollfd> _pollFds;

	// List of connected clients
	std::list<Client *> _clients;

	// Track fds for clients (connection and pipe)
	std::map<int, FdEntry> _clientFds;

	void _initSockets();
	void _runPoll();
	void _setFdPollEvent(int fd, short event);

	void _addPollFd(int fd, short event);
	bool _hasPollFd(int fd) const;
	void _addClient(int fd, const ConfigFileParsing::server_t *server);
	void _addClientFd(int fd, Client *client, FdType type);
	Client *_findClient(int fd) const;
	void _closeClientFd(Client *client, FdType type);
	void _removeClientFd(int fd);
	void _removeClient(Client *client);
	void _removePollFd(std::size_t index);
	void _removePollFd(int fd);

	void _processConnections();
	void _processTimeouts();
	void _handleNewConnection(int socketFd);
	int _handleClient(int fd, short revents);
};

#endif /* CONNECTION_HPP */
