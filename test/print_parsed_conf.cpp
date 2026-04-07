/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_parsed_conf.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zalemu <zalemu@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/25 13:36:19 by mathboye          #+#    #+#             */
/*   Updated: 2026/04/03 17:55:37 by zalemu           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/tests.hpp"

void display_parsed_conf_file(const ConfigFileParsing &config)
{
	std::list<t_servers>	srvs = config.getServers();
	std::list<t_servers>::iterator it;

	for (it = srvs.begin(); it != srvs.end(); ++it)
	{
		std::cout << "server_name : " << it->server_name << std::endl;
		std::cout << "listen_interface : " << it->listen_interface << std::endl;
		std::cout << "listen_port : " << it->listen_port << std::endl;
		std::cout << "is_default_server : " << it->is_default_server << std::endl << std::endl;

		std::cout << "Locations : " << std::endl;
		std::list<t_locations>	locs = it->locations;
		std::list<t_locations>::iterator it2;
		for (it2 = locs.begin(); it2 != locs.end(); ++it2)
		{
			std::cout << "location : " << it2->location << std::endl;
			std::cout << "autoindex : " << it2->autoindex << std::endl;
			std::cout << "return_code : " << it2->return_code << std::endl;
			std::cout << "return_location : " << it2->return_location << std::endl;
			std::cout << "root : " << it2->root << std::endl;
			std::cout << "index : " << it2->index << std::endl << std::endl;

			std::cout << "Allowed methods : ";
			std::set<std::string>::iterator it3;
			for (it3 = it2->allowed_methods.begin(); it3 != it2->allowed_methods.end(); ++it3)
			{
				std::cout << it3->c_str() << " ";
			}
			std::cout << std::endl;
		}

		std::cout << "Status codes : " << std::endl;
		std::cout << "OK : " << it->error_pages.OK << std::endl;
		std::cout << "NoContent : " << it->error_pages.NoContent << std::endl;
		std::cout << "MovedPermanently : " << it->error_pages.MovedPermanently << std::endl;
		std::cout << "MovedTemporarily : " << it->error_pages.MovedTemporarily << std::endl;
		std::cout << "BadRequest : " << it->error_pages.BadRequest << std::endl;
		std::cout << "Unauthorized : " << it->error_pages.Unauthorized << std::endl;
		std::cout << "Forbidden : " << it->error_pages.Forbidden << std::endl;
		std::cout << "NotFound : " << it->error_pages.NotFound << std::endl;
		std::cout << "InternalServerError : " << it->error_pages.InternalServerError << std::endl;
		std::cout << "NotImplemented : " << it->error_pages.NotImplemented << std::endl;
		std::cout << "BadGateway : " << it->error_pages.BadGateway << std::endl;
		std::cout << "ServiceUnavailable : " << it->error_pages.ServiceUnavailable << std::endl;
	}
}
