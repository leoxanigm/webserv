#include <string>
#include <sstream>
#include <stdexcept>
#include <ctime>

#include "../includes/HTTPError.hpp"
#include "../includes/HTTPResponse.hpp"

HTTPResponse::HTTPResponse()
{
	_statusCode = 200;
	_httpVersion = "HTTP/1.1";
	_body = "";
}

HTTPResponse::~HTTPResponse() {}

void HTTPResponse::setStatusCode(int code)
{
	if (code < 200 || code > 505)
		throw std::invalid_argument("Error: setStatusCode: code must be >= 200 and <= 505");
	_statusCode = code;
}

void HTTPResponse::setHeader(const std::string &key, const std::string &value)
{
	if (key != "Content-Length" && key != "Date" && key != "Connection")
		_headers[key] = value;
}

void HTTPResponse::setBody(const std::string &body)
{
	_body = body;
}

void HTTPResponse::setBody(const std::vector<char> &body)
{
	_body = std::string(body.begin(), body.end());
}

std::string HTTPResponse::toString() const
{
	std::ostringstream header;

	header << _statusLineFromCode() << "\r\n";
	header << "Date: " << _dateGMT() << "\r\n";
	
	if (_statusCode != 204)
		header << "Content-Length: " << _body.size() << "\r\n";

	header << "Connection: close\r\n";

	std::map<std::string, std::string>::const_iterator it = _headers.begin();

	for (; it != _headers.end(); it++)
		header << it->first << ": " << it->second << "\r\n";

	header << "\r\n";
	header << _body;

	return (header.str());
}

std::string HTTPResponse::_statusLineFromCode(void) const
{
	std::ostringstream statusLine;

	statusLine << _httpVersion << " ";

	if (_statusCode >= 400 && _statusCode <= 505) // error
	{
		HTTPError errorStatus(_statusCode);
		statusLine << errorStatus.what();
	}
	else // success or redirect
	{
		switch (_statusCode)
		{
		case OK:
			statusLine << "200 OK";
			break;
		case CREATED:
			statusLine << "201 Created";
			break;
		case 303:
			statusLine << "303 See Other";
			break;
		case PERMANENT_REDIRECT:
			statusLine << "308 Permanent Redirect";
			break;
		default:
			statusLine << "500 Internal Server Error";
			break;
		}
	}

	return (statusLine.str());
}

std::string HTTPResponse::_dateGMT() const
{
	std::time_t now = std::time(0);
	std::tm *gmt = std::gmtime(&now);
	char buff[30];

	std::strftime(buff, sizeof(buff), "%a, %d %b %Y %H:%M:%S GMT", gmt);
	return (std::string(buff));
}
