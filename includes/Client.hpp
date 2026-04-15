#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

#include "./parsing/ConfigFileParsing.hpp"
#include "./parsing/HTTPRequestParser.hpp"
#include "./CGIHandler.hpp"

class Client
{
public:
	Client(int connFd, const ConfigFileParsing::server_t &server);
	~Client();

	enum State
	{
		RECV_REQUEST,
		SEND_RESPONSE,
		READ_PIPE,
		WRITE_PIPE,
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

	// lets Connection know when to add/remove poll fds
	bool hasActivePipeRead() const;
	bool hasActivePipeWrite() const;

private:
	Client();
	Client(const Client &obj);
	Client &operator=(const Client &obj);

	const ConfigFileParsing::server_t &_server;
	HTTPRequest _req;

	int _connFd;
	int _pipeReadFd;
	int _pipeWriteFd;
	std::string _inBuffer;
	std::string _outBuffer;
	std::size_t _sentSize;
	State _state;

	// CGI
	CGIHandler _cgi;
	bool _isCGI;
	std::size_t _cgiBodyOffset;
	std::string _cgiRawOutput;

	void _finalizeCGIResponse();
};

#endif /* CLIENT_HPP */