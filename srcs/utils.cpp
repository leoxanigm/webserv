#include <fcntl.h>
#include <iostream>
#include <fstream>

#include "../includes/utils.hpp"

int utils::setNonBlocking(int fd)
{
	int flags;

	flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return (-1);

	return (fcntl(fd, F_SETFL, flags | O_NONBLOCK));
}

int utils::ft_count(const std::string &s, char c)
{
	int res = 0;

	for (size_t i = 0; i < s.length(); i++)
		if (s[i] == c)
			res++;

	return (res);
}

bool utils::ft_str_is_digits(const std::string &str)
{
	for (int i = 0; str[i]; i++)
	{
		if (!(str[i] >= 48 && str[i] <= 57))
			return (false);
	}
	return (true);
}

/**
 * Reads file from the `path` argument as binary and returns a
 * a vector of char bytes.
 */
std::vector<char> utils::readFile(const std::string &path)
{
	std::vector<char> readData;
	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file.is_open())
		return (readData);

	file.seekg(0, file.end);
	std::size_t size = file.tellg();
	file.seekg(0, file.beg);

	readData.resize(size);
	file.read(readData.data(), size);
	file.close();

	return (readData);
}

bool utils::ft_endswith(const std::string &fullString, const std::string &ending)
{
    if (fullString.length() >= ending.length())
		return (fullString.compare(fullString.length() - ending.length(), ending.length(), ending) == 0);
    else
        return (false);
}

bool utils::_writeFile(const std::string &path, const std::string &body)
{
    std::ofstream ofs(path.c_str(), std::ios::binary);
    if (!ofs.is_open())
        return (false);
    ofs.write(body.c_str(), body.size());
    return (!ofs.fail());
}
