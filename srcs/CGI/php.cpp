#include <unistd.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>
#include "../../includes/parsing/HTTPRequestParser.hpp"

int call_php_cgi(HTTPRequest request)
{
	char *argv[] = {(char *)"php-cgi", NULL};
	
	std::string gateway_interface = "GATEWAY_INTERFACE=CGI/1.1";
	std::string script_filename = "SCRIPT_FILENAME=" + request.getRequestTarget();
	std::string redirect_status = "REDIRECT_STATUS=200";
	std::string request_method;
	std::string content_type;
	std::string content_length;
	std::string query_string;
	
	if (request.getMethod() == 0)
		request_method = "REQUEST_METHOD=GET";
	else if (request.getMethod() == 1)
		request_method = "REQUEST_METHOD=POST";
	else if (request.getMethod() == 2)
		request_method = "REQUEST_METHOD=DELETE";
	
	std::vector<char *> envp;
	envp.push_back((char *)gateway_interface.c_str());
	envp.push_back((char *)script_filename.c_str());
	envp.push_back((char *)redirect_status.c_str());
	envp.push_back((char *)request_method.c_str());
	
	if (request.getMethod() == 1)
	{
		std::map<std::string, std::string> content_type_map = request.getContentType();
		content_type = "CONTENT_TYPE=" + content_type_map.begin()->first + "/" + content_type_map.begin()->second;
		envp.push_back((char *)content_type.c_str());
		
		content_length = "CONTENT_LENGTH=" + std::to_string(request.getContentLength());
		envp.push_back((char *)content_length.c_str());
	}
	else if (request.getMethod() == 0)
	{
		query_string = "QUERY_STRING=";
		envp.push_back((char *)query_string.c_str());
	}
	
	envp.push_back(NULL);
	
	execve("/usr/bin/php-cgi", argv, envp.data());
	
	return 0;
}
