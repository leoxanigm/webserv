#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <vector>
#include <map>
#include <poll.h>

#include "Socket.hpp"
#include "Client.hpp"

/**
 * A class that handles one socket connection and its related events
 */
class Connection
{
public:
	Connection(); // Remove
	// Connection(const Config &config); // Init with config
	~Connection();

	void openConnection(void);
	void closeConnection();

private:
	// Connection();
	Connection(const Connection &obj);
	Connection &operator=(const Connection &obj);

	bool _sigint;

	// const Config _config;
	Socket _socket;
	std::vector<struct pollfd> _pollFds;
	std::map<int, Client > _fdClientConn; // track fd for remote connections
	std::map<int, Client &> _fdClientPipe; // track fd for server pipes

	void _runPoll();
	void _processConnections();
	void _handleNewConnection();
	void _handleClient(std::size_t index, short event);

	int _addClient();
	int _removeClient();
};

#endif /* CONNECTION_HPP */
