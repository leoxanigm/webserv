#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <string>
#include <map>

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
	HTTPResponse(const HTTPRequest &req, const ConfigFileParsing &config);
	~HTTPResponse();

	void setStatusCode(int code);
	void setHTTPVersion(const std::string &version);
	void setHeader(const std::string &key, const std::string &value);
	void setBody(const std::string &body);

	std::string toString();

private:
	HTTPResponse();
	HTTPResponse(const HTTPResponse &obj);
	HTTPResponse &operator=(const HTTPResponse &obj);

	int _statusCode;
	std::string _httpVersion;
	std::map<std::string, std::string> _headers;
	std::string _body;
};

// class ResponseBuilder
// {
// public:
// 	static HTTPResponse build(const HTTPRequest &req, const ConfigFileParsing &config);
// };

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
