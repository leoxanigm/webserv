#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <string>
#include <vector>
#include <map>
#include <sys/types.h>

class CGIHandler
{
public:
	CGIHandler();
	~CGIHandler();

	bool init(const std::vector<std::string> &argv,
	          const std::map<std::string, std::string> &env,
	          const std::string &workingDir);

	int     getReadFd() const;
	int     getWriteFd() const;
	pid_t   getPid() const;

	ssize_t writeInput(const char *buf, size_t len);
	ssize_t readOutput(char *buf, size_t len);

	void    closeWriteFd();
	void    closeReadFd();

	bool    isWriteClosed() const;
	bool    isReadClosed() const;
	bool    isChildExited() const;

	int     pollChild();
	int     waitChild();
	int     getExitStatus() const;

	void    terminate();

private:
	CGIHandler(const CGIHandler &rhs);
	CGIHandler &operator=(const CGIHandler &rhs);

	bool setNonBlocking(int fd);
	bool setCloseOnExec(int fd);
	void closeFd(int &fd);
	void cleanupParentSide();

	char **buildArgv(const std::vector<std::string> &argv) const;
	char **buildEnvp(const std::map<std::string, std::string> &env) const;
	void   freeCStringArray(char **arr) const;

private:
	int   _readFd;
	int   _writeFd;
	pid_t _pid;

	bool _readClosed;
	bool _writeClosed;
	bool _childExited;
	int  _waitStatus;
};

#endif // CGIHANDLER_HPP