#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <csignal>
#include <poll.h>
#include <sys/socket.h>
#include <cstring>

#include "../includes/parsing/ConfigFileParsing.hpp"
#include "../includes/Connection.hpp"
#include "../includes/UI.hpp"
#include "../includes/tests.hpp"

volatile sig_atomic_t pollStop = 0;

void sigIntHandler(int sig)
{
	(void) sig;
	pollStop = 1;
}

int main(int argc, char const *argv[])
{
	signal(SIGINT, sigIntHandler);

	if (argc != 2)
	{
		std::cout << RED << "Not enough arguments\n" << "Usage: ./" << argv[0] << " <conf_file.conf>" << std::endl;
		return (1);
	}

    std::ifstream file(argv[1]);
    std::string line;
    std::string raw;

    while(std::getline(file, line))
	{
		raw += line;
		raw += "\n";
	}
	ConfigFileParsing	config(raw);
	display_parsed_conf_file(config);  // Debug
	return (0);

	try
	{
		// if (argc == 2)
		// 	ConfigFileParsing config(argv[1]);
		// else
		// 	ConfigFileParsing config;

		// ConfigFileParsing config;
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
