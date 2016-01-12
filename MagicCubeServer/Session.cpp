#include "stdafx.h"
#include "Session.h"

map<ReadErrorType, string> ReadErrorMessage = 
{
	{ READERROR_PROTOCOL_MISMATCH,  "protocol mismatch"   },
	{ READERROR_PACKAGE_TOO_LONG,   "package is too long" },
	{ READERROR_PACKAGE_EMPTY,      "package is empty"    },
	{ READERROR_UNKNOWN,            "unknown"             }
};

#ifdef ENABLE_IPV4
Session::Session(TcpServer &server, sockaddr_in addr, int fd)
	: server(server), fd(fd), sAddr(addr)//, writtenEvent(false)
{
	char buffer[INET6_ADDRSTRLEN + 1] = { 0 };
	evutil_inet_ntop(addr.sin_family, &(addr.sin_addr), buffer, sizeof(buffer));
	RemoteAddress = buffer;
	RemotePort = ntohs(addr.sin_port);

	evutil_make_socket_nonblocking(fd);
	buffev = bufferevent_socket_new(server.Base, fd, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE | BEV_OPT_DEFER_CALLBACKS);
}
#endif

#ifdef ENABLE_IPV6
Session::Session(TcpServer &server, sockaddr_in6 addr, int fd)
	: server(server), IsIPv6(true), fd(fd), sAddr6(addr)//, writtenEvent(false)
{
	char buffer[INET6_ADDRSTRLEN + 1] = { 0 };
	evutil_inet_ntop(addr.sin6_family, &(addr.sin6_addr), buffer, sizeof(buffer));
	RemoteAddress = buffer;
	RemotePort = ntohs(addr.sin6_port);

	evutil_make_socket_nonblocking(fd);
	buffev = bufferevent_socket_new(server.Base, fd, BEV_OPT_CLOSE_ON_FREE);
}
#endif

Session::~Session()
{
	Close();
	readLock.lock();
	readLock.unlock();
	writeLock.lock();
	writeLock.unlock();
}

void Session::KeepAlive()
{
	LastAlive = time(NULL);
}

void Session::SetCallbacks()
{
	SetCallbacks(true, true, true);
}

void Session::SetCallbacks(bool read, bool write, bool event)
{
	bufferevent_data_cb readCB = NULL, writeCB = NULL;
	bufferevent_event_cb eventCB = NULL;
	short eventsEN = EV_PERSIST;
	if (read)
	{
		readCB = readCallbackDispatcher;
		eventsEN |= EV_READ;
	}
	if (write)
	{
		writeCB = writeCallbackDispatcher;
		eventsEN |= EV_WRITE;
	}
	if (event)
	{
		eventCB = errorCallbackDispatcher;
	}

	bufferevent_setcb(buffev, 
		readCB,
		writeCB,
		eventCB, (void*)this);

	if (read || write || event)
	{
		bufferevent_enable(buffev, eventsEN);
	}
	else
	{
		bufferevent_disable(buffev, EV_READ | EV_WRITE);
	}
}

void Session::ClearCallbacks()
{
	SetCallbacks(false, false, false);
}

void Session::ReadCallback()
{
	if (!IsAlive) return;

	unique_lock<mutex> lck(readLock);
	debug("fd = %u, entering read", (unsigned int)fd);
	while (IsAlive)
	{
		KeepAlive();
		switch (readState)
		{
		case READSTATE_NONE:
			readLength = 0;
			readState = READSTATE_READING_HEADER;
			continue;
			break;
		case READSTATE_READING_HEADER:
		{
			const size_t headerLength = sizeof(PackageHeader);

			while (readLength < headerLength)
			{
				size_t currentLength = bufferevent_read(buffev, headerBuffer + readLength, headerLength - readLength);
				if (currentLength <= 0) break;

				readLength += currentLength;
			}

			if (readLength == headerLength)
			{
				PackageHeader header = *(PackageHeader*)headerBuffer;
				header.length = ntohpacklen(header.length);

				if (memcmp(headerBuffer, "GET ", min(headerLength, (size_t)4)) == 0)
				{
					// http request!
					char buff[sizeof(PackageHeader) + 1];
					memcpy(buff, headerBuffer, sizeof(PackageHeader));
					buff[sizeof(PackageHeader)] = '\0';

					lineBuffer = buff;

					readState = READSTATE_READING_LINE;
					continue;
				}
				else if (memcmp(headerBuffer, MAGIC_MARK, min(headerLength, sizeof(MAGIC_MARK) - 1)) != 0)
				{
					readErrorCode = READERROR_PROTOCOL_MISMATCH;
					readState = READSTATE_ERROR;
					continue;
				}
				
				if (header.length == 0 || header.length > PACKAGE_MAXLENGTH)
				{
					// package is zero or too long
					debug("fd = %u, data length: %d == 0 || too long", (unsigned int)fd, header.length);
					
					if (header.length == 0)
						readErrorCode = READERROR_PACKAGE_EMPTY;
					else
						readErrorCode = READERROR_PACKAGE_TOO_LONG;
					
					readState = READSTATE_ERROR;
					continue;
				}

				currentPackage = (Package*)malloc(sizeof(PackageHeader) + header.length);
				if (!currentPackage)
				{
					throw bad_alloc();
				}
				currentPackage->header = header; // copy

				memset(currentPackage->data, 0x00, header.length); // ensure

				readLength = 0;
				readState = READSTATE_READING_DATA;
				continue;
			}
			break;
		}
		case READSTATE_READING_DATA:
		{
			debug("fd = %u, recv %d", (unsigned int)fd, currentPackage->header.length);
			while (readLength < currentPackage->header.length)
			{
				size_t currentLength = bufferevent_read(buffev, currentPackage->data + readLength, currentPackage->header.length - readLength);
				if (currentLength <= 0) break;

				readLength += currentLength;
			}

			if (readLength == currentPackage->header.length)
			{
				OnPackage(currentPackage); // TODO: sync or async?
				if (currentPackage)
				{
					delete currentPackage;
					currentPackage = NULL;
				}
				readLength = 0;
				readState = READSTATE_NONE;
				continue;
			}
			break;
		}
		case READSTATE_READING_LINE:
		{
			char ch;
			if (bufferevent_read(buffev, &ch, 1) > 0)
			{
				lineBuffer += ch;
				if (lineBuffer.length() > HTTP_HEADER_MAXLENGTH)
				{
					// http header is too long
					readErrorCode = READERROR_PACKAGE_TOO_LONG;
					readState = READSTATE_ERROR;
					continue;
				}
				if (endsWith(lineBuffer, "\r\n\r\n"))
				{
					Package *pack = MakePackage(lineBuffer);
					OnPackage(pack);
					if (pack)
					{
						delete pack;
						pack = NULL;
					}
					readState = READSTATE_NONE;
					continue;
				}
				continue;
			}
			break;
		}
		case READSTATE_ERROR:
		{
			SendPackage("{\"error\": \"" + ReadErrorMessage[readErrorCode] + "\"}");
			FlushAndClose();
			break;
		}
		}
		break;
	}
	debug("fd = %u, exitting read", (unsigned int)fd);
}

void Session::WriteCallback()
{
	if (!IsAlive) return;

	{
		unique_lock<mutex> lck(writeLock);
		if (isFirstCall)
		{
			debug("fd = %u, dropping first write callback calling", (unsigned int)fd);
			// drop first write callback calling
			isFirstCall = false;
#ifndef __linux
			return;
#endif
		}
	}

	debug("fd = %u, write callback called", (unsigned int)fd);

	if (closeAfterWritten)
	{
		debug("fd = %u, closeAfterWritten is set, closing", (unsigned int)fd);
		Close();
	}
}

void Session::ErrorCallback(short event)
{
	if (!IsAlive) return;

	const char *msg = "";
	if (event & BEV_EVENT_TIMEOUT)
	{
		msg = "timed out"; // if bufferevent_set_timeouts() called
	}
	else if (event & BEV_EVENT_EOF)
	{
		msg = "connection closed";
	}
	else if (event & BEV_EVENT_ERROR)
	{
		msg = "some other error";
		__perror("error");
	}

	debug("fd = %u, %s", (unsigned int)fd, msg);
	
	Close();
}

void Session::DoQueue()
{
	if (!IsAlive) return;

	unique_lock<mutex> lck(writeLock);
	debug("fd = %u, dequeuing", (unsigned int)fd);

	while (!pendingPackages.empty())
	{
		Package *&pack = pendingPackages.front();
		if (pack->header.length > 0)
		{
			size_t toSendLength = sizeof(PackageHeader) + ntohpacklen(pack->header.length);
			char *packdata = (char*)pack;

			bufferevent_write(buffev, packdata, toSendLength);
		}
		delete pack;
		pack = NULL;

		pendingPackages.pop();
	}

	debug("fd = %u, dequeued", (unsigned int)fd);
}

void Session::OnPackage(Package *&pack)
{
	if (!pack || pack->header.length == 0) return;
	pack->data[pack->header.length - 1] = '\0';
	debug("on package (fd = %u): %c%c, %s", (unsigned int)fd, pack->header.magic[0], pack->header.magic[1], pack->data);

	// TODO: process received package

	if (memcmp(pack->data, "GET ", min(pack->header.length, (size_t)4)) == 0)
	{
		unique_lock<mutex> lck(writeLock);

		string content = "<h1>It really works!</h1><p>" /*+ to_string(rand()) +*/ "</p>";
		string header = "HTTP/1.0 200 OK\r\nServer: Wandai/0.1\r\nConnection: close\r\nContent-Type: text/html\r\nContent-Length: " + to_string(content.length()) + "\r\n\r\n";
		string data = header + content;

		bufferevent_write(buffev, data.c_str(), data.length());
		/*Package *pack = (Package*)malloc(sizeof(PackageHeader) + 1024 * 1024 * 1024);
		pack->header.length = 1024 * 1024 * 1024;
		SendPackage(pack);*/
		FlushAndClose();
	}
	
	delete pack;
	pack = NULL;
}

void Session::SendPackage(Package *&pack)
{
	if (!pack || pack->header.length == 0) return;
	pack->header.length = htonpacklen(pack->header.length);
	//writtenEvent.Reset();
	pendingPackages.push(pack);

	DoQueue();
}

void Session::SendPackage(string str)
{
	Package *pack = MakePackage(str);
	SendPackage(pack);
}

void Session::FlushAndClose()
{
	closeAfterWritten = true;
}

void Session::Close()
{
	if (!IsAlive) return;
	IsAlive = false;

	debug("fd = %u, closing", (unsigned int)fd);
	
	if (buffev)
	{
		ClearCallbacks(); // need clear callbacks BEFORE shutdown.
	}
	
	//writtenEvent.Set();
	
	if (fd)
	{
		shutdown(fd, SHUT_RDWR);
		close(fd);
		fd = NULL;
	}
	
	if (buffev)
	{
		bufferevent_free(buffev);
		buffev = NULL;
	}
	
	if (currentPackage)
	{
		delete currentPackage;
		currentPackage = NULL;
	}
	
	while (!pendingPackages.empty())
	{
		Package *&pack = pendingPackages.front();
		if (pack)
		{
			delete pack;
			pack = NULL;
		}
		pendingPackages.pop();
	}
}

Package *Session::MakePackage(string &strdata)
{
	package_len_t len = (package_len_t)((strdata.length() + 1) * sizeof(char));
	Package *pack = (Package*)malloc(sizeof(PackageHeader) + len);
	memcpy(pack->header.magic, MAGIC_MARK, sizeof(MAGIC_MARK) - 1);
	pack->header.length = len;
	memcpy(pack->data, strdata.c_str(), len);
	return pack;
}

package_len_t htonpacklen(package_len_t len)
{
	if (sizeof(package_len_t) == 2)
	{
		return (package_len_t)htons(len);
	}
	else if (sizeof(package_len_t) == 4)
	{
		return (package_len_t)htonl(len);
	}
}

package_len_t ntohpacklen(package_len_t len)
{
	if (sizeof(package_len_t) == 2)
	{
		return (package_len_t)ntohs(len);
	}
	else if (sizeof(package_len_t) == 4)
	{
		return (package_len_t)ntohl(len);
	}
}

void readCallbackDispatcher(bufferevent *buffev, void *arg)
{
	((Session*)arg)->ReadCallback();
	((Session*)arg)->server.CleanSession((Session*)arg);
}

void writeCallbackDispatcher(bufferevent *buffev, void *arg)
{
	((Session*)arg)->WriteCallback();
	((Session*)arg)->server.CleanSession((Session*)arg);
}

void errorCallbackDispatcher(bufferevent *buffev, short event, void *arg)
{
	((Session*)arg)->ErrorCallback(event);
	((Session*)arg)->server.CleanSession((Session*)arg);
}
