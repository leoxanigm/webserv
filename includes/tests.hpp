/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tests.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathboye <mathboye@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/25 13:37:15 by mathboye          #+#    #+#             */
/*   Updated: 2026/03/26 13:08:20 by mathboye         ###   ########.fr       */
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

#endif
