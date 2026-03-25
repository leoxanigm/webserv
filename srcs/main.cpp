#include <iostream>

#include "../includes/Socket.hpp"
#include "../includes/parsing/ConfigFileParsing.hpp"
#include "../includes/UI.hpp"
#include "../includes/tests.hpp"

int main(int argc, char const *argv[])
{
	(void) argc;
	(void) argv;

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

	try
	{
		Socket webservSocket;
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return (1);
	}

	return (0);
}
