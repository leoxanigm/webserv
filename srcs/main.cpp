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
	
	std::ifstream	file;
	if (argc < 2)
		file.open("./confs/default.conf");
	else
		file.open(argv[1]);

	if (!file.is_open())
	{
		std::cerr << RED << "The given configuration file can not be opened." << RESET << std::endl;
		return (1);
	}

	std::string			line;
	std::string			raw;

    while(std::getline(file, line))
	{
		raw += line;
		raw += "\n";
	}

	ConfigFileParsing	config(raw);
	if (!config.isValid())
	{
		// display_parsed_conf_file(config);
		std::cerr << RED << "The given configuration file is not valid." << RESET << std::endl;
		std::cerr << RED << "On " << BLUE << UNDERLINE_BLUE << argv[1] << RESET << RED << ". See reference below:" << std::endl;
		std::istringstream	errors(config.getErrorLine().c_str());
		std::string			err;
		while(std::getline(errors, err))
			std::cerr << RED << "|\t" << err << std::endl;
		return (1);
	}

	try
	{
		Connection connection(config.getServers());
		connection.start();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
		return (1);
	}

	return (0);
}
