/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_utils.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathboye <mathboye@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/17 22:13:32 by mathboye          #+#    #+#             */
/*   Updated: 2026/03/17 22:58:25 by mathboye         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/parsing/parsing_utils.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>

std::string	trim(const std::string &value)
{
	std::string out = value;
	out.erase(out.begin(), std::find_if(out.begin(), out.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	out.erase(std::find_if(out.rbegin(), out.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), out.end());
	return (out);
}

std::string	toLower(const std::string &value)
{
	std::string out = value;
	std::transform(out.begin(), out.end(), out.begin(), ::tolower);
	return (out);
}

void	splitByComma(const std::string &value, std::vector<std::string> &out)
{
	std::string::size_type start = 0;
	while (start < value.size())
	{
		std::string::size_type comma = value.find(',', start);
		if (comma == std::string::npos)
		{
			out.push_back(trim(value.substr(start)));
			break;
		}
		out.push_back(trim(value.substr(start, comma - start)));
		start = comma + 1;
	}
}

bool	parseQualityParam(const std::string &params, std::string &outQ)
{
	std::string::size_type qPos = params.find("q=");
	if (qPos == std::string::npos)
		return (false);
	outQ = trim(params.substr(qPos + 2));
	return (!outQ.empty());
}

float	parseQualityFloat(const std::string &params, bool &ok)
{
	std::string qValue;
	ok = false;
	if (!parseQualityParam(params, qValue))
		return (1.0f);
	char *end;
	float q = std::strtof(qValue.c_str(), &end);
	ok = (end != qValue.c_str() && *end == '\0');
	return (ok ? q : 1.0f);
}
