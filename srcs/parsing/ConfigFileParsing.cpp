/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFileParsing.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathboye <mathboye@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/23 14:09:04 by mathboye          #+#    #+#             */
/*   Updated: 2026/03/26 12:41:57 by mathboye         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/parsing/ConfigFileParsing.hpp"
#include "../../includes/parsing/parsing_utils.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <cstdlib>

std::vector<std::string> tokenize(const std::string& str)
{
    std::vector<std::string> tokens;
    std::string token;
    bool in_quotes = false;
    char quote_char = 0;

    for (size_t i = 0; i < str.length(); ++i)
	{
        char c = str[i];

        if (c == '#')
		{
            while (i < str.length() && str[i] != '\n')
				++i;
            continue;
        }
        if (in_quotes)
		{
            token += c;
            if (c == quote_char)
			{
                in_quotes = false;
                tokens.push_back(token);
                token.clear();
            }
        }
		else
		{
            if (c == '"' || c == '\'')
			{
                in_quotes = true;
                quote_char = c;
                token += c;
            }
			else if (std::isspace(c))
			{
                if (!token.empty())
				{
                    tokens.push_back(token);
                    token.clear();
                }
            }
			else if (c == '{' || c == '}' || c == ';')
			{
                if (!token.empty())
				{
                    tokens.push_back(token);
                    token.clear();
                }
                tokens.push_back(std::string(1, c));
            }
			else
                token += c;
        }
    }
    if (!token.empty())
		tokens.push_back(token);
    return (tokens);
}

ConfigFileParsing::ConfigFileParsing(std::string raw_conf)
{
    std::vector<std::string> tokens = tokenize(raw_conf);
    
    t_servers current_server;
    t_locations current_location;
    bool in_server = false;
    bool in_location = false;
    
    for (size_t i = 0; i < tokens.size(); ++i)
	{
        if (tokens[i] == "server" && i + 1 < tokens.size() && tokens[i + 1] == "{")
		{
            current_server = t_servers();
            in_server = true;
            ++i;
        }
		else if (tokens[i] == "location" && in_server && !in_location && i + 2 < tokens.size())
		{
            current_location = t_locations();
            in_location = true;

            if (tokens[i + 2] == "{")
			{
                current_location.location = tokens[i + 1];
                i += 2;
            }
			else if (tokens[i + 3] == "{")
			{
                current_location.location = tokens[i + 1] + " " + tokens[i + 2];
                i += 3;
            }
        }
		else if (tokens[i] == "}")
		{
            if (in_location)
			{
                current_server.locations.push_back(current_location);
                in_location = false;
            }
			else if (in_server)
			{
                _servers.push_back(current_server);
                in_server = false;
            }
        }
		else if (in_location)
		{
            if (tokens[i] == "root" && i + 2 < tokens.size() && tokens[i + 2] == ";")
			{
                current_location.root = tokens[i + 1];
                i += 2;
            }
			else if (tokens[i] == "index" && i + 2 < tokens.size() && tokens[i + 2] == ";")
			{
                current_location.index = tokens[i + 1];
                i += 2;
            }
			else if (tokens[i] == "cgi_php" && i + 2 < tokens.size() && tokens[i + 2] == ";")
			{
                current_location.is_cgi = true;
                current_location.cgi_type = PHP;
                current_location.cgi_location = tokens[i + 1];
                i += 2;
            }
			else if (tokens[i] == "cgi_python" && i + 2 < tokens.size() && tokens[i + 2] == ";")
			{
                current_location.is_cgi = true;
                current_location.cgi_type = PYTHON;
                current_location.cgi_location = tokens[i + 1];
                i += 2;
            }
			else if (tokens[i] == "autoindex" && i + 2 < tokens.size() && tokens[i + 2] == ";")
			{
                current_location.autoindex = (tokens[i + 1] == "on");
                i += 2;
            }
			else if (tokens[i] == "return" && i + 2 < tokens.size() && tokens[i + 2] == ";")
			{
                current_location.return_code = std::atoi(tokens[i + 1].c_str());
                i += 2;
            }
			else
			{
                while (i < tokens.size() && tokens[i] != ";" && tokens[i] != "}")
					++i;
                if (tokens[i] == "}")
					--i;
            }
        }
		else if (in_server)
		{
            if (tokens[i] == "listen" && i + 2 < tokens.size())
			{
                std::string listen_val = tokens[i + 1];

                if (listen_val.find(":") != std::string::npos)
				{
                    current_server.listen_interface = listen_val.substr(0, listen_val.find_last_of(":"));
                    current_server.listen_port = listen_val.substr(listen_val.find_last_of(":") + 1);
                }
				else
                    current_server.listen_port = listen_val;

                while (i < tokens.size() && tokens[i] != ";")
				{
                    if (tokens[i] == "default_server")
						current_server.is_default_server = true;
                    i++;
                }
            }
			else if (tokens[i] == "server_name" && i + 2 < tokens.size() && tokens[i + 2] == ";")
			{
                current_server.server_name = tokens[i + 1];
                i += 2;
            }
			else if (tokens[i] == "error_page" && i + 3 < tokens.size() && tokens[i + 3] == ";")
			{
                int code = std::atoi(tokens[i + 1].c_str());
                std::string page = tokens[i + 2];

                if (code == 404)
					current_server.error_pages.NotFound = page;
                else if (code == 403)
					current_server.error_pages.Forbidden = page;
                else if (code == 500)
					current_server.error_pages.InternalServerError = page;
                i += 3;
            }
			else
			{
                while (i < tokens.size() && tokens[i] != ";" && tokens[i] != "}")
					++i;
                if (tokens[i] == "}")
					--i;
            }
        }
    }
}

const std::list<t_servers>	&ConfigFileParsing::getServers(void) const
{
	return (_servers);
}

ConfigFileParsing::~ConfigFileParsing()
{}
