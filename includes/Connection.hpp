#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <vector>
#include <map>
#include <poll.h>

#include "Socket.hpp"

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
	void closeConnection(int sig);
	void attachEvent(short event, int (*callBack)(int));

private:
	// Connection();
	Connection(const Connection &obj);
	Connection &operator=(const Connection &obj);

	bool _sigint;

	// const Config _config;
	Socket _socket;
	std::vector<struct pollfd> _pollFds;
	std::map<short, int (*)(int)> _events;

	void _initEvents(void);

	void _runPoll();
	void _processConnections();
	void _handleNewConnection();
	void _handleClient(std::size_t index, short event);
};



#endif /* CONNECTION_HPP */
