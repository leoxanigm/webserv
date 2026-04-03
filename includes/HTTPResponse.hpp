#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <string>
#include <vector>
#include <map>

#include "./HTTPError.hpp"
#include "./parsing/HTTPRequestParser.hpp"
#include "./parsing/ConfigFileParsing.hpp"

// Date
// Server
// Content-Type
// Content-Length
// Location
// Connection
// Allow
// Content-Language
// Content-Location
// Last-Modified
// Set-Cookie
// Retry-After
// WWW-Authenticate
// Transfer-Encoding if ever supported
// CGI-generated headers

class HTTPResponse
{
public:
	HTTPResponse();
	~HTTPResponse();

	enum Code
	{
		OK = 200,
		CREATED = 201,
		// using 308 instead of 301 as we want the user agent to use the same
		// request method on the redirect URI
		// send Location attribute with the new data location
		PERMANENT_REDIRECT = 308,
	};

	void setStatusCode(int code);
	void setHeader(const std::string &key, const std::string &value);
	void setBody(const std::string &body);
	void setBody(const std::vector<char> &body);

	std::string toString() const;

private:
	HTTPResponse(const HTTPResponse &obj);
	HTTPResponse &operator=(const HTTPResponse &obj);

	int _statusCode;
	std::string _httpVersion;
	std::map<std::string, std::string> _headers;
	std::string _body;

	std::string _statusLineFromCode(void) const;
	std::string _dateGMT() const;
};

// class CGIHandler
// {
// public:
// 	CGIHandler(const HTTPRequest &req, const ConfigFileParsing &config);
// 	~CGIHandler();

// 	void execute();
// 	int getReadFd() const;
// 	int getWriteFd() const;
// 	pid_t getPid() const;
// };

#endif /* HTTP_RESPONSE_HPP */
