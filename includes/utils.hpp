#ifndef UTILS_HPP
#define UTILS_HPP

namespace utils
{
	int		setNonBlocking(int fd);
	int		ft_count(const std::string &s, char c);
	bool	ft_str_is_digits(const std::string& str);
	bool	ft_endswith(const std::string &fullString, const std::string &ending);
}

#endif /* UTILS_HPP */
