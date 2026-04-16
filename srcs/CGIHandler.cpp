#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstring>
#include <cstdlib>

#include "../includes/CGIHandler.hpp"
#include "../includes/parsing/ConfigFileParsing.hpp"
#include "../includes/parsing/HTTPRequestParser.hpp"
#include "../includes/ResponseBuilder.hpp"
#include "../includes/HTTPError.hpp"
#include "../includes/utils.hpp"

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

/**
 * Initializes and starts CGI child process, pipe, etc.
 * Throws HTTPError on error.
 */
bool CGIHandler::startCGI(const ConfigFileParsing::location_t *location, const HTTPRequest &req)
{
	std::string requestPath = ResponseBuilder::_stripQueryString(req.getRequestTarget());
	std::string scriptPath = ResponseBuilder::_resolvePath(requestPath, location);
	std::string scriptBasename = scriptPath;

	// TODO
	// http://localhost:8080/python?test=true doesn't work
	// but http://localhost:8080/python/?test=true does
	// also query strings on normal GET not working

	// plus, add default error page html files

	if ((location->cgi_type == PHP && (scriptBasename.find(".php") == std::string::npos)) ||
		(location->cgi_type == PYTHON && (scriptBasename.find(".py") == std::string::npos)))
	{
		if (scriptBasename[scriptBasename.size() - 1] == '/')
			scriptBasename += location->index;
		else
			scriptBasename += "/" + location->index;
	}

	scriptBasename = scriptBasename.substr(scriptBasename.find_last_of('/') + 1);

	std::vector<std::string> argv;
	argv.push_back(location->cgi_location);
	argv.push_back(scriptBasename);

	std::map<std::string, std::string> env;
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["REQUEST_METHOD"] = _methodToString(req);
	env["SCRIPT_NAME"] = requestPath.substr(location->location.size());
	env["QUERY_STRING"] = _extractQueryString(req.getRequestTarget());
	env["SERVER_PROTOCOL"] = req.getHTTPVersion();
	env["CONTENT_LENGTH"] = _contentLenString(req);
	env["CONTENT_TYPE"] = ResponseBuilder::_getRequestMediaType(req.getContentType());
	env["REQUEST_URI"] = req.getRequestTarget();
	env["SCRIPT_FILENAME"] = scriptBasename;
	env["REDIRECT_STATUS"] = "200";
	env["HTTP_COOKIE"] = _buildCookieString(req);

	if (!_init(argv, env, location->root))
		return (false);

	return (true);
}

void CGIHandler::_buildArgv(
	const std::vector<std::string> &argv, std::vector<char *> &execArgv)
{
	execArgv.clear();
	for (std::size_t i = 0; i < argv.size(); i++)
		execArgv.push_back(const_cast<char *>(argv[i].c_str()));
	execArgv.push_back(NULL);
}

void CGIHandler::_buildEnvp(
	const std::map<std::string, std::string> &env,
	std::vector<std::string> &tempEnv,
	std::vector<char *> &execEnv)
{
	std::map<std::string, std::string>::const_iterator it = env.begin();
	for (; it != env.end(); it++)
		tempEnv.push_back(it->first + "=" + it->second);

	for (std::size_t i = 0; i < tempEnv.size(); i++)
		execEnv.push_back(const_cast<char *>(tempEnv[i].c_str()));
	execEnv.push_back(NULL);
}

std::string CGIHandler::_methodToString(const HTTPRequest &req)
{
	switch (req.getMethod())
	{
	case GET:
		return "GET";
	case POST:
		return "POST";
	case DELETE:
		return "DELETE";
	}
	return ("GET");
}

std::string CGIHandler::_extractQueryString(const std::string &uri)
{
	std::string::size_type q = uri.find('?');
	if (q == std::string::npos)
		return "";
	return (uri.substr(q + 1));
}

std::string CGIHandler::_contentLenString(const HTTPRequest &req)
{
	if (req.getContentLength() == 0)
		return ("");

	std::ostringstream contentLen;
	contentLen << req.getContentLength();
	return (contentLen.str());
}

std::string CGIHandler::_buildCookieString(const HTTPRequest &req)
{
	const std::map<std::string, std::string>			&cookies = req.getCookie();
	std::string											result;
	std::map<std::string, std::string>::const_iterator	it = cookies.begin();

	for (; it != cookies.end(); ++it)
	{
		if (!result.empty())
			result += "; ";
		result += it->first + "=" + it->second;
	}
	return (result);
}

bool CGIHandler::_init(const std::vector<std::string> &argv,
					   const std::map<std::string, std::string> &env,
					   const std::string &workingDir)
{
	if (argv.empty())
		return (false);
	if (_pid > 0 && !_childExited)
		return (false); // already running

	int stdinPipe[2] = {-1, -1};
	int stdoutPipe[2] = {-1, -1};

	if (pipe(stdinPipe) == -1)
		return (false);
	if (pipe(stdoutPipe) == -1)
	{
		close(stdinPipe[0]);
		close(stdinPipe[1]);
		return (false);
	}

	std::vector<char *> execArgv;
	// We use this so that char pointer stays alive since execEnv points to its strings
	std::vector<std::string> tempEnv;
	std::vector<char *> execEnv;

	_buildArgv(argv, execArgv);
	_buildEnvp(env, tempEnv, execEnv);

	_pid = fork();

	if (_pid == -1)
	{
		close(stdinPipe[0]);
		close(stdinPipe[1]);
		close(stdoutPipe[0]);
		close(stdoutPipe[1]);

		return (false);
	}

	if (_pid == 0)
	{
		if (!workingDir.empty())
		{
			if (chdir(workingDir.c_str()) == -1)
				std::exit(127);
		}

		close(stdinPipe[1]);
		close(stdoutPipe[0]);

		if (dup2(stdinPipe[0], STDIN_FILENO) == -1)
			std::exit(127);
		if (dup2(stdoutPipe[1], STDOUT_FILENO) == -1)
			std::exit(127);

		close(stdinPipe[0]);
		close(stdoutPipe[1]);

		execve(execArgv[0], &execArgv[0], &execEnv[0]);

		std::exit(127);
	}

	close(stdinPipe[0]);
	close(stdoutPipe[1]);

	_writeFd = stdinPipe[1];
	_readFd = stdoutPipe[0];

	_writeClosed = false;
	_readClosed = false;
	_childExited = false;
	_waitStatus = 0;

	if (utils::setNonBlocking(_writeFd) == -1 || utils::setNonBlocking(_readFd) == -1)
	{
		closeWriteFd();
		closeReadFd();
		terminate();
		waitChild();
		return (false);
	}

	return (true);
}

int CGIHandler::getReadFd() const { return (_readFd); }
int CGIHandler::getWriteFd() const { return (_writeFd); }
pid_t CGIHandler::getPid() const { return (_pid); }

ssize_t CGIHandler::writeInput(const char *buf, size_t len)
{
	if (_writeClosed || _writeFd == -1)
		return (-1);
	return (write(_writeFd, buf, len));
}

ssize_t CGIHandler::readOutput(char *buf, size_t len)
{
	if (_readClosed || _readFd == -1)
		return (-1);
	return (read(_readFd, buf, len));
}

void CGIHandler::closeWriteFd()
{
	if (!_writeClosed)
	{
		if (_writeFd != -1)
			close(_writeFd);
		_writeFd = -1;
		_writeClosed = true;
	}
}

void CGIHandler::closeReadFd()
{
	if (!_readClosed)
	{
		if (_readFd != -1)
			close(_readFd);
		_readFd = -1;
		_readClosed = true;
	}
}

bool CGIHandler::isWriteClosed() const { return (_writeClosed); }
bool CGIHandler::isReadClosed() const { return (_readClosed); }
bool CGIHandler::isChildExited() const { return (_childExited); }

int CGIHandler::reapChildNonBlocking()
{
	if (_pid <= 0 || _childExited)
		return (_childExited ? 1 : -1);

	pid_t ret = waitpid(_pid, &_waitStatus, WNOHANG);
	if (ret == 0)
		return (0);
	if (ret == -1)
		return (-1);

	_childExited = true;
	return (1);
}

int CGIHandler::waitChild()
{
	if (_pid <= 0 || _childExited)
		return (_childExited ? 0 : -1);

	if (waitpid(_pid, &_waitStatus, 0) == -1)
		return (-1);

	_childExited = true;
	return (0);
}

int CGIHandler::getExitStatus() const
{
	if (!_childExited)
		return (-1);
	if (WIFEXITED(_waitStatus))
		return (WEXITSTATUS(_waitStatus));
	if (WIFSIGNALED(_waitStatus))
		return (128 + WTERMSIG(_waitStatus));
	return (-1);
}

void CGIHandler::terminate()
{
	if (_pid <= 0 || _childExited)
		return;

	kill(_pid, SIGTERM);
	// TODO
	// handle timeout
}