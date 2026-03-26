/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tests.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathboye <mathboye@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/25 13:37:15 by mathboye          #+#    #+#             */
/*   Updated: 2026/03/26 11:58:27 by mathboye         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TESTS_HPP
# define TESTS_HPP

#include <iostream>
#include "../includes/Socket.hpp"
#include "../includes/parsing/ConfigFileParsing.hpp"
#include "../includes/parsing/HTTPRequestsParsing.hpp"
#include "../includes/UI.hpp"

void	display_parsed_conf_file(const ConfigFileParsing &config);

#endif
