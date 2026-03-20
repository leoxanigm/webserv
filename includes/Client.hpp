#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
public:
	Client(int connFd); // Change to Client(int connFd, const Config &config);
	~Client();

	int handleRead(int fd);
	int handleWrite(int fd);

	int getPipeFd(void) const; // Maybe we need _getPipeWriteFd and _getPipeReadFd?

private:
	Client();
	Client(const Client &obj);
	Client &operator=(const Client &obj);

	const int _connFd;
	int _pipeFd; // Maybe we need _pipeWriteFd and _pipeReadFd?
	std::string _inBuffer;
	std::string _outBuffer;

	enum State
	{
		READ_REQUEST,
		SEND_REQUEST,
		READ_PIPE,
		SEND_PIPE,
		CLOSE_PIPE
	} _state;
};

#endif /* CLIENT_HPP */
