#ifndef SOCKET_HPP
#define SOCKET_HPP

/**
 * A socket listener class. Throws on error.
 */
class Socket
{
public:
	Socket(const char *host, const char *service);
	Socket(const Socket &obj);
	~Socket();

	int fd(void) const;
	const char *host() const;
	const char *service() const;

private:
	Socket();
	Socket &operator=(const Socket &obj);

	const char *_host;
	const char *_service;
	int _listener; // listener file descriptor

	void _initListenerSocket(void);
};

#endif /* SOCKET_HPP */
