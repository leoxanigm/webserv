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

HTTPRequest::HTTPRequest()
	: _method(GET), _valid(true), _headerComplete(false), _bodyComplete(false), _error(0)
{
}

HTTPRequest::HTTPRequest(const std::string &raw) : _method(GET),
												   _valid(true),
												   _headerComplete(false),
												   _bodyComplete(false),
												   _contentLength(0),
												   _hasContentLength(false)
{
	parseRaw(raw);
}

HTTPRequest::HTTPRequest(const HTTPRequest &other)
{
	*this = other;
}

HTTPRequest &HTTPRequest::operator=(const HTTPRequest &other)
{
	if (this == &other)
		return (*this);
	_method = other._method;
	_requestTarget = other._requestTarget;
	_httpVersion = other._httpVersion;
	_host = other._host;
	_userAgent = other._userAgent;
	_languages = other._languages;
	_cookies = other._cookies;
	_accept = other._accept;
	_contentType = other._contentType;
	_transfert_encoding = other._transfert_encoding;
	_contentLength = other._contentLength;
	_hasContentLength = other._hasContentLength;
	_headerComplete = other._headerComplete;
	_bodyComplete = other._bodyComplete;
	_body = other._body;
	_valid = other._valid;
	_error = other._error;
	return (*this);
}

HTTPRequest::~HTTPRequest()
{
}

void HTTPRequest::setMethod(t_http_methods method)
{
	_method = method;
}

t_http_methods HTTPRequest::getMethod() const
{
	return (_method);
}

void HTTPRequest::setRequestTarget(const std::string &target)
{
	_requestTarget = target;
}

bool HTTPRequest::isValid() const
{
	return (_valid);
}

bool HTTPRequest::isComplete() const
{
	if (_method == GET || _method == DELETE)
		return (_headerComplete);
	if (_method == POST)
		return (_headerComplete && _bodyComplete);
	return (false);
}

const unsigned int &HTTPRequest::getError() const
{
	return (_error);
}

const std::string &HTTPRequest::getRequestTarget() const
{
	return (_requestTarget);
}

void HTTPRequest::setHTTPVersion(const std::string &version)
{
	_httpVersion = version;
}

const std::string &HTTPRequest::getHTTPVersion() const
{
	return (_httpVersion);
}

const std::string &HTTPRequest::getHost() const
{
	return (_host);
}

void HTTPRequest::setHost(const std::string &host)
{
	_host = host;
}

std::size_t HTTPRequest::getContentLength() const
{
	return (_contentLength);
}

void HTTPRequest::setContentLength(const int &length)
{
	_contentLength = length;
}

void HTTPRequest::setUserAgent(const std::string &agent)
{
	_userAgent = agent;
}

const std::string &HTTPRequest::getUserAgent() const
{
	return (_userAgent);
}

void HTTPRequest::setBody(const std::string &body)
{
	_body = body;
}

const std::string &HTTPRequest::getBody() const
{
	return (_body);
}

const std::string &HTTPRequest::getTransfertEncoding() const
{
	return (_transfert_encoding);
}

void HTTPRequest::setTransfertEncoding(const std::string &encoding)
{
	_transfert_encoding = encoding;
}

void HTTPRequest::setLanguages(const std::map<std::string, float> &languages)
{
	_languages = languages;
}

const std::map<std::string, float> &HTTPRequest::getLanguages() const
{
	return (_languages);
}

void HTTPRequest::setContentType(const std::map<std::string, std::string> &contentType)
{
	_contentType = contentType;
}

const std::map<std::string, std::string> &HTTPRequest::getContentType() const
{
	return (_contentType);
}

void HTTPRequest::setCookie(const std::map<std::string, std::string> &cookie)
{
	_cookies = cookie;
}

const std::map<std::string, std::string> &HTTPRequest::getCookie() const
{
	return (_cookies);
}

void HTTPRequest::setAccept(const std::map<std::string, std::string> &accept)
{
	_accept = accept;
}

const std::map<std::string, std::string> &HTTPRequest::getAccept() const
{
	return (_accept);
}

std::string HTTPRequest::percent_decode(const std::string &in)
{
	std::string out;

	for (std::string::size_type i = 0; i < in.size(); ++i)
	{
		char c = in[i];

		if (c == '%')
		{
			if (i + 2 >= in.size())
				return ("");

			char h1 = in[i + 1];
			char h2 = in[i + 2];

			int v1;
			if (h1 >= '0' && h1 <= '9')
				v1 = h1 - '0';
			else if (h1 >= 'A' && h1 <= 'F')
				v1 = h1 - 'A' + 10;
			else if (h1 >= 'a' && h1 <= 'f')
				v1 = h1 - 'a' + 10;
			else
				v1 = -1;

			int v2;
			if (h2 >= '0' && h2 <= '9')
				v2 = h2 - '0';
			else if (h2 >= 'A' && h2 <= 'F')
				v2 = h2 - 'A' + 10;
			else if (h2 >= 'a' && h2 <= 'f')
				v2 = h2 - 'a' + 10;
			else
				v2 = -1;

			if (v1 < 0 || v2 < 0)
				return ("");

			out.push_back(static_cast<char>((v1 << 4) | v2)); // Bitwise operator to go from hex to char value
			i += 2;
		}
		else
			out.push_back(c);
	}
	return (out);
}

bool HTTPRequest::parseMethodToken(const std::string &methodToken)
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

bool HTTPRequest::parseRequestLine(const std::string &line)
{
	std::string::size_type s1 = line.find(' ');
	if (utils::ft_count(line, ' ') > 2 || s1 == std::string::npos)
	{
		_error = 400; // invalid request line
		return (false);
	}
	std::string::size_type s2 = line.find(' ', s1 + 1);
	if (s2 == std::string::npos)
	{
		_error = 400; // invalid request line
		return (false);
	}

	std::string methodToken = line.substr(0, s1);
	std::string target = line.substr(s1 + 1, s2 - s1 - 1);
	std::string version = line.substr(s2 + 1);

	if (methodToken.empty() || target.empty() || version.empty())
	{
		_error = 400; // invalid request line
		return (false);
	}
	if (!parseMethodToken(methodToken))
	{
		_error = 405; // unsupported method
		return (false);
	}
	if (target[0] != '/')
	{
		_error = 400; // invalid request target
		return (false);
	}
	if (version.compare("HTTP/1.0") != 0 && version.compare("HTTP/1.1") != 0)
	{
		_error = 400; // invalid http version
		return (false);
	}

	_requestTarget = percent_decode(target);
	// std::cout << "R Target : " << _requestTarget << std::endl;
	if (_requestTarget.empty())
	{
		_error = 500; // Internal error, malformed encoded target
		return (false);
	}
	_httpVersion = version;
	return (true);
}

bool HTTPRequest::parseAcceptLanguage(const std::string &value)
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

bool HTTPRequest::parseAcceptHeader(const std::string &value)
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

bool HTTPRequest::parseContentType(const std::string &value)
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
	if (toLower(mediaType) == "multipart/form-data" && _contentType.find("boundary") == _contentType.end())
		return (false);
	return (true);
}

void HTTPRequest::parseContentLength(const std::string &value)
{
	if (!utils::ft_str_is_digits(value))
	{
		_contentLength = 0;
		_hasContentLength = false;
		return;
	}
	char *end = NULL;
	long parsed = std::strtol(value.c_str(), &end, 10);
	if (end == value.c_str() || *end != '\0' || parsed < 0 || parsed > INT_MAX)
	{
		_contentLength = 0;
		_hasContentLength = false;
		return;
	}
	_contentLength = static_cast<std::size_t>(parsed);
	_hasContentLength = true;
}

bool HTTPRequest::parseCookie(const std::string &value)
{
	std::string::size_type start = 0;

	while (start < value.size())
	{
		std::string::size_type end = value.find(';', start);
		if (end == std::string::npos)
			end = value.size();

		std::string pair = trim(value.substr(start, end - start));
		if (pair.empty())
		{
			start = end + 1;
			continue;
		}
		std::string::size_type eq = pair.find('=');
		if (eq == std::string::npos)
			return (false);

		std::string name = trim(pair.substr(0, eq));
		std::string val = trim(pair.substr(eq + 1));
		if (name.empty())
			return (false);
		_cookies[name] = val;
		start = end + 1;
	}
	return (true);
}

bool HTTPRequest::parseHeaderLine(const std::string &line)
{
	std::string::size_type sep = line.find(':');
	if (sep == std::string::npos)
	{
		_error = 400; // invalid header line
		return (false);
	}

	std::string key = trim(line.substr(0, sep));
	std::string value = trim(line.substr(sep + 1));
	if (key.empty())
	{
		_error = 400; // invalid header key
		return (false);
	}

	std::string keyLower = toLower(key);

	if (keyLower == "host")
	{
		if (!_host.empty())
		{
			_error = 400; // duplicate host header
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
	if (keyLower == "cookie")
		return (parseCookie(value));
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
		if (_hasContentLength)
		{
			_error = 400; // duplicate host header
			return (false);
		}
		parseContentLength(value);
		return (_hasContentLength);
	}
	return (true);
}

/*
	* @brief
		This function parses a raw HTTP request and puts its values into the dedicated class.
	* @param `raw`
		The raw request formated with CRLF at the end on each line.
*/
void HTTPRequest::parseRaw(const std::string &raw)
{
	std::string::size_type body_begin = raw.find("\r\n\r\n");
	std::istringstream tmpBody(raw.substr(body_begin + 4));

	if (body_begin == std::string::npos)
	{
		_valid = false;
		_error = 400; // missing header/body separator
		return;
	}

	_headerComplete = true;
	if (tmpBody.str().find("\r\n\r\n") != std::string::npos || tmpBody.str().size() >= _contentLength)
		_bodyComplete = true;

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
				return;
			}
			continue;
		}

		if (line.empty())
			break;

		if (!parseHeaderLine(line))
		{
			_valid = false;
			return;
		}
	}

	if (_host.empty())
	{
		_valid = false;
		_error = 400; // missing header/body separator
		return;
	}
	if (!_transfert_encoding.empty() && _contentLength > 0)
	{
		_valid = false;
		_error = 400; // both content-length and transfer-encoding are set
		return;
	}

	std::string bodyLine;
	int lineNum = 0;
	int length = -1;

	// GET /index.html HTTP/1.1\r\nHost: localhost\r\nAccept: text/html\r\nTransfer-Encoding: chunked\r\n\r\n7\r\nWelcome\r\n1c\r\nto Mozilla Developer Network\r\n0\r\n\r\n
	if (toLower(_transfert_encoding) == "chunked")
	{
		while (std::getline(tmpBody, bodyLine) && length != 0)
		{
			if (!bodyLine.empty() && bodyLine[bodyLine.size() - 1] == '\r')
				bodyLine.erase(bodyLine.size() - 1);
			if (lineNum % 2 == 0)
			{
				char *end;
				length = std::strtol(bodyLine.c_str(), &end, 16);
				if (end == bodyLine.c_str() || *end != '\0' || length < 0)
				{
					_valid = false;
					_error = 400; // invalid chunked encoding
					return;
				}
			}
			else if (lineNum % 2 == 1)
			{
				_body += bodyLine;
			}
			lineNum++;
		}
		if (length != 0)
		{
			_valid = false;
			_error = 400; // invalid chunked body
			return;
		}

		_bodyComplete = true;
	}
	else
	{
		_body = tmpBody.str();

		if (_body.length() != _contentLength)
		{
			_valid = false;
			_error = 400; // content length is not the same length as the actual body
			return;
		}
	}
}
