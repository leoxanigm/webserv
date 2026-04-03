#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

#include "./parsing/ConfigFileParsing.hpp"
#include "./parsing/HTTPRequestParser.hpp"

class Client
{
public:
	Client(int connFd, const ConfigFileParsing::location_list_t &locations);
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

private:
	Client();
	Client(const Client &obj);
	Client &operator=(const Client &obj);

	const ConfigFileParsing::location_list_t &_locations;
	HTTPRequest _req;

	int _connFd;
	int _pipeReadFd;
	int _pipeWriteFd;
	std::string _inBuffer;
	std::string _outBuffer;

	std::size_t _sentSize;
	State _state;
};

#endif /* CLIENT_HPP */
