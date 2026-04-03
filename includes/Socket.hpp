#ifndef SOCKET_HPP
#define SOCKET_HPP

/**
 * A socket listener class. Throws on error.
 */
class Socket
{
public:
	static int newSocket(const char *host, const char *service);

private:
	Socket();
	Socket(const Socket &obj);
	Socket &operator=(const Socket &obj);
	~Socket();
};

#endif /* SOCKET_HPP */
