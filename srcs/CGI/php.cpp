/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   php.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathboye <mathboye@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/02 15:41:14 by mathboye          #+#    #+#             */
/*   Updated: 2026/04/02 15:49:28 by mathboye         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/cgi.hpp"

std::string call_php_cgi(HTTPRequest request, std::string loc)
{
	int pipe_in[2];
	int pipe_out[2];
	
	if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
		return ("500");

	pid_t pid = fork();
	if (pid == -1)
	{
		close(pipe_in[0]);
		close(pipe_in[1]);
		close(pipe_out[0]);
		close(pipe_out[1]);
		return ("500");
	}
	
	if (pid == 0)
	{
		dup2(pipe_in[0], STDIN_FILENO);
		close(pipe_in[1]);
		
		dup2(pipe_out[1], STDOUT_FILENO);
		close(pipe_out[0]);
		
		char *argv[] = {(char *)"php-cgi", NULL};
		
		std::string gateway_interface = "GATEWAY_INTERFACE=CGI/1.1";
		std::string script_filename = "SCRIPT_FILENAME=" + loc;
		std::string redirect_status = "REDIRECT_STATUS=200";
		std::string request_method;
		std::string content_type;
		std::string content_length;
		std::string query_string;
		
		if (request.getMethod() == GET)
			request_method = "REQUEST_METHOD=GET";
		else if (request.getMethod() == POST)
			request_method = "REQUEST_METHOD=POST";
		else if (request.getMethod() == DELETE)
			request_method = "REQUEST_METHOD=DELETE";
		
		std::vector<char *> envp;
		envp.push_back((char *)gateway_interface.c_str());
		envp.push_back((char *)script_filename.c_str());
		envp.push_back((char *)redirect_status.c_str());
		envp.push_back((char *)request_method.c_str());
		if (request.getMethod() == POST)
		{
			std::map<std::string, std::string> content_type_map = request.getContentType();
			content_type = "CONTENT_TYPE=" + content_type_map.begin()->first + "/" + content_type_map.begin()->second;
			envp.push_back((char *)content_type.c_str());
			
			std::stringstream ss;
			ss << request.getContentLength();
			content_length = "CONTENT_LENGTH=" + ss.str();
			envp.push_back((char *)content_length.c_str());
		}
		else if (request.getMethod() == GET)
		{
			query_string = "QUERY_STRING=" + request.getBody();
			envp.push_back((char *)query_string.c_str());
		}
		
		envp.push_back(NULL);
		
		execve("/usr/bin/php-cgi", argv, envp.data());
		exit(1);
	}
	else
	{
		close(pipe_in[0]);
		close(pipe_out[1]);
		
		if (request.getMethod() == POST && !request.getBody().empty())
			write(pipe_in[1], request.getBody().c_str(), request.getBody().length());
		close(pipe_in[1]);
		
		std::stringstream output;
		char buffer[4096];
		ssize_t bytes_read;
		
		while ((bytes_read = read(pipe_out[0], buffer, sizeof(buffer))) > 0)
			output.write(buffer, bytes_read);
		close(pipe_out[0]);
		
		int status;
		waitpid(pid, &status, 0);
		
		return (output.str());
	}
	return ("");
}
