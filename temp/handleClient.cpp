int Connection::_handleClient(int fd, short event)
{
	Client *client = _findClient(fd);
	if (!client)
		return (-1);

	int status = 0;

	if (event & POLLIN)
		status = client->handleRead(fd);

	if (event & POLLOUT)
		status = client->handleWrite(fd);

	// Register CGI read fd if needed
	int pr = client->getPipeReadFd();
	if (pr > -1 && _clientFds.find(pr) == _clientFds.end())
	{
		_addPollFd(pr, POLLIN);
		FdEntry e;
		e.type = FD_PIPE_READ;
		e.client = client;
		_clientFds[pr] = e;
	}

	// Register CGI write fd if needed
	int pw = client->getPipeWriteFd();
	if (pw > -1 && _clientFds.find(pw) == _clientFds.end())
	{
		_addPollFd(pw, POLLOUT);
		FdEntry e;
		e.type = FD_PIPE_WRITE;
		e.client = client;
		_clientFds[pw] = e;
	}

	// Keep poll events coherent with state
	if (client->hasActivePipeRead())
		_setFdPollEvent(client->getPipeReadFd(), POLLIN);

	if (client->hasActivePipeWrite())
		_setFdPollEvent(client->getPipeWriteFd(), POLLOUT);

	// socket should usually be write-ready only when sending response
	if (client->getState() == Client::SEND_RESPONSE)
		_setFdPollEvent(client->getConnFd(), POLLOUT);
	else
		_setFdPollEvent(client->getConnFd(), POLLIN);

	if (client->getState() == Client::CLOSE_CONNECTION)
		status = -1;

	return status;
}