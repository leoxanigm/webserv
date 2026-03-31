/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequestParser.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathboye <mathboye@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/17 23:10:00 by mathboye          #+#    #+#             */
/*   Updated: 2026/03/17 23:10:00 by mathboye         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/parsing/HTTPRequestParser.hpp"
#include "../../includes/parsing/parsing_utils.hpp"
#include "../../includes/utils.hpp"

#include <climits>

HTTPRequest::HTTPRequest() : _method(GET), _valid(true), _error("")
{}

HTTPRequest::HTTPRequest(const std::string &raw) : _method(GET),
													_valid(true),
													_contentLength(0)
{
	parseRaw(raw);
}

HTTPRequest::HTTPRequest(const HTTPRequest &other)
{
	*this = other;
}

HTTPRequest	&HTTPRequest::operator=(const HTTPRequest &other)
{
	if (this == &other)
		return (*this);
	_method = other._method;
	_requestTarget = other._requestTarget;
	_httpVersion = other._httpVersion;
	_host = other._host;
	_userAgent = other._userAgent;
	_languages = other._languages;
	_accept = other._accept;
	_contentType = other._contentType;
	_transfert_encoding = other._transfert_encoding;
	_contentLength = other._contentLength;
	_body = other._body;
	_valid = other._valid;
	_error = other._error;
	return (*this);
}

HTTPRequest::~HTTPRequest()
{}

void	HTTPRequest::setMethod(t_http_methods method)
{
	_method = method;
}

t_http_methods	HTTPRequest::getMethod() const
{
	return (_method);
}

void	HTTPRequest::setRequestTarget(const std::string &target)
{
	_requestTarget = target;
}

bool	HTTPRequest::isValid() const
{
	return (_valid);
}

const std::string	&HTTPRequest::getError() const
{
	return (_error);
}

const std::string	&HTTPRequest::getRequestTarget() const
{
	return (_requestTarget);
}

void	HTTPRequest::setHTTPVersion(const std::string &version)
{
	_httpVersion = version;
}

const std::string	&HTTPRequest::getHTTPVersion() const
{
	return (_httpVersion);
}

const std::string	&HTTPRequest::getHost() const
{
	return (_host);
}

void	HTTPRequest::setHost(const std::string &host)
{
	_host = host;
}

const int	&HTTPRequest::getContentLength() const
{
	return (_contentLength);
}

void	HTTPRequest::setContentLength(const int &length)
{
	_contentLength = length;
}

void	HTTPRequest::setUserAgent(const std::string &agent)
{
	_userAgent = agent;
}

const std::string	&HTTPRequest::getUserAgent() const
{
	return (_userAgent);
}

void	HTTPRequest::setBody(const std::string &body)
{
	_body = body;
}

const std::string	&HTTPRequest::getBody() const
{
	return (_body);
}

const std::string	&HTTPRequest::getTransfertEncoding() const
{
	return (_transfert_encoding);
}

void	HTTPRequest::setTransfertEncoding(const std::string &encoding)
{
	_transfert_encoding = encoding;
}

void	HTTPRequest::setLanguages(const std::map<std::string, float> &languages)
{
	_languages = languages;
}

const std::map<std::string, float>	&HTTPRequest::getLanguages() const
{
	return (_languages);
}

void	HTTPRequest::setContentType(const std::map<std::string, std::string> &contentType)
{
	_contentType = contentType;
}

const std::map<std::string, std::string>	&HTTPRequest::getContentType() const
{
	return (_contentType);
}

void	HTTPRequest::setAccept(const std::map<std::string, std::string> &accept)
{
	_accept = accept;
}

const std::map<std::string, std::string>	&HTTPRequest::getAccept() const
{
	return (_accept);
}

bool	HTTPRequest::parseMethodToken(const std::string &methodToken)
{
	if (methodToken == "GET")
	{
		_method = GET;
		return (true);
	}
	if (methodToken == "POST")
	{
		_method = POST;
		return (true);
	}
	if (methodToken == "DELETE")
	{
		_method = DELETE;
		return (true);
	}
	return (false);
}

bool	HTTPRequest::parseRequestLine(const std::string &line)
{
	std::string::size_type s1 = line.find(' ');
	if (utils::ft_count(line, ' ') > 2 || s1 == std::string::npos)
	{
		_error = "invalid request line";
		return (false);
	}
	std::string::size_type s2 = line.find(' ', s1 + 1);
	if (s2 == std::string::npos)
	{
		_error = "invalid request line";
		return (false);
	}

	std::string methodToken = line.substr(0, s1);
	std::string target = line.substr(s1 + 1, s2 - s1 - 1);
	std::string version = line.substr(s2 + 1);

	if (methodToken.empty() || target.empty() || version.empty())
	{
		_error = "invalid request line";
		return (false);
	}
	if (!parseMethodToken(methodToken))
	{
		_error = "unsupported method";
		return (false);
	}
	if (target[0] != '/')
	{
		_error = "invalid request target";
		return (false);
	}
	if (version.compare(0, 5, "HTTP/") != 0)
	{
		_error = "invalid http version";
		return (false);
	}

	_requestTarget = target;
	_httpVersion = version;
	return (true);
}

bool	HTTPRequest::parseAcceptLanguage(const std::string &value)
{
	std::string::size_type start = 0;

	while (start <= value.size())
	{
		std::string::size_type comma = value.find(',', start);
		std::string item = trim(value.substr(start, comma == std::string::npos ? std::string::npos : comma - start));
		if (!item.empty())
		{
			std::string langToken = item;
			float qValue = 1.0f;
			std::string::size_type semi = item.find(';');
			if (semi != std::string::npos)
			{
				langToken = trim(item.substr(0, semi));
				if (langToken.empty())
					return (false);
				std::string qStr;
				std::string::size_type qPos = item.find("q=", semi);
				if (qPos != std::string::npos)
				{
					qStr = trim(item.substr(qPos + 2));
					char *end;
					float q = std::strtof(qStr.c_str(), &end);
					if (end == qStr.c_str() || *end != '\0')
						return (false);
					qValue = q;
				}
				else
					return (false);
			}
			if (!langToken.empty())
				_languages[langToken] = qValue;
			else
				return (false);
		}
		if (comma == std::string::npos)
			break;
		start = comma + 1;
	}
	return (true);
}

bool	HTTPRequest::parseAcceptHeader(const std::string &value)
{
	std::string::size_type start = 0;
	while (start <= value.size())
	{
		std::string::size_type comma = value.find(',', start);
		std::string item = trim(value.substr(start, comma == std::string::npos ? std::string::npos : comma - start));
		if (!item.empty())
		{
			std::string typeToken = item;
			std::string qValue = "";
			std::string::size_type semi = item.find(';');
			if (semi != std::string::npos)
			{
				typeToken = trim(item.substr(0, semi));
				if (typeToken.empty())
					return (false);
				std::string::size_type qPos = item.find("q=", semi);
				if (qPos == std::string::npos)
					return (false);
				std::string qParsed = trim(item.substr(qPos + 2));
				if (qParsed.empty())
					return (false);
				qValue = qParsed;
			}
			if (!typeToken.empty())
				_accept[typeToken] = qValue;
			else
				return (false);
		}
		if (comma == std::string::npos)
			break;
		start = comma + 1;
	}
	if (_accept.find("*/*") == _accept.end())
		_accept["*/*"] = "";
	return (true);
}

bool	HTTPRequest::parseContentType(const std::string &value)


{
	std::string::size_type semi = value.find(';');
	std::string mediaType = trim(value.substr(0, semi));
	if (mediaType.empty())
		return (false);
	_contentType[mediaType] = "";

	if (semi != std::string::npos)
	{
		std::string::size_type start = semi + 1;
		while (start <= value.size())
		{
			std::string::size_type next_semi = value.find(';', start);
			std::string param = trim(value.substr(start, next_semi == std::string::npos ? std::string::npos : next_semi - start));

			if (!param.empty())
			{
				std::string::size_type eq = param.find('=');
				if (eq == std::string::npos)
					return (false);
				std::string pKey = trim(param.substr(0, eq));
				std::string pVal = trim(param.substr(eq + 1));
				if (pKey.empty() || pVal.empty())
					return (false);
				_contentType[pKey] = pVal;
			}

			if (next_semi == std::string::npos)
				break;
			start = next_semi + 1;
		}
	}
	if (toLower(mediaType) == "multipart/form-data"
		&& _contentType.find("boundary") == _contentType.end())
		return (false);
	return (true);
}

void	HTTPRequest::parseContentLength(const std::string &value)


{
	if (!utils::ft_str_is_digits(value))
	{
		_contentLength = -1;
		return ;
	}
	char *end = NULL;
	long parsed = std::strtol(value.c_str(), &end, 10);
	if (end == value.c_str() || *end != '\0' || parsed < 0 || parsed > INT_MAX)
	{
		_contentLength = -1;
		return ;
	}
	_contentLength = static_cast<int>(parsed);
}

bool	HTTPRequest::parseHeaderLine(const std::string &line)


{
	std::string::size_type sep = line.find(':');
	if (sep == std::string::npos)
	{
		_error = "invalid header line";
		return (false);
	}

	std::string key = trim(line.substr(0, sep));
	std::string value = trim(line.substr(sep + 1));
	if (key.empty())
	{
		_error = "invalid header key";
		return (false);
	}

	std::string keyLower = key;
	std::transform(keyLower.begin(), keyLower.end(), keyLower.begin(), ::tolower);

	if (keyLower == "host")
	{
		if (!_host.empty())
		{
			_error = "duplicate host header";
			return (false);
		}
		_host = value;
		return (true);
	}
	if (keyLower == "user-agent")
	{
		if (_userAgent.empty())
			_userAgent = value;
		return (true);
	}
	if (keyLower == "accept-language")
		return (parseAcceptLanguage(value));
	if (keyLower == "accept")
		return (parseAcceptHeader(value));
	if (keyLower == "content-type")
		return (parseContentType(value));
	if (keyLower == "transfer-encoding")
	{
		if (_transfert_encoding.empty())
			_transfert_encoding = toLower(value);
		return (true);
	}
	if (keyLower == "content-length")
	{
		if (_contentLength != 0)
		{
			_error = "duplicate content-length header";
			return (false);
		}
		parseContentLength(value);
		if (_contentLength == -1)
			return (false);
		return (true);
	}
	return (true);
}

/*
	* @brief
		This function parses a raw HTTP request and puts its values into the dedicated class.
	* @param `raw`
		The raw request formated with CRLF at the end on each line.
*/
void	HTTPRequest::parseRaw(const std::string &raw)


{
	std::string::size_type body_begin = raw.find("\r\n\r\n");
	if (body_begin == std::string::npos)
	{
		_valid = false;
		_error = "missing header/body separator";
		return ;
	}

	std::istringstream rawStream(raw.substr(0, body_begin + 2));
	std::string line;
	bool isFirstLine = true;

	while (std::getline(rawStream, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		if (isFirstLine)
		{
			isFirstLine = false;
			if (!parseRequestLine(line))
			{
				_valid = false;
				return ;
			}
			continue ;
		}

		if (line.empty())
			break ;

		if (!parseHeaderLine(line))
		{
			_valid = false;
			return ;
		}
	}

	_body = raw.substr(body_begin + 4);

	if (_host.empty())
	{
		_valid = false;
		_error = "missing host header line";
		return ;
	}
	if (!_transfert_encoding.empty() && _contentLength > 0)
	{
		_valid = false;
		_error = "both content-length and transfer-encoding are set";
		return ;
	}
	if (toLower(_transfert_encoding) == "chunked")
	{
		if (_body.find("\r\n0\r\n\r\n") == std::string::npos)
		{
			_valid = false;
			_error = "invalid chunked body";
		}
		return ;
	}
	if (_body.length() != static_cast<unsigned int>(_contentLength))
	{
		_valid = false;
		_error = "content length is not the same length as the actual body";
		return ;
	}
	if (_method == GET && (!_body.empty() || !_contentType.empty()))
	{
		_valid = false;
		_error = "body in a GET request";
	}
}
