#include "../../includes/response/HTTPError.hpp"

HTTPError::HTTPError(int code)
	: _code(code), _message(buildErrMsg(code)) {}

HTTPError::HTTPError(const HTTPError &obj)
	: std::exception(obj), _code(obj._code), _message(obj._message) {}

HTTPError::~HTTPError() throw() {}

const char *HTTPError::what() const throw()
{
	return (_message.c_str());
}

int HTTPError::code() const throw()
{
	return (_code);
}

std::string HTTPError::buildErrMsg(int code)
{
	switch (code)
	{
	case BAD_REQUEST:
		return "400 Bad Request";
	case UNAUTHORIZED:
		return "401 Unauthorized";
	case FORBIDDEN:
		return "403 Forbidden";
	case NOT_FOUND:
		return "404 Not Found";
	case METHOD_NOT_ALLOWED:
		return "405 Method Not Allowed";
	case NOT_ACCEPTABLE:
		return "406 Not Acceptable";
	case REQUEST_TIMEOUT:
		return "408 Request Timeout";
	case LENGTH_REQUIRED:
		return "411 Length Required";
	case PAYLOAD_TOO_LARGE:
		return "413 Payload Too Large";
	case UNSUPPORTED_MEDIA_TYPE:
		return "415 Unsupported Media Type";
	case RANGE_NOT_SATISFIABLE:
		return "416 Range Not Satisfiable";
	case EXPECTATION_FAILED:
		return "417 Expectation Failed";
	case INTERNAL_SERVER_ERROR:
		return "500 Internal Server Error";
	case NOT_IMPLEMENTED:
		return "501 Not Implemented";
	case BAD_GATEWAY:
		return "502 Bad Gateway";
	case SERVICE_UNAVAILABLE:
		return "503 Service Unavailable";
	case GATEWAY_TIMEOUT:
		return "504 Gateway Timeout";
	case HTTP_VERSION_NOT_SUPPORTED:
		return "505 HTTP Version Not Supported";
	default:
		return "Unknown HTTP Error";
	}
}
