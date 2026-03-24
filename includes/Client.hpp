#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
public:
	Client(int connFd); // Change to Client(int connFd, const Config &config);
	Client(const Client &obj);
	~Client();

	enum State
	{
		READ_REQUEST,
		SEND_RESPONSE,
		READ_PIPE,
		SEND_PIPE,
		CLOSE_PIPE,
		CLOSE_CONNECTION
	};

	int handleRead(int fd);
	int handleWrite(int fd);

	int connFd() const;
	int getPipeFd(void) const; // Maybe we need _getPipeWriteFd and _getPipeReadFd?

	State getState() const;

	// We want to add Client objects to a set 
	bool operator<(const Client &rhs) const;

private:
	Client();
	Client &operator=(const Client &obj);

	const int _connFd;
	int _pipeFd; // Maybe we need _pipeWriteFd and _pipeReadFd?
	std::string _inBuffer;
	std::string _outBuffer;

	State _state;
};

#endif /* CLIENT_HPP */
