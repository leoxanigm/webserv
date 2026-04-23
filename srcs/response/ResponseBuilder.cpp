#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <ctime>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#include "../../includes/response/ResponseBuilder.hpp"
#include "../../includes/response/HTTPError.hpp"
#include "../../includes/parsing/HTTPRequestParser.hpp"
#include "../../includes/parsing/ConfigFileParsing.hpp"
#include "../../includes/utils.hpp"

ResponseBuilder::ResponseBuilder(
	const HTTPRequest &req, const ConfigFileParsing::server_t &server)
	: _request(req), _server(server)
{
	_isCGI = false;
	_location = 0;

	if (!_request.isValid())
	{
		_response.setStatusCode(_request.getError());
		_response.setBody(utils::readFile(_getErrorPage(_request.getError())));
	}
	else
	{
		const ConfigFileParsing::location_t *location = 0;
		location = _longestPrefixLocationMatch(_request.getPath());
		if (location == 0)
		{
			_response.setStatusCode(HTTPError::NOT_FOUND);
			_response.setBody(utils::readFile(_getErrorPage(HTTPError::NOT_FOUND)));
		}
		else
		{
			_location = location;

			if (location->is_cgi)
			{
				if (_isValidCGIScript())
					_isCGI = true;
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

					// _response.setStatusCode(HTTPError::NOT_FOUND);
					// _response.setBody(utils::readFile(_getErrorPage(HTTPError::NOT_FOUND)));
				}
			}
			else if (location->return_code == 308) // handle redirect first
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

void ResponseBuilder::buildErrorRes(int code)
{
	_response.setStatusCode(code);
	_response.setBody(utils::readFile(_getErrorPage(code)));
}

std::string ResponseBuilder::str() const
{
	return (_response.toString());
}

bool ResponseBuilder::isCGI() const
{
	return (_isCGI);
}

const ConfigFileParsing::location_t *ResponseBuilder::getLocation() const
{
	return (_location);
}

void ResponseBuilder::_processGET(const ConfigFileParsing::location_t *location)
{
	if (location->allowed_methods.count("GET") == 0)
	{
		_response.setStatusCode(HTTPError::METHOD_NOT_ALLOWED);
		_response.setBody(utils::readFile(_getErrorPage(HTTPError::METHOD_NOT_ALLOWED)));
		return;
	}

	std::string requestTarget = _stripQueryString(_request.getPath());
	std::string path = _resolvePath(requestTarget, location);

	struct stat st;

	// Can't find file or directory
	if (stat(path.c_str(), &st) != 0)
	{
		_response.setStatusCode(HTTPError::NOT_FOUND);
		_response.setBody(utils::readFile(_getErrorPage(HTTPError::NOT_FOUND)));
		return;
	}

	// Directory
	if (S_ISDIR(st.st_mode))
	{
		if (!requestTarget.empty() && requestTarget[requestTarget.size() - 1] != '/')
		{
			_response.setStatusCode(HTTPResponse::PERMANENT_REDIRECT);
			_response.setHeader("Location", requestTarget + "/");
			return;
		}

		// Try to serve index file if configured
		if (!location->index.empty())
		{
			std::string indexPath = path;
			if (!indexPath.empty() && indexPath[indexPath.size() - 1] != '/')
				indexPath += "/";
			indexPath += location->index;

			struct stat indexSt;
			if (stat(indexPath.c_str(), &indexSt) == 0 && S_ISREG(indexSt.st_mode))
			{
				std::vector<char> fileContent = utils::readFile(indexPath);
				if (!fileContent.empty() || indexSt.st_size == 0)
				{
					_response.setStatusCode(HTTPResponse::OK);
					_response.setHeader("Content-Type", _getMimeType(indexPath));
					_response.setBody(fileContent);
					return;
				}
			}
		}

		// No index file or can't read it, check autoindex
		if (location->autoindex)
			_directoryListing(requestTarget, path);
		else
		{
			_response.setStatusCode(HTTPError::FORBIDDEN);
			try
			{
				_response.setBody(utils::readFile(_getErrorPage(HTTPError::FORBIDDEN)));
			}
			catch (const std::exception &e)
			{
				std::cerr << "Error setting body in ResponseBuilder::_processGET: "
						  << e.what() << '\n';
			}
		}
		return;
	}

	// File
	if (S_ISREG(st.st_mode))
	{
		std::vector<char> fileContent = utils::readFile(path);
		if (fileContent.empty() && st.st_size > 0)
		{
			// File could not be read (permissions or other error)
			_response.setStatusCode(HTTPError::FORBIDDEN);
			_response.setBody(utils::readFile(_getErrorPage(HTTPError::FORBIDDEN)));
			return;
		}

		_response.setStatusCode(HTTPResponse::OK);
		_response.setHeader("Content-Type", _getMimeType(path));
		_response.setBody(fileContent);
		return;
	}

	// Not a regular file or directory (socket, device, etc.)
	_response.setStatusCode(HTTPError::FORBIDDEN);
	_response.setBody(utils::readFile(_getErrorPage(HTTPError::FORBIDDEN)));
}

void ResponseBuilder::_processPOST(const ConfigFileParsing::location_t *location)
{
	if (location->allowed_methods.count("POST") == 0)
	{
		_response.setStatusCode(HTTPError::METHOD_NOT_ALLOWED);
		_response.setBody(utils::readFile(_getErrorPage(HTTPError::METHOD_NOT_ALLOWED)));
		return;
	}

	std::string requestTarget = _stripQueryString(_request.getPath());
	std::string path = _resolvePath(requestTarget, location);

	std::string mediaType = _getRequestMediaType(_request.getContentType());
	bool created = false;
	std::string responseBody;

	if (mediaType == "multipart/form-data")
	{
		std::map<std::string, std::string> contentType = _request.getContentType();
		std::string boundary = contentType["boundary"];
		if (boundary.empty())
		{
			_response.setStatusCode(HTTPError::BAD_REQUEST);
			_response.setBody(utils::readFile(_getErrorPage(HTTPError::BAD_REQUEST)));
			return;
		}

		std::string filename;
		std::string fileContent;
		if (!_parseMultipartFile(_request.getBody(), boundary, filename, fileContent))
		{
			_response.setStatusCode(HTTPError::BAD_REQUEST);
			_response.setBody(utils::readFile(_getErrorPage(HTTPError::BAD_REQUEST)));
			return;
		}

		std::string uploadPath = path;
		if (!uploadPath.empty() && uploadPath[uploadPath.size() - 1] != '/')
		{
			// if target file name is not explicit, treat path as directory
			struct stat dirSt;
			if (stat(uploadPath.c_str(), &dirSt) == 0 && S_ISDIR(dirSt.st_mode))
				uploadPath += "/";
		}

		if (!uploadPath.empty() && uploadPath[uploadPath.size() - 1] != '/')
			uploadPath += "/";

		uploadPath += filename;

		created = utils::_writeFile(uploadPath, fileContent);
		responseBody = "File uploaded";
	}
	else
	{
		// application/x-www-form-urlencoded or plain body
		created = utils::_writeFile(path, _request.getBody());
		responseBody = "Data stored";
	}

	if (!created)
	{
		_response.setStatusCode(HTTPError::FORBIDDEN);
		_response.setBody(utils::readFile(_getErrorPage(HTTPError::FORBIDDEN)));
		return;
	}

	_response.setStatusCode(HTTPResponse::CREATED);
	_response.setHeader("Content-Type", "text/plain");
	_response.setBody(responseBody);
}

void ResponseBuilder::_processDELETE(const ConfigFileParsing::location_t *location)
{
	if (location->allowed_methods.count("DELETE") == 0)
	{
		_response.setStatusCode(HTTPError::METHOD_NOT_ALLOWED);
		_response.setBody(utils::readFile(_getErrorPage(HTTPError::METHOD_NOT_ALLOWED)));
		return;
	}

	std::string requestTarget = _stripQueryString(_request.getPath());
	std::string path = _resolvePath(requestTarget, location);

	struct stat st;
	if (stat(path.c_str(), &st) != 0)
	{
		_response.setStatusCode(HTTPError::NOT_FOUND);
		_response.setBody(utils::readFile(_getErrorPage(HTTPError::NOT_FOUND)));
		return;
	}

	// Don't allow deleting directories
	if (S_ISDIR(st.st_mode))
	{
		_response.setStatusCode(HTTPError::FORBIDDEN);
		_response.setBody(utils::readFile(_getErrorPage(HTTPError::FORBIDDEN)));
		return;
	}

	// Delete the file
	if (std::remove(path.c_str()) != 0)
	{
		_response.setStatusCode(HTTPError::FORBIDDEN);
		_response.setBody(utils::readFile(_getErrorPage(HTTPError::FORBIDDEN)));
		return;
	}

	_response.setStatusCode(HTTPResponse::OK);
}

void ResponseBuilder::_directoryListing(const std::string &reqLocation, const std::string &fullPath)
{
	errno = 0;
	DIR *directory = opendir(fullPath.c_str());
	if (!directory)
	{
		if (errno == EACCES || errno == ENOTDIR)
		{
			_response.setStatusCode(HTTPError::FORBIDDEN);
			_response.setBody(utils::readFile(_getErrorPage(HTTPError::FORBIDDEN)));
		}
		else
		{
			_response.setStatusCode(HTTPError::INTERNAL_SERVER_ERROR);
			_response.setBody(utils::readFile(_getErrorPage(HTTPError::INTERNAL_SERVER_ERROR)));
		}

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

		filePath = fullPath;
		if (!filePath.empty() && filePath[filePath.size() - 1] != '/')
			filePath += "/";
		filePath += dirIt->d_name;

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

	_response.setStatusCode(HTTPResponse::OK);
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
	case 406:
		return (_server.error_pages.NotAcceptable);
	case 408:
		return (_server.error_pages.RequestTimeout);
	case 411:
		return (_server.error_pages.LengthRequired);
	case 413:
		return (_server.error_pages.PayloadTooLarge);
	case 415:
		return (_server.error_pages.UnsupportedMediaType);
	case 416:
		return (_server.error_pages.RangeNotSatisfiable);
	case 417:
		return (_server.error_pages.ExpectationFailed);
	case 500:
		return (_server.error_pages.InternalServerError);
	case 501:
		return (_server.error_pages.NotImplemented);
	case 502:
		return (_server.error_pages.BadGateway);
	case 503:
		return (_server.error_pages.ServiceUnavailable);
	case 504:
		return (_server.error_pages.GatewayTimeout);
	case 505:
		return (_server.error_pages.HttpVersionNotSupported);
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

		if (len > reqURI.size())
			continue;

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
		bool boundaryMatch = (reqURI.size() > len && reqURI[len] == '/');

		if ((exactMatch || boundaryMatch) && len > locationLen)
		{
			location = &(*it);
			locationLen = len;
		}
	}

	return (location);
}

bool ResponseBuilder::_isValidCGIScript() const
{
	std::string requestPath = ResponseBuilder::_stripQueryString(_request.getPath());
	std::string scriptPath = ResponseBuilder::_resolvePath(requestPath, _location);
	std::string path = scriptPath;

	if ((_location->cgi_type == PHP && (path.find(".php") == std::string::npos)) ||
		(_location->cgi_type == PYTHON && (path.find(".py") == std::string::npos)))
	{
		if (path[path.size() - 1] == '/')
			path += _location->index;
		else
			path += "/" + _location->index;
	}

	if (access(path.c_str(), F_OK) == -1)
		return (false);

	if (access(path.c_str(), R_OK) == -1)
		return (false);

	struct stat st;
	if (stat(path.c_str(), &st) == -1)
		return (false);

	if (!S_ISREG(st.st_mode))
		return (false);

	return (true);
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

std::string ResponseBuilder::_stripQueryString(const std::string &uri)
{
	std::string::size_type questionPos = uri.find('?');
	if (questionPos == std::string::npos)
		return (uri);
	return (uri.substr(0, questionPos));
}

std::string ResponseBuilder::_getRequestMediaType(
	const std::map<std::string, std::string> &contentType)
{
	std::string mediaType;
	std::vector<std::pair<std::string, std::string> > params;

	for (std::map<std::string, std::string>::const_iterator it = contentType.begin();
		 it != contentType.end(); ++it)
	{
		if (it->first.find('/') != std::string::npos)
			mediaType = it->first;
		else
			params.push_back(*it);
	}

	if (mediaType.empty())
		return "";

	std::string result = mediaType;
	for (std::vector<std::pair<std::string, std::string> >::const_iterator it = params.begin();
		 it != params.end(); ++it)
	{
		result += "; ";
		result += it->first;
		result += "=";
		result += it->second;
	}
	return result;
}

bool ResponseBuilder::_parseMultipartFile(
	const std::string &body,
	const std::string &boundary,
	std::string &outFilename,
	std::string &outContent)
{
	std::string delim = "--" + boundary;
	std::string endDelim = delim + "--";

	std::string::size_type partStart = body.find(delim);
	if (partStart == std::string::npos)
		return (false);

	partStart += delim.size();
	if (body.compare(partStart, 2, "\r\n") == 0)
		partStart += 2;

	std::string::size_type headersEnd = body.find("\r\n\r\n", partStart);
	if (headersEnd == std::string::npos)
		return (false);

	std::string headers = body.substr(partStart, headersEnd - partStart);
	std::string::size_type filenamePos = headers.find("filename=\"");
	if (filenamePos == std::string::npos)
		return (false);

	filenamePos += 10;
	std::string::size_type filenameEnd = headers.find('"', filenamePos);
	if (filenameEnd == std::string::npos)
		return (false);

	outFilename = headers.substr(filenamePos, filenameEnd - filenamePos);

	std::string::size_type contentStart = headersEnd + 4;
	std::string::size_type partEnd = body.find(delim, contentStart);
	if (partEnd == std::string::npos)
		return (false);

	if (partEnd >= 2 && body[partEnd - 2] == '\r' && body[partEnd - 1] == '\n')
		partEnd -= 2;

	outContent = body.substr(contentStart, partEnd - contentStart);
	return (true);
}
