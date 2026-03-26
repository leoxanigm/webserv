/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tests.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zalemu <zalemu@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/25 13:37:15 by mathboye          #+#    #+#             */
/*   Updated: 2026/03/26 12:27:27 by zalemu           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TESTS_HPP
# define TESTS_HPP

#include <iostream>
#include "../includes/Socket.hpp"
#include "../includes/parsing/ConfigFileParsing.hpp"
#include "../includes/UI.hpp"

void	display_parsed_conf_file(const ConfigFileParsing &config);
void	testHttpParsing(void);

#endif
