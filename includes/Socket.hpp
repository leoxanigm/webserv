#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <vector>

class Socket
{
public:
	Socket();
	Socket(const Socket &obj);
	Socket &operator=(const Socket &obj);
	~Socket();

private:
	std::vector<struct pollfd> _pollFds;

	int _getListenerSocket(void) const;
	void _runPoll(int listener);
	void _processConnections(int listener);
	void _handleNewConnection(int listener);
	void _handleClient(std::size_t index);
};

#endif /* SOCKET_HPP */
