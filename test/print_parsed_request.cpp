/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_parsed_request.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathboye <mathboye@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/25 13:36:19 by mathboye          #+#    #+#             */
/*   Updated: 2026/04/17 13:09:34 by mathboye         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/tests.hpp"

void display_parsed_request(const HTTPRequest &request)
{
	std::cout << "--- HTTP Request ---" << std::endl;
	std::cout << "Method: ";
	switch (request.getMethod())
	{
		case GET: std::cout << "GET"; break;
		case POST: std::cout << "POST"; break;
		case DELETE: std::cout << "DELETE"; break;
	}
	std::cout << std::endl;
	
	std::cout << "Target: " << request.getRequestTarget() << std::endl;
	std::cout << "Version: " << request.getHTTPVersion() << std::endl;
	std::cout << "Host: " << request.getHost() << std::endl;
	std::cout << "User-Agent: " << request.getUserAgent() << std::endl;
	std::cout << "Transfer-Encoding: " << request.getTransfertEncoding() << std::endl;
	std::cout << "Content-Length: " << request.getContentLength() << std::endl;
	
	std::cout << "Accept Languages: " << std::endl;
	for (std::map<std::string, float>::const_iterator it = request.getLanguages().begin(); it != request.getLanguages().end(); ++it)
		std::cout << "  - " << it->first << ": " << it->second << std::endl;
		
	std::cout << "Accept: " << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = request.getAccept().begin(); it != request.getAccept().end(); ++it)
		std::cout << "  - " << it->first << ": " << it->second << std::endl;
		
	std::cout << "Content-Type: " << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = request.getContentType().begin(); it != request.getContentType().end(); ++it)
		std::cout << "  - " << it->first << ": " << it->second << std::endl;
		
	std::cout << "Cookies: " << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = request.getCookie().begin(); it != request.getCookie().end(); ++it)
		std::cout << "  - " << it->first << ": " << it->second << std::endl;

	std::cout << "Body: " << request.getBody() << std::endl;
	std::cout << "Complete: " << (request.isComplete() ? "true" : "false") << std::endl;
	std::cout << "Valid: " << (request.isValid() ? "true" : "false") << std::endl;
	if (!request.isValid())
		std::cout << "Error: " << request.getError() << std::endl;
	std::cout << "--------------------" << std::endl;
}
