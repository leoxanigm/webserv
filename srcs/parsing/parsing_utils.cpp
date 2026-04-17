/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_utils.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathboye <mathboye@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/17 22:13:32 by mathboye          #+#    #+#             */
/*   Updated: 2026/04/07 16:31:02 by mathboye         ###   ########.fr       */
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
	double q = std::strtod(qValue.c_str(), &end);
	ok = (end != qValue.c_str() && *end == '\0');
	return (ok ? static_cast<float>(q) : 1.0f);
}

std::vector<std::string> extractBraces(const std::string& input)
{
    std::vector<std::string> results;
    int brace_count = 0;
    size_t start_pos = 0;
    bool in_brace = false;

    for (size_t i = 0; i < input.length(); ++i)
	{
        if (input[i] == '{')
		{
            if (brace_count == 0)
			{
                start_pos = i + 1;
                in_brace = true;
            }
            brace_count++;
        }
		else if (input[i] == '}')
		{
            brace_count--;
            if (brace_count == 0 && in_brace)
			{
                results.push_back(input.substr(start_pos, i - start_pos));
                in_brace = false;
            }
        }
    }
    return (results);
}

bool validateLocationPath(const std::string &path)
{
    if (path.empty() || path[0] != '/')
        return (false);
    for (size_t i = 1; i < path.size(); ++i)
    {
        if (path[i] == '/' && path[i - 1] == '/')
            return (false);
        if (!std::isalnum((unsigned char)path[i]) && path[i] != '-' && path[i] != '_' && path[i] != '/')
            return (false);
    }
    return (true);
}
