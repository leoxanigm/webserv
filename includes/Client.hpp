#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
public:
	Client(int connFd); // Change to Client(int connFd, const Config &config);
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

	int getConnFd() const;
	int getPipeReadFd(void) const;
	int getPipeWriteFd(void) const;
	void closeConnFd();
	void closePipeReadFd();
	void closePipeWriteFd();

	State getState() const;

	// We want to add Client objects to a set 
	bool operator<(const Client &rhs) const;

private:
	Client();
	Client(const Client &obj);
	Client &operator=(const Client &obj);

	int _connFd;
	int _pipeReadFd;
	int _pipeWriteFd;
	std::string _inBuffer;
	std::string _outBuffer;

	State _state;
};

#endif /* CLIENT_HPP */
