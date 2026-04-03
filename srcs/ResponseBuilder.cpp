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
	// TODO:
	// request error checking

	const ConfigFileParsing::location_t *location = 0;
	location = _longestPrefixLocationMatch(_request.getRequestTarget());
	if (location == 0)
	{
		_response.setStatusCode(404);
		// _res.setBody(_pathToBody(_getErrorPage(404)));
	}
	else
	{
		if (_request.getMethod() == GET)
			_processGET(location);
		if (_request.getMethod() == POST)
			_processPOST(location);
		if (_request.getMethod() == DELETE)
			_processDELETE(location);
	}
}

ResponseBuilder::~ResponseBuilder() {}

std::string ResponseBuilder::str() const
{
	return (_response.toString());
}

void ResponseBuilder::_processGET(const ConfigFileParsing::location_t *location)
{
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
	(void) location;
}

void ResponseBuilder::_processDELETE(const ConfigFileParsing::location_t *location)
{
	(void) location;
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
	while ((dirIt = readdir(directory)) != 0)
	{
		if (std::strcmp(dirIt->d_name, ".") == 0 ||
			std::strcmp(dirIt->d_name, "..") == 0)
			continue;

		output << "<tr><td><a href=\"" << dirIt->d_name << "\">"
			   << dirIt->d_name << "</a></td>";

		filePath = fullPath + std::string(dirIt->d_name);
		if (stat(filePath.c_str(), &statBuff) == -1)
			continue;

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

const ConfigFileParsing::location_t *ResponseBuilder::_longestPrefixLocationMatch(
	const std::string &reqLocation) const
{

	const ConfigFileParsing::location_t *location = 0;
	std::size_t longestMatch = 0, slashLoc = 0, currMatch, i;
	ConfigFileParsing::location_iterator_t it = _server.locations.begin();
	std::string confLocation;

	for (; it != _server.locations.end(); it++)
	{
		currMatch = i = 0;
		confLocation = it->location[it->location.size() - 1] == '/'
						   ? it->location
						   : it->location + "/";

		while (i < reqLocation.size() && i < confLocation.size() && reqLocation[i] == confLocation[i])
		{
			if (reqLocation[i] == '/')
				slashLoc = i + 1;
			currMatch++;
			i++;
		}

		if ((currMatch > longestMatch && i == it->location.size() && i == reqLocation.size()) // Exact match
			|| slashLoc > longestMatch														  // Last slash location
		)
		{
			longestMatch = currMatch;
			location = &(*it);
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
