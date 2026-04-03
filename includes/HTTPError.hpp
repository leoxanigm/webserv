#ifndef HTTP_ERROR_HPP
#define HTTP_ERROR_HPP

#include <string>
#include <exception>

class HTTPError : public std::exception
{
public:
	HTTPError(int code);
	HTTPError(const HTTPError &obj);
	~HTTPError() throw();

	enum Code
	{
		BAD_REQUEST = 400,
		FORBIDDEN = 403,
		NOT_FOUND = 404,
		METHOD_NOT_ALLOWED = 405,
		NOT_ACCEPTABLE = 406,
		LENGTH_REQUIRED = 411,
		PAYLOAD_TOO_LARGE = 413,
		UNSUPPORTED_MEDIA_TYPE = 415,
		RANGE_NOT_SATISFIABLE = 416,
		EXPECTATION_FAILED = 417,
		INTERNAL_SERVER_ERROR = 500,
		NOT_IMPLEMENTED = 501,
		BAD_GATEWAY = 502,
		GATEWAY_TIMEOUT = 504,
		HTTP_VERSION_NOT_SUPPORTED = 505
	};

	const char *what() const throw();
	int code() const throw();

private:
	HTTPError();
	HTTPError &operator=(const HTTPError &obj);

	const int _code;
	const std::string _message;

	static std::string buildErrMsg(int code);
};

#endif /* HTTP_ERROR_HPP */
