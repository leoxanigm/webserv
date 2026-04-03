/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathboye <mathboye@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/02 15:40:34 by mathboye          #+#    #+#             */
/*   Updated: 2026/04/02 15:47:41 by mathboye         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
#define CGI_HPP

#include <unistd.h>
#include <sys/wait.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>  
#include <cstring>

#include "./parsing/HTTPRequestParser.hpp"
#include "./parsing/ConfigFileParsing.hpp"

std::string	call_php_cgi(HTTPRequest request, std::string loc);

#endif
