#include <string>
#include <ctime>

#include "../includes/HTTPResponse.hpp"

std::string HTTPResponse::directoryListing(const HTTPRequest &req)
{
	// location should have been properly constructed here.
	// For example, if url was /files and in config if /files was
	// routed to ./www/html/files,
	// req.getReqLocation() == /files/
	// req.getFullPath() == ./www/html/files/
	// This make sure we separate error checking concern from response
	// const std::string &reqLocation req.getReqLocation(); // TODO
	// const std::string &fullPath req.getFullPath(); // TODO

	(void) req; // remove after fixing the line above

	const std::string &reqLocation = "/files";
	const std::string &fullPath = "./www/html/files";

	std::string response;
	
	return (response);
}

const std::string &HTTPResponse::_dateGMT()
{
	std::time_t now = std::time(0);
	std::tm *gmt = std::gmtime(&now);
	char buff[30];

	std::strftime(buff, sizeof(buff), "%a, %d %b %Y %H:%M:%S GMT", gmt);
	return (std::string(buff));
}

const std::string &HTTPResponse::_successHeader()
{
	std::string header = "HTTP/1.1 200 OK\r\n";
	header += "Date: " + _dateGMT() + "\r\n";
	header += "Server: webserv\r\n";
	header += "Content-Type: text/html;charset=UTF-8\r\n";
}
