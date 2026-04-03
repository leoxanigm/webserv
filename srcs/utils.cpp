#include <fcntl.h>
#include <iostream>
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

bool utils::ft_str_is_digits(const std::string& str)
{
    for (int i = 0; str[i]; i++)
	{
        if (!(str[i] >= 48 && str[i] <= 57))
            return (false);
    }
	return (true);
}

bool utils::ft_endswith(const std::string &fullString, const std::string &ending)
{
    if (fullString.length() >= ending.length())
		return (fullString.compare(fullString.length() - ending.length(), ending.length(), ending) == 0);
    else
        return (false);
}
