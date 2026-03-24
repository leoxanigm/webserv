#include <iostream>
#include <fstream>
#include <sstream>
#include <csignal>
#include <poll.h>
#include <sys/socket.h>
#include <cstring>

#include "../includes/Connection.hpp"

volatile sig_atomic_t pollStop = 0;

void sigIntHandler(int sig)
{
	(void) sig;
	pollStop = 1;
}

int main(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;

	signal(SIGINT, sigIntHandler);

	try
	{
		// if (argc == 2)
		// 	ConfigFileParsing config(argv[1]);
		// else
		// 	ConfigFileParsing config;

		ConfigFileParsing config;
		Connection connection(config);
		connection.start();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
		return (1);
	}

	return (0);
}
