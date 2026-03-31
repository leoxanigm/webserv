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
	struct stat statBuff;
	struct std::tm *tm;
	char dateStr[256];
	std::string filePath;
	while ((dirIt = readdir(directory)) != 0)
	{
		if (std::strcmp(dirIt->d_name, ".") == 0 ||
			std::strcmp(dirIt->d_name, "..") == 0)
			continue;

		filePath = fullPath + std::string(dirIt->d_name);
		if (stat(filePath.c_str(), &statBuff) == -1)
			continue;

		tm = std::localtime(&statBuff.st_mtime);
		std::strftime(dateStr, sizeof(dateStr), "%Y-%m-%d %H:%M", tm);

		std::cout << dirIt->d_name << " "
				  << dateStr << " ";

		if (S_ISREG(statBuff.st_mode)) // If regular file, show size
			std::cout << static_cast<float>(statBuff.st_size) / 1000 << "K\n";
		else
			std::cout << "-\n";
	}

	// text.txt 2026-03-27 12:25 0K
	// file.pdf 2026-03-27 12:25 0K
	// audio.mp3 2026-03-27 12:26 0K
	// video.mp4 2026-03-27 13:22 25.814K
	// folder 2026-03-27 13:52 -

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