#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include "../includes/ResponseBuilder.hpp"
#include "../includes/HTTPError.hpp"
#include "../includes/parsing/HTTPRequestParser.hpp"
#include "../includes/parsing/ConfigFileParsing.hpp"

ResponseBuilder::ResponseBuilder(const HTTPRequest &req, const ConfigFileParsing &config)
	: _req(req), _config(config)
{
	_directoryListing();
}

ResponseBuilder::~ResponseBuilder()
{
}

void ResponseBuilder::_directoryListing()
{
	// location should have been properly constructed here.
	// For example, if url was /files and in config if /files was
	// routed to ./www/html/files,
	// _req.getReqLocation() == /files/
	// _req.getFullPath() == ./www/html/files/
	// This make sure we separate error checking concern from response
	// const std::string &reqLocation _req.getReqLocation(); // TODO
	// const std::string &fullPath _req.getFullPath(); // TODO

	// const std::string &reqLocation = "/files/";
	const std::string &fullPath = "./www/html/files/";

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
			_res.setStatusCode(HTTPError::FORBIDDEN);
		// _res.setBody(_readFile(_config.getDefaultPage(HTTPError::FORBIDDEN)));
		else
			_res.setStatusCode(HTTPError::INTERNAL_SERVER_ERROR);
		// _res.setBody(_readFile(_config.getDefaultPage(HTTPError::FORBIDDEN)));

		return;
	}

	struct dirent *dirIt = 0;
	while ((dirIt = readdir(directory)) != 0)
	{
		std::cout << dirIt->d_name << " " << dirIt->d_type << " " << dirIt->d_reclen << '\n';
	}

	// .  24
	// ..  24
	// text.txt 32
	// file.pdf 32
	// audio.mp3 32
	// video.mp4 32

	closedir(directory);
	// std::string response;
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