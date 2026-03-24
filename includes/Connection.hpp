#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>
#include <vector>
#include <map>
#include <list>
#include <poll.h>

#include "Socket.hpp"
#include "Client.hpp"

// TEMP. REMOVE

typedef struct s_locations
{
	// std::string location = "/";
	// bool autoindex = false;
	// unsigned int return_code = 200;
	// std::string root = "./";
	// std::string index = "index.html";
	// // Did I miss some ?
} t_locations;

typedef struct s_servers
{
	std::list<t_locations> locations;
	// t_HTTPStatusCodesPages	error_pages;
	// bool is_default_server = false;
	// std::string interface = "localhost";
	std::string interface;
	// std::string listen_port = "8080";
	std::string listen_port;
	// std::string server_name = "webserv";
} t_servers;

class ConfigFileParsing
{
private:
	std::list<t_servers> _servers;

public:
	ConfigFileParsing()
	{
		t_servers server;
		server.interface = "localhost";
		server.listen_port = "8080";
		_servers.push_back(server);
	}
	~ConfigFileParsing() {}

	const std::list<t_servers> &servers() const
	{
		return _servers;
	}
};

// TEMP. REMOVE

/**
 * A class that handles one socket connection and its related events
 */
class Connection
{
public:
	Connection(const ConfigFileParsing &config); // Init with config
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

	struct FdEntry
	{
		FdType type;
		Client *client;
	};

	typedef const std::list<t_locations> locations_list_t;

	const ConfigFileParsing &_config;

	// Associative map for socket fd and corresponding config
	std::map<int, locations_list_t *> _sockets;

	std::vector<struct pollfd> _pollFds;

	// List of connected clients
	std::list<Client *> _clients;

	// Track fds for clients (connection and pipe)
	std::map<int, FdEntry> _clientFds;

	void _initSockets();
	void _runPoll();
	void _setFdPollEvent(int fd, short event);

	void _addPollFd(int fd, short event);
	void _addClient(int fd);
	Client *_findClient(int fd) const;
	void _closeClientFd(Client *client, FdType type);
	void _removeClient(int fd);
	void _removePollFd(std::size_t index);

	void _processConnections();
	void _handleNewConnection(int socketFd);
	int _handleClient(int fd, short event);
};

#endif /* CONNECTION_HPP */
