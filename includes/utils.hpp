#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>

namespace utils
{
	int					setNonBlocking(int fd);
	int					ft_count(const std::string &s, char c);
	bool				ft_str_is_digits(const std::string& str);
	std::vector<char>	readFile(const std::string &path);
}

#endif /* UTILS_HPP */
