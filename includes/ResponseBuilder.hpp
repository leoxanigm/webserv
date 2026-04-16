#ifndef RESPONSE_BUILDER_HPP
#define RESPONSE_BUILDER_HPP

#include "./parsing/HTTPRequestParser.hpp"
#include "./parsing/ConfigFileParsing.hpp"
#include "./HTTPResponse.hpp"

class ResponseBuilder
{
public:
	ResponseBuilder(
		const HTTPRequest &req, const ConfigFileParsing::server_t &server);
	~ResponseBuilder();

	void buildErrorRes(int code);
	std::string str() const;
	bool isCGI() const;
	const ConfigFileParsing::location_t *getLocation() const;

	static std::string _getMimeType(const std::string &path);
	static std::string _resolvePath(
		const std::string &reqURI, const ConfigFileParsing::location_t *location);
	static std::string _stripQueryString(const std::string &uri);
	static std::string _getRequestMediaType(
		const std::map<std::string, std::string> &contentType);
	static bool _parseMultipartFile(
		const std::string &body,
		const std::string &boundary,
		std::string &outFilename,
		std::string &outContent);

private:
	ResponseBuilder();
	ResponseBuilder(const ResponseBuilder &obj);
	ResponseBuilder &operator=(const ResponseBuilder &obj);

	const HTTPRequest &_request;
	const ConfigFileParsing::location_t *_location;
	const ConfigFileParsing::server_t &_server;
	HTTPResponse _response;

	bool _isCGI;

	void _processGET(const ConfigFileParsing::location_t *location);
	void _processPOST(const ConfigFileParsing::location_t *location);
	void _processDELETE(const ConfigFileParsing::location_t *location);
	void _directoryListing(const std::string &reqLocation, const std::string &fullPath);
	const std::string &_getErrorPage(int code) const;
	const ConfigFileParsing::location_t *_longestPrefixLocationMatch(
		const std::string &reqLocation) const;
	bool _isValidCGIScript() const;
};

#endif /* RESPONSE_BUILDER_HPP */
