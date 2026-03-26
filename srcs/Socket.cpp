#include <stdexcept>
#include <string>

#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>

#include "../includes/utils.hpp"
#include "../includes/Socket.hpp"
#include "../includes/parsing/HTTPRequestParser.hpp"

int Socket::newSocket(const char *host, const char *service)
{
	if (!host || !service)
		return (-1);

	int socketFd, retVal, optVal = 1;

	struct addrinfo addrInfoHints, *addrInfo, *addrInfoIt;

	std::memset(&addrInfoHints, 0, sizeof(addrInfoHints));
	addrInfoHints.ai_family = AF_UNSPEC; // ipv4 or ipv6
	addrInfoHints.ai_socktype = SOCK_STREAM;
	addrInfoHints.ai_flags = AI_PASSIVE; // get ip for me

	if ((retVal = getaddrinfo(host, service, &addrInfoHints, &addrInfo)) != 0)
		throw std::runtime_error("Error: " + std::string(gai_strerror(retVal)));

	for (addrInfoIt = addrInfo; addrInfoIt != 0; addrInfoIt = addrInfoIt->ai_next)
	{
		socketFd = socket(
			addrInfoIt->ai_family,
			addrInfoIt->ai_socktype,
			addrInfoIt->ai_protocol);

		if (socketFd < 0)
		{
			close(socketFd);
			continue;
		}

		if (utils::setNonBlocking(socketFd) == -1)
		{
			close(socketFd);
			continue;
		}

		// Allow bind without wait
		// https://stackoverflow.com/a/3229926
		if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) != 0)
		{
			close(socketFd);
			continue;
		}

		if (bind(socketFd, addrInfoIt->ai_addr, addrInfoIt->ai_addrlen) != 0)
		{
			close(socketFd);
			continue;
		}

		break; // We were able to bind
	}

	freeaddrinfo(addrInfo);

	if (addrInfoIt == 0)
	{
		close(socketFd);
		throw std::runtime_error(
			"Error: unable to bind socket on port "  + std::string(service));
	}

	if (listen(socketFd, SOMAXCONN) != 0)
	{
		close(socketFd);
		throw std::runtime_error(
			"Error: unable to listen socket on port " + std::string(service));
	}

	return (socketFd);
}
