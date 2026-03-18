#include <iostream>

#include "../includes/Socket.hpp"

int main(int argc, char const *argv[])
{
	(void) argc;
	(void) argv;

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
