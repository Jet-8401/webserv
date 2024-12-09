#include "../headers/HttpGetDirectory.hpp"
HttpGetDirectory::HttpGetDirectory(const HttpParser& parser): HttpParser(parser)
{
	this->_request.setEvents(EPOLLOUT);
}
