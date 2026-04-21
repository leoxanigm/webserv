#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

namespace utils
{
	int					setNonBlocking(int fd);
	int					ft_count(const std::string &s, char c);
	bool				ft_str_is_digits(const std::string& str);
	bool				ft_endswith(const std::string &fullString, const std::string &ending);
	std::vector<char>	readFile(const std::string &path);
	bool				_writeFile(const std::string &path, const std::string &body);
	long long			nowSeconds();
}

#endif /* UTILS_HPP */
