#include "../headers/HttpGetDirectory.hpp"
#include <sys/epoll.h>

HttpGetDirectory::HttpGetDirectory(const HttpParser& parser): HttpParser(parser)
{
	this->_request.setEvents(EPOLLOUT);
}
