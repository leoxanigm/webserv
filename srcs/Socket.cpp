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

Socket::Socket(const char *host, const char *service) : _host(host), _service(service)
{
	// Initialize listener on start,
	_initListenerSocket(); // _listener
}

Socket::Socket(const Socket &obj) : _host(obj._host), _service(obj._service)
{
	_listener = obj._listener;
}

Socket::~Socket() {}

int Socket::fd(void) const
{
	return (_listener);
}

const char *Socket::host() const
{
	return (_host);
}

const char *Socket::service() const
{
	return (_service);
}

void Socket::_initListenerSocket(void)
{
	int listener, retVal, optVal = 1;

	struct addrinfo addrInfoHints, *addrInfo, *addrInfoIt;

	std::memset(&addrInfoHints, 0, sizeof(addrInfoHints));
	addrInfoHints.ai_family = AF_UNSPEC; // ipv4 or ipv6
	addrInfoHints.ai_socktype = SOCK_STREAM;
	addrInfoHints.ai_flags = AI_PASSIVE; // get ip for me

	if ((retVal = getaddrinfo(_host, _service, &addrInfoHints, &addrInfo)) != 0)
		throw std::runtime_error("Error: " + std::string(gai_strerror(retVal)));

	for (addrInfoIt = addrInfo; addrInfoIt != 0; addrInfoIt = addrInfoIt->ai_next)
	{
		listener = socket(
			addrInfoIt->ai_family,
			addrInfoIt->ai_socktype,
			addrInfoIt->ai_protocol);

		if (listener < 0)
		{
			close(listener);
			continue;
		}

		if (utils::setNonBlocking(listener) == -1)
		{
			close(listener);
			continue;
		}

		// Allow bind without wait
		// https://stackoverflow.com/a/3229926
		if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) != 0)
			continue;

		if (bind(listener, addrInfoIt->ai_addr, addrInfoIt->ai_addrlen) != 0)
		{
			close(listener);
			continue;
		}

		break; // We were able to bind
	}

	if (addrInfoIt == 0)
	{
		close(listener);
		throw std::runtime_error(
			"Error: unable to bind socket on port " + std::string(_service));
	}

	freeaddrinfo(addrInfo);

	if (listen(listener, SOMAXCONN) != 0)
	{
		close(listener);
		throw std::runtime_error(
			"Error: unable to listen socket on port " + std::string(_service));
	}

	_listener = listener;
}
