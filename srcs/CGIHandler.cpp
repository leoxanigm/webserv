

#include <unistd.h>  
#include <fcntl.h>   
#include <sys/wait.h>
#include <signal.h>  
#include <cerrno>
#include <cstring>   
#include <cstdlib>   

#include "CGIHandler.hpp"

CGIHandler::CGIHandler()
	: _readFd(-1),
	  _writeFd(-1),
	  _pid(-1),
	  _readClosed(true),
	  _writeClosed(true),
	  _childExited(false),
	  _waitStatus(0)
{
}

CGIHandler::~CGIHandler()
{
	closeWriteFd();
	closeReadFd();

	if (_pid > 0 && !_childExited)
	{
		terminate();
		waitChild();
	}
}

bool CGIHandler::setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return false;
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		return false;
	return true;
}

bool CGIHandler::setCloseOnExec(int fd)
{
	int flags = fcntl(fd, F_GETFD, 0);
	if (flags == -1)
		return false;
	if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
		return false;
	return true;
}

void CGIHandler::closeFd(int &fd)
{
	if (fd != -1)
	{
		close(fd);
		fd = -1;
	}
}

void CGIHandler::cleanupParentSide()
{
	closeWriteFd();
	closeReadFd();
}

char **CGIHandler::buildArgv(const std::vector<std::string> &argv) const
{
	char **out = new char*[argv.size() + 1];
	for (size_t i = 0; i < argv.size(); ++i)
	{
		out[i] = new char[argv[i].size() + 1];
		std::strcpy(out[i], argv[i].c_str());
	}
	out[argv.size()] = NULL;
	return out;
}

char **CGIHandler::buildEnvp(const std::map<std::string, std::string> &env) const
{
	char **out = new char*[env.size() + 1];
	size_t i = 0;
	for (std::map<std::string, std::string>::const_iterator it = env.begin(); it != env.end(); ++it, ++i)
	{
		std::string kv = it->first + "=" + it->second;
		out[i] = new char[kv.size() + 1];
		std::strcpy(out[i], kv.c_str());
	}
	out[env.size()] = NULL;
	return out;
}

void CGIHandler::freeCStringArray(char **arr) const
{
	if (!arr)
		return;
	for (size_t i = 0; arr[i] != NULL; ++i)
		delete[] arr[i];
	delete[] arr;
}

bool CGIHandler::init(const std::vector<std::string> &argv,
                      const std::map<std::string, std::string> &env,
                      const std::string &workingDir)
{
	if (argv.empty())
		return false;
	if (_pid > 0 && !_childExited)
		return false; // already running

	int stdinPipe[2] = {-1, -1};
	int stdoutPipe[2] = {-1, -1};

	if (pipe(stdinPipe) == -1)
		return false;
	if (pipe(stdoutPipe) == -1)
	{
		close(stdinPipe[0]);
		close(stdinPipe[1]);
		return false;
	}

	_pid = fork();
	if (_pid == -1)
	{
		close(stdinPipe[0]); close(stdinPipe[1]);
		close(stdoutPipe[0]); close(stdoutPipe[1]);
		_pid = -1;
		return false;
	}

	if (_pid == 0)
	{
		close(stdinPipe[1]);
		close(stdoutPipe[0]);

		if (dup2(stdinPipe[0], STDIN_FILENO) == -1)
			std::exit(127);
		if (dup2(stdoutPipe[1], STDOUT_FILENO) == -1)
			std::exit(127);

		close(stdinPipe[0]);
		close(stdoutPipe[1]);

		if (!workingDir.empty())
		{
			if (chdir(workingDir.c_str()) == -1)
				std::exit(127);
		}

		char **execArgv = buildArgv(argv);
		char **execEnvp = buildEnvp(env);

		execve(execArgv[0], execArgv, execEnvp);

		// execve failed
		freeCStringArray(execArgv);
		freeCStringArray(execEnvp);
		std::exit(127);
	}

	// Parent process
	close(stdinPipe[0]);
	close(stdoutPipe[1]);

	_writeFd = stdinPipe[1];
	_readFd  = stdoutPipe[0];

	_writeClosed = false;
	_readClosed  = false;
	_childExited = false;
	_waitStatus  = 0;

	// Poll-friendly
	if (!setNonBlocking(_writeFd) || !setNonBlocking(_readFd))
	{
		cleanupParentSide();
		terminate();
		waitChild();
		return false;
	}
	setCloseOnExec(_writeFd);
	setCloseOnExec(_readFd);

	return true;
}

int CGIHandler::getReadFd() const  { return _readFd; }
int CGIHandler::getWriteFd() const { return _writeFd; }
pid_t CGIHandler::getPid() const   { return _pid; }

ssize_t CGIHandler::writeInput(const char *buf, size_t len)
{
	if (_writeClosed || _writeFd == -1)
		return -1;
	return write(_writeFd, buf, len);
}

ssize_t CGIHandler::readOutput(char *buf, size_t len)
{
	if (_readClosed || _readFd == -1)
		return -1;
	return read(_readFd, buf, len);
}

void CGIHandler::closeWriteFd()
{
	if (!_writeClosed)
	{
		closeFd(_writeFd);
		_writeClosed = true;
	}
}

void CGIHandler::closeReadFd()
{
	if (!_readClosed)
	{
		closeFd(_readFd);
		_readClosed = true;
	}
}

bool CGIHandler::isWriteClosed() const { return _writeClosed; }
bool CGIHandler::isReadClosed() const  { return _readClosed; }
bool CGIHandler::isChildExited() const { return _childExited; }

int CGIHandler::pollChild()
{
	if (_pid <= 0 || _childExited)
		return _childExited ? 1 : -1;

	pid_t ret = waitpid(_pid, &_waitStatus, WNOHANG);
	if (ret == 0)
		return 0;
	if (ret == -1)
		return -1;

	_childExited = true;
	return 1;
}

int CGIHandler::waitChild()
{
	if (_pid <= 0 || _childExited)
		return _childExited ? 0 : -1;

	if (waitpid(_pid, &_waitStatus, 0) == -1)
		return -1;

	_childExited = true;
	return 0;
}

int CGIHandler::getExitStatus() const
{
	if (!_childExited)
		return -1;
	if (WIFEXITED(_waitStatus))
		return WEXITSTATUS(_waitStatus);
	if (WIFSIGNALED(_waitStatus))
		return 128 + WTERMSIG(_waitStatus);
	return -1;
}

void CGIHandler::terminate()
{
	if (_pid <= 0 || _childExited)
		return;

	kill(_pid, SIGTERM);
	// TODO
    // handle timeout
}