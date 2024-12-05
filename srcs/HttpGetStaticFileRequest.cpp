#include "../headers/HttpGetStaticFileRequest.hpp"
#include <fcntl.h>
#include <unistd.h>

HttpGetStaticFileRequest::HttpGetStaticFileRequest(const ServerConfig& config)
    : HttpRequest(config), _file_fd(-1)
{}

HttpGetStaticFileRequest::~HttpGetStaticFileRequest()
{
    _closeFile();
}

bool HttpGetStaticFileRequest::parse(const uint8_t* packet, const size_t packet_size)
{
    if (!HttpRequest::parse(packet, packet_size))
        return false;

    if (this->_state == CHECK_METHOD) {
        if (!_openFile())
            return false;
        this->_state = DONE;
    }
    return true;
}

ssize_t HttpGetStaticFileRequest::writePacket(uint8_t* io_buffer, size_t buff_length)
{
    if (_file_fd == -1)
        return -1;
    return read(_file_fd, io_buffer, buff_length);
}

bool HttpGetStaticFileRequest::_openFile()
{
    _file_path = _matching_location->getRoot() + _path;
    _file_fd = open(_file_path.c_str(), O_RDONLY);
    return _file_fd != -1;
}

void HttpGetStaticFileRequest::_closeFile()
{
    if (_file_fd != -1) {
        close(_file_fd);
        _file_fd = -1;
    }
}
