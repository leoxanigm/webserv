#include <sstream>
#include <ctime>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#include "../includes/ResponseBuilder.hpp"
#include "../includes/HTTPError.hpp"
#include "../includes/parsing/HTTPRequestParser.hpp"
#include "../includes/parsing/ConfigFileParsing.hpp"
#include "../includes/utils.hpp"

ResponseBuilder::ResponseBuilder(
	const HTTPRequest &req, const ConfigFileParsing::server_t &server)
	: _request(req), _server(server)
{
	_isCGI = false;

	if (!_request.isValid())
	{
		_response.setStatusCode(_request.getError());
		_response.setBody(utils::readFile(_getErrorPage(_request.getError())));
	}
	else
	{
		const ConfigFileParsing::location_t *location = 0;
		location = _longestPrefixLocationMatch(_request.getRequestTarget());
		if (location == 0)
		{
			_response.setStatusCode(HTTPError::NOT_FOUND);
			_response.setBody(utils::readFile(_getErrorPage(HTTPError::NOT_FOUND)));
		}
		else
		{
			// TODO
			// handle query strings
			// static file
			// directory index
			// autoindex
			// CGI
			// upload
			// delete
			// error

			// handle redirect first
			if (location->return_code == 301 || location->return_code == 308)
			{
				_response.setStatusCode(HTTPResponse::PERMANENT_REDIRECT);
				_response.setHeader("Location", location->return_location);
			}
			else
			{
				try
				{
					if (_request.getMethod() == GET)
						_processGET(location);
					if (_request.getMethod() == POST)
						_processPOST(location);
					if (_request.getMethod() == DELETE)
						_processDELETE(location);
				}
				catch (const HTTPError &e)
				{
					_response.setStatusCode(e.code());
					_response.setBody(utils::readFile(_getErrorPage(e.code())));
				}
				catch (const std::exception &e)
				{
					_response.setStatusCode(HTTPError::INTERNAL_SERVER_ERROR);
					_response.setBody(utils::readFile(_getErrorPage(HTTPError::INTERNAL_SERVER_ERROR)));
				}
			}
		}
	}
}

ResponseBuilder::~ResponseBuilder() {}

std::string ResponseBuilder::str() const
{
	return (_response.toString());
}

bool ResponseBuilder::isCGI() const
{
	return (_isCGI);
}

void ResponseBuilder::_processGET(const ConfigFileParsing::location_t *location)
{
	if (location->allowed_methods.count("GET") == 0)
	{
		_response.setStatusCode(HTTPError::METHOD_NOT_ALLOWED);
		_response.setBody(utils::readFile(_getErrorPage(HTTPError::METHOD_NOT_ALLOWED)));
		return;
	}

	std::string path = _resolvePath(_request.getRequestTarget(), location);

	if (location->is_cgi)
	{
	}
	else
	{
		if (_request.getRequestTarget()[_request.getRequestTarget().size() - 1] == '/')
		{
			if (location->autoindex)
				_directoryListing();
			else
			{
				std::cout << "here" << '\n';
				_response.setStatusCode(HTTPError::FORBIDDEN);
				try
				{
					_response.setBody(utils::readFile(_getErrorPage(HTTPError::FORBIDDEN)));
				}
				catch (const std::exception &e)
				{
					// For error pages, not being able to read the file is unrecoverable.
					std::cerr << "Error setting body in ResponseBuilder::_processGET: "
							  << e.what() << '\n';
				}
			}
		}
	}
}

void ResponseBuilder::_processPOST(const ConfigFileParsing::location_t *location)
{
	if (location->allowed_methods.count("POST") == 0)
	{
		_response.setStatusCode(HTTPError::METHOD_NOT_ALLOWED);
		_response.setBody(utils::readFile(_getErrorPage(HTTPError::METHOD_NOT_ALLOWED)));
		return;
	}
}

void ResponseBuilder::_processDELETE(const ConfigFileParsing::location_t *location)
{
	if (location->allowed_methods.count("DELETE") == 0)
	{
		_response.setStatusCode(HTTPError::METHOD_NOT_ALLOWED);
		_response.setBody(utils::readFile(_getErrorPage(HTTPError::METHOD_NOT_ALLOWED)));
		return;
	}
}

void ResponseBuilder::_directoryListing()
{
	// location should have been properly constructed here.
	// For example, if url was /files and in config if /files was
	// routed to ./www/html/files,
	// _request.getReqLocation() == /files/
	// _request.getFullPath() == ./www/html/files/
	// This make sure we separate error checking concern from response
	// const std::string &reqLocation _request.getReqLocation(); // TODO
	// const std::string &fullPath _request.getFullPath(); // TODO

	const std::string reqLocation = "/files/";
	const std::string fullPath = "./www/html/files/";

	// TODO
	// * Check if directory listing allowed for the current fullPath in config
	// 	 if not return 403 Forbidden
	// * Add method to get default pages from _config
	//	 - add a private method to read from default page and return string

	errno = 0;
	DIR *directory = opendir(fullPath.c_str());
	if (!directory)
	{
		if (errno == EACCES || errno == ENOTDIR)
			_response.setStatusCode(HTTPError::FORBIDDEN);
		// _response.setBody(_readFile(_config.getDefaultPage(HTTPError::FORBIDDEN)));
		else
			_response.setStatusCode(HTTPError::INTERNAL_SERVER_ERROR);
		// _response.setBody(_readFile(_config.getDefaultPage(HTTPError::FORBIDDEN)));

		return;
	}

	std::string title = "Index of " + reqLocation;

	std::string parentDir;
	if (reqLocation[reqLocation.size() - 1] == '/')
		parentDir = reqLocation.substr(0, reqLocation.size() - 1);
	else
		parentDir = reqLocation;

	parentDir = parentDir.substr(0, parentDir.find_last_of("/"));
	if (parentDir.empty())
		parentDir = "/";

	std::ostringstream output;
	output << "<!DOCTYPE html><html><head><title>" << title
		   << "</title></head><body><h1>" << title
		   << "</h1><table><tr><th>Name</th><th>Last modified</th>"
		   << "<th>Size</th></tr><tr><th colspan=\"3\"><hr /></th></tr>"
		   << "<tr><td><a href=\"" << parentDir << "\">Parent Directory</a></td>"
		   << "<td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr>";

	struct dirent *dirIt = 0;
	struct stat statBuff;
	struct std::tm *tm;
	char dateStr[256];
	std::string filePath;
	std::string href;
	while ((dirIt = readdir(directory)) != 0)
	{
		if (std::strcmp(dirIt->d_name, ".") == 0 ||
			std::strcmp(dirIt->d_name, "..") == 0)
			continue;

		filePath = fullPath + std::string(dirIt->d_name);
		if (stat(filePath.c_str(), &statBuff) == -1)
			continue;

		output << "<tr><td><a href=\"" << dirIt->d_name;

		if (S_ISDIR(statBuff.st_mode)) // If directory add `/` to the href
			output << '/';

		output << "\">"
			   << dirIt->d_name << "</a></td>";

		tm = std::localtime(&statBuff.st_mtime);
		std::strftime(dateStr, sizeof(dateStr), "%Y-%m-%d %H:%M", tm);

		output << "<td>" << dateStr << "</td><td align=\"right\">";

		if (S_ISREG(statBuff.st_mode)) // If regular file, show size
			output << static_cast<float>(statBuff.st_size) / 1000 << "K\n";
		else
			output << " - \n";

		output << "</td></tr>";
	}

	output << "</table></body></html>";
	_response.setBody(output.str());

	closedir(directory);
}

const std::string &ResponseBuilder::_getErrorPage(int code) const
{
	switch (code)
	{
	case 200:
		return (_server.error_pages.OK);
	case 204:
		return (_server.error_pages.NoContent);
	case 301:
		return (_server.error_pages.MovedPermanently);
	case 302:
		return (_server.error_pages.MovedTemporarily);
	case 400:
		return (_server.error_pages.BadRequest);
	case 401:
		return (_server.error_pages.Unauthorized);
	case 403:
		return (_server.error_pages.Forbidden);
	case 404:
		return (_server.error_pages.NotFound);
	case 405:
		return (_server.error_pages.MethodNotAllowed);
	case 500:
		return (_server.error_pages.InternalServerError);
	case 501:
		return (_server.error_pages.NotImplemented);
	case 502:
		return (_server.error_pages.BadGateway);
	case 503:
		return (_server.error_pages.ServiceUnavailable);
	}

	return (_server.error_pages.InternalServerError);
}

/**
 * Searches for a longest prefix route from our list of locations and
 * returns a pointer to the location struct.
 * For example, in config, if we have location for `/` and `/images`
 * we match /images/logo.png to `/images`. If we don't have `/images`
 * location, we match it to `/`.
 */
const ConfigFileParsing::location_t *
ResponseBuilder::_longestPrefixLocationMatch(const std::string &reqURI) const
{
	const ConfigFileParsing::location_t *location = 0;
	std::size_t locationLen = 0;

	ConfigFileParsing::location_iterator_t it = _server.locations.begin();
	for (; it != _server.locations.end(); ++it)
	{
		const std::string &loc = it->location;
		std::size_t len = loc.size();

		if (reqURI.compare(0, len, loc) != 0)
			continue;

		// Handle root match as special case
		if (loc == "/" && !reqURI.empty() && reqURI[0] == '/' && len > locationLen)
		{
			location = &(*it);
			locationLen = len;
			continue;
		}

		bool exactMatch = (reqURI.size() == len);
		bool boundaryMatch = (!exactMatch && reqURI[len] == '/');

		if ((exactMatch || boundaryMatch) && len > locationLen)
		{
			location = &(*it);
			locationLen = len;
		}
	}

	return (location);
}

std::string ResponseBuilder::_getMimeType(const std::string &path)
{
	std::string::size_type slash = path.find_last_of('/');
	std::string::size_type dot = path.find_last_of('.');

	if (dot == std::string::npos || (slash != std::string::npos && dot < slash))
		return "application/octet-stream";

	std::string ext = path.substr(dot + 1);

	if (ext == "html" || ext == "htm")
		return "text/html";
	if (ext == "css")
		return "text/css";
	if (ext == "js")
		return "application/javascript";
	if (ext == "json")
		return "application/json";
	if (ext == "txt")
		return "text/plain";
	if (ext == "jpg" || ext == "jpeg")
		return "image/jpeg";
	if (ext == "png")
		return "image/png";
	if (ext == "gif")
		return "image/gif";
	if (ext == "svg")
		return "image/svg+xml";
	if (ext == "ico")
		return "image/x-icon";
	if (ext == "pdf")
		return "application/pdf";
	if (ext == "mp4")
		return "video/mp4";
	if (ext == "webm")
		return "video/webm";

	return "application/octet-stream";
}

/**
 * Construct resolved file path from request URI and route root.
 * For example, for root = ./www/site/images and URI = /images/logo.png
 * resolved path = ./www/site/images/logo.png
 */
std::string ResponseBuilder::_resolvePath(
	const std::string &reqURI, const ConfigFileParsing::location_t *location)
{
	std::string suffix = reqURI.substr(location->location.size());

	if (!suffix.empty() && suffix[0] != '/')
		suffix.insert(suffix.begin(), '/');

	if (!location->root.empty() &&
		location->root[location->root.size() - 1] == '/' &&
		!suffix.empty() && suffix[0] == '/')
		return (location->root + suffix.substr(1));

	if (!location->root.empty() &&
		location->root[location->root.size() - 1] != '/' &&
		!suffix.empty() && suffix[0] != '/')
		return (location->root + "/" + suffix);

	return (location->root + suffix);
}
