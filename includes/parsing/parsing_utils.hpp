/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_utils.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathboye <mathboye@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/17 23:15:00 by mathboye          #+#    #+#             */
/*   Updated: 2026/04/03 13:53:16 by mathboye         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSING_UTILS_HPP
# define PARSING_UTILS_HPP

# include <string>
# include <vector>

std::string					trim(const std::string &value);
std::string					toLower(const std::string &value);
void						splitByComma(const std::string &value, std::vector<std::string> &out);
bool						parseQualityParam(const std::string &params, std::string &outQ);
float						parseQualityFloat(const std::string &params, bool &ok);
std::vector<std::string>	extractBraces(const std::string& input);
bool						validateLocationPath(const std::string &path);

#endif
