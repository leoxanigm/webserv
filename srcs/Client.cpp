#include "../includes/Client.hpp"
#include "../includes/HTTPError.hpp"

Client::Client(int connFd) : _connFd(connFd), _pipeFd(-1) {}

Client::~Client() {}

int Client::handleRead(int fd)
{
	(void) fd;
	return (-1);
}

int Client::handleWrite(int fd)
{
	(void) fd;
	return (-1);
}

/**
 * If a client object uses CGI, we have to poll read/write to pipe,
 * so we can request the object for the pipe read/write fd
 */
int Client::getPipeFd(void) const
{
	return (_pipeFd);
}
