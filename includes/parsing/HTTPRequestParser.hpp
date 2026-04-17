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
#include <clocale>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>

// TODO:  Done
// // Check invalid URI

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
		std::string							_path;
    	std::string							_queryString;
		std::string							_httpVersion;
		std::string							_host;
		std::string							_userAgent;
		std::map<std::string, float>		_languages;
		std::map<std::string, std::string>	_cookies;				// <name, value>
		std::map<std::string, std::string>	_accept;
		bool								_valid;
		bool								_headerComplete;
		bool								_bodyComplete;
		unsigned int						_error;

		// POST request specific datas
		std::string							_transfert_encoding;
		std::size_t							_contentLength;
		bool								_hasContentLength;
		std::map<std::string, std::string>	_contentType;			// REFACTORED : <type, charset> : <application/x-www-form-urlencoded, UTF-8>
		std::string							_body;

		bool		parseMethodToken(const std::string &methodToken);
		bool		parseRequestTarget(const std::string &target);
		bool		parseRequestLine(const std::string &line);
		bool		parseAcceptLanguage(const std::string &value);
		bool		parseContentType(const std::string &value);
		bool		parseAcceptHeader(const std::string &value);
		bool		parseHeaderLine(const std::string &line);
		void		parseRaw(const std::string &raw);
		void		parseContentLength(const std::string &value);
		bool		parseCookie(const std::string &value);

		std::string	percent_decode(const std::string &in);

	public:
		HTTPRequest();
		HTTPRequest(const std::string &raw);
		HTTPRequest(const HTTPRequest &other);
		HTTPRequest	&operator=(const HTTPRequest &other);
		~HTTPRequest();

		bool								isValid() const;
		bool								isComplete() const;
		const unsigned int					&getError() const;

		void		setMethod(t_http_methods method);
		t_http_methods	getMethod() const;

		void		setRequestTarget(const std::string &target);
		const std::string	&getRequestTarget() const;
		const std::string	&getPath() const;
    	const std::string	&getQueryString() const;

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

		void		setCookie(const std::map<std::string, std::string> &content);
		const std::map<std::string, std::string>	&getCookie() const;

		void		setContentLength(const int &length);
		std::size_t	getContentLength() const;
};

#endif
