/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tests.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathboye <mathboye@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/25 13:37:15 by mathboye          #+#    #+#             */
/*   Updated: 2026/04/03 13:54:59 by mathboye         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TESTS_HPP
# define TESTS_HPP

#include <iostream>
#include "./Socket.hpp"
#include "./parsing/ConfigFileParsing.hpp"
#include "./parsing/HTTPRequestParser.hpp"
#include "../includes/UI.hpp"

void	display_parsed_conf_file(const ConfigFileParsing &config);
void	display_parsed_request(const HTTPRequest &request);
void	testHttpParsing(void);
void	testConfParsing(void);

#endif
