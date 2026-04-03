#ifndef RESPONSE_BUILDER_HPP
#define RESPONSE_BUILDER_HPP

#include "./parsing/HTTPRequestParser.hpp"
#include "./parsing/ConfigFileParsing.hpp"
#include "./HTTPResponse.hpp"

class ResponseBuilder
{
public:
	ResponseBuilder(const HTTPRequest &req, const ConfigFileParsing &config);
	~ResponseBuilder();

	const std::string &getBody() const;

private:
	ResponseBuilder(const ResponseBuilder &obj);
	ResponseBuilder &operator=(const ResponseBuilder &obj);

	const HTTPRequest &_req;
	const ConfigFileParsing &_config;
	HTTPResponse _res;

	std::string _body;

	void _directoryListing();

	static std::string _getMimeType(const std::string &path);
};



#endif /* RESPONSE_BUILDER_HPP */
