/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFileParsing.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathboye <mathboye@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/18 15:43:52 by mathboye          #+#    #+#             */
/*   Updated: 2026/03/23 13:27:37 by mathboye         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGFILEPARSING_HPP
# define CONFIGFILEPARSING_HPP

# include <list>
# include <string>
# include <cerrno>
# include <clocale>
# include <cstdlib>
# include <algorithm>
# include <iostream>
# include <sstream>
# include <fstream>

typedef struct s_HTTPStatusCodePages
{
	std::string	OK					= "./confs/status_pages/200.html";
	std::string	NoContent			= "./confs/status_pages/204.html";

	std::string	MovedPermanently	= "./confs/status_pages/301.html";
	std::string	MovedTemporarily	= "./confs/status_pages/302.html";

	std::string	BadRequest			= "./confs/status_pages/400.html";
	std::string	Unauthorized		= "./confs/status_pages/401.html";
	std::string	Forbidden			= "./confs/status_pages/403.html";
	std::string	NotFound			= "./confs/status_pages/404.html";

	std::string	InternalServerError	= "./confs/status_pages/500.html";
	std::string	NotImplemented		= "./confs/status_pages/501.html";
	std::string	BadGateway			= "./confs/status_pages/502.html";
	std::string	ServiceUnavailable	= "./confs/status_pages/503.html";
}	t_HTTPStatusCodesPages;

typedef struct	s_locations
{
	std::string		location	= "/";
	bool			autoindex	= false;
	unsigned int	return_code	= 200;
	std::string		root		= "./";
	std::string		index		= "index.html";
	// Did I miss some ?
}	t_locations;

typedef struct	s_servers
{
	std::list<t_locations>	locations;
	t_HTTPStatusCodesPages	error_pages;
	bool					is_default_server = false;
	char					*listen_port	= "8080";
	std::string				server_name	= "webserv";
}	t_servers;

class ConfigFileParsing
{
	private:
		std::list<t_servers>	_servers;

	public:
		ConfigFileParsing(/* args */);
		~ConfigFileParsing();
};

ConfigFileParsing::ConfigFileParsing(/* args */)
{
}

ConfigFileParsing::~ConfigFileParsing()
{
}

#endif
