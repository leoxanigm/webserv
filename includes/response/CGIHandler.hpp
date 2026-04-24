#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <string>
#include <vector>
#include <map>
#include <sys/types.h>

#include "../parsing/HTTPRequestParser.hpp"
#include "./HTTPResponse.hpp"

class CGIHandler
{
public:
	CGIHandler();
	~CGIHandler();

	bool startCGI(const ConfigFileParsing::location_t *location, const HTTPRequest &req);

	int getReadFd() const;
	int getWriteFd() const;
	pid_t getPid() const;

	ssize_t writeInput(const char *buf, std::size_t len);
	ssize_t readOutput(char *buf, std::size_t len);

	void closeWriteFd();
	void closeReadFd();

	bool isWriteClosed() const;
	bool isReadClosed() const;
	bool isChildExited() const;

	int reapChildNonBlocking();
	int getExitStatus() const;

	void terminate();

private:
	CGIHandler(const CGIHandler &rhs);
	CGIHandler &operator=(const CGIHandler &rhs);

	int _readFd;
	int _writeFd;
	pid_t _pid;

	bool _readClosed;
	bool _writeClosed;
	bool _childExited;
	int _waitStatus;

	bool _init(const std::vector<std::string> &argv,
			   const std::map<std::string, std::string> &env,
			   const std::string &workingDir);

	static void _buildArgv(
		const std::vector<std::string> &argv, std::vector<char *> &execArgv);
	static void _buildEnvp(
		const std::map<std::string, std::string> &env,
		std::vector<std::string> &tempEnv,
		std::vector<char *> &execEnv);
	static std::string _methodToString(const HTTPRequest &req);
	static std::string _contentLenString(const HTTPRequest &req);
	static std::string _buildCookieString(const HTTPRequest &req);
	static std::string _resolveCGIRelativeScriptPath(
		const std::string &requestPath,
		const ConfigFileParsing::location_t *location);
};

#endif // CGIHANDLER_HPP