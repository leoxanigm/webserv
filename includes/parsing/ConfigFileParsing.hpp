/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFileParsing.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathboye <mathboye@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/18 15:43:52 by mathboye          #+#    #+#             */
/*   Updated: 2026/03/25 13:32:14 by mathboye         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGFILEPARSING_HPP
# define CONFIGFILEPARSING_HPP

# include <list>
# include <string>
# include <cerrno>
# include <clocale>
# include <vector>
# include <algorithm>
# include <iostream>
# include <sstream>
# include <fstream>

typedef struct s_HTTPStatusCodePages
{
	std::string	OK;
	std::string	NoContent;

	std::string	MovedPermanently;
	std::string	MovedTemporarily;

	std::string	BadRequest;
	std::string	Unauthorized;
	std::string	Forbidden;
	std::string	NotFound;

	std::string	InternalServerError;
	std::string	NotImplemented;
	std::string	BadGateway;
	std::string	ServiceUnavailable;

	s_HTTPStatusCodePages() : 
		OK("./confs/status_pages/200.html"),
		NoContent("./confs/status_pages/204.html"),
		MovedPermanently("./confs/status_pages/301.html"),
		MovedTemporarily("./confs/status_pages/302.html"),
		BadRequest("./confs/status_pages/400.html"),
		Unauthorized("./confs/status_pages/401.html"),
		Forbidden("./confs/status_pages/403.html"),
		NotFound("./confs/status_pages/404.html"),
		InternalServerError("./confs/status_pages/500.html"),
		NotImplemented("./confs/status_pages/501.html"),
		BadGateway("./confs/status_pages/502.html"),
		ServiceUnavailable("./confs/status_pages/503.html") {}
}	t_HTTPStatusCodesPages;

typedef struct	s_locations
{
	std::string		location;
	bool			autoindex;
	unsigned int	return_code;
	std::string		root;
	std::string		index;
	// Did I miss some ?

	s_locations() : 
		location("/"),
		autoindex(false),
		return_code(200),
		root("./"),
		index("index.html") {}
}	t_locations;

typedef struct	s_servers
{
	std::list<t_locations>	locations;
	t_HTTPStatusCodesPages	error_pages;
	bool					is_default_server;
	std::string				listen_port;
	std::string				listen_interface;
	std::string				server_name;

	s_servers() : 
		is_default_server(false),
		listen_port("8080"),
		listen_interface("[::]"),
		server_name("webserv") {}
}	t_servers;

class ConfigFileParsing
{
	private:
		std::list<t_servers>	_servers;

	public:
		ConfigFileParsing(std::string raw_conf);
		std::list<t_servers>	getServers(void);
		~ConfigFileParsing();
};

#endif
