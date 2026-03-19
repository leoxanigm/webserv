#include <iostream>
#include <fstream>
#include <sstream>

#include <csignal> // Move to Connections/ServerEvents

#include <poll.h>
#include <sys/socket.h>
#include <cstring>

#include "../includes/Connection.hpp"

int testRecvHandler(int fd)
{
	char buffer[256];

	errno = 0;
	int nBytes = recv(fd, buffer, sizeof(buffer) - 1, 0);

	if (nBytes <= 0)
	{
		if (nBytes == 0) // connection closed
			std::cout << "INFO: socket " << fd << " hang up\n";
		else
			std::cerr << "ERROR: receiving data: " << std::strerror(errno) << '\n';

		return (-1);
	}

	buffer[nBytes] = '\0';

	// std::string rawRequest(buffer, nBytes);
	// HTTPRequest request(rawRequest);

	// /*
	// Example for testing :
	// GET / HTTP/1.1\r\nHost: localhost:9999\r\n
	// */
	// if (!request.isValid())
	// 	std::cerr << "WARN: failed to parse HTTP request from socket "
	// 				<< fd << ": " << request.getError() << '\n';
	// else
	// 	std::cout << "INFO: received from " << fd
	// 				<< ": " << nBytes << " bytes. \n";

	std::ifstream file("./www/html/index.html");
	std::ostringstream content;

	content << file.rdbuf();

	std::string body = content.str();
	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n"
			 << "Content-Type: text/html\r\n"
			 << "Content-Length: " << body.length() << "\r\n"
			 << "\r\n"
			 << body;

	std::string arr = response.str();
	int send_res = send(fd, arr.c_str(), arr.length(), 0);
	if (send_res == -1)
		std::cerr << "ERROR: sending response: " << strerror(errno) << '\n';
	else
		std::cout << "INFO: sent " << send_res << " bytes to socket " << fd << '\n';

	return (0);

	return (0);
}

// int testSendHandler(int fd)
// {
// 	/*
// 	* TESTS

// 	Here we have an `index.html` file to test a static webpage
// 	without taking in account the header yet
// 	*/
// 	std::ifstream file("./www/html/index.html");
// 	std::ostringstream content;

// 	content << file.rdbuf();

// 	std::string body = content.str();
// 	std::ostringstream response;
// 	response << "HTTP/1.1 200 OK\r\n"
// 			 << "Content-Type: text/html\r\n"
// 			 << "Content-Length: " << body.length() << "\r\n"
// 			 << "\r\n"
// 			 << body;

// 	std::string arr = response.str();
// 	int send_res = send(fd, arr.c_str(), arr.length(), 0);
// 	if (send_res == -1)
// 		std::cerr << "ERROR: sending response: " << strerror(errno) << '\n';
// 	else
// 		std::cout << "INFO: sent " << send_res << " bytes to socket " << fd << '\n';

// 	return (0);
// }

int main(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;

	try
	{
		Connection connection;
		connection.attachEvent(POLLIN, testRecvHandler);
		// connection.attachEvent(POLLOUT, testSendHandler);
		connection.openConnection();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
		return (1);
	}

	return (0);
}
