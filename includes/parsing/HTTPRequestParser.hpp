/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequestParser.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathboye <mathboye@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/17 23:10:00 by mathboye          #+#    #+#             */
/*   Updated: 2026/03/17 23:10:00 by mathboye         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUESTPARSER_HPP
# define HTTPREQUESTPARSER_HPP

# include <map>
# include <string>
#include <cerrno>
#include <clocale>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>

typedef enum e_http_methods
{
	GET,
	POST,
	DELETE
}	t_http_methods;

class HTTPRequest
{
	private:
		// Headers
		t_http_methods						_method;
		std::string							_requestTarget;
		std::string							_httpVersion;
		std::string							_host;
		std::string							_userAgent;
		std::map<std::string, float>		_languages;
		std::map<std::string, std::string>	_accept;
		bool								_valid;
		std::string							_error;

		// POST request specific datas
		std::string							_transfert_encoding;
		int									_contentLength;
		std::map<std::string, std::string>	_contentType;  // REFACTORED : <type, charset> : <application/x-www-form-urlencoded, UTF-8>
		std::string							_body;

		bool		parseMethodToken(const std::string &methodToken);
		bool		parseRequestLine(const std::string &line);
		bool		parseAcceptLanguage(const std::string &value);
		bool		parseContentType(const std::string &value);
		bool		parseAcceptHeader(const std::string &value);
		bool		parseHeaderLine(const std::string &line);
		void		parseRaw(const std::string &raw);
		void		parseContentLength(const std::string &value);

	public:
		HTTPRequest();
		HTTPRequest(const std::string &raw);
		HTTPRequest(const HTTPRequest &other);
		HTTPRequest	&operator=(const HTTPRequest &other);
		~HTTPRequest();

		bool								isValid() const;
		const std::string					&getError() const;

		void		setMethod(t_http_methods method);
		t_http_methods	getMethod() const;

		void		setRequestTarget(const std::string &target);
		const std::string	&getRequestTarget() const;

		void		setHTTPVersion(const std::string &version);
		const std::string	&getHTTPVersion() const;

		void		setHost(const std::string &host);
		const std::string	&getHost() const;

		void		setUserAgent(const std::string &agent);
		const std::string	&getUserAgent() const;

		void		setTransfertEncoding(const std::string &encoding);
		const std::string	&getTransfertEncoding() const;

		void		setBody(const std::string &body);
		const std::string	&getBody() const;

		void		setLanguages(const std::map<std::string, float> &languages);
		const std::map<std::string, float>		&getLanguages() const;

		void		setAccept(const std::map<std::string, std::string> &accept);
		const std::map<std::string, std::string>	&getAccept() const;

		void		setContentType(const std::map<std::string, std::string> &content);
		const std::map<std::string, std::string>	&getContentType() const;

		void		setContentLength(const int &length);
		const int	&getContentLength() const;
};

#endif
