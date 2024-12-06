#include "../headers/HttpGetStaticMethod.hpp"
#include "../headers/HttpHandler.hpp"
#include <fcntl.h>
#include <sstream>
#include <unistd.h>

HttpGetStaticMethod::HttpGetStaticMethod(HttpHandler& handler) :
    AHttpMethod(handler),
    _file_fd(-1),
    _bytes_sent(0)
{}

HttpGetStaticMethod::~HttpGetStaticMethod(void)
{
    if (this->_file_fd != -1)
        ::close(this->_file_fd);
}

bool HttpGetStaticMethod::parse(const uint8_t* packet, const size_t packet_size)
{
    (void)packet;
    (void)packet_size;

    return this->_resolveLocation()
        && S_ISREG(this->_file_stat.st_mode)
        && this->_openFile()
        && this->_setHeaders();
}

bool HttpGetStaticMethod::_openFile(void)
{
    if (access(this->_complete_path.c_str(), R_OK) == -1) {
        this->referer.setStatusCode(403);
        return false;
    }

    this->_file_fd = open(this->_complete_path.c_str(), O_RDONLY);
    if (this->_file_fd == -1) {
        this->referer.setStatusCode(500);
        return false;
    }

    return true;
}

bool HttpGetStaticMethod::_setHeaders(void)
{
    std::stringstream size_str;
    size_str << this->_file_stat.st_size;

    std::string extension = this->_complete_path.substr(
        this->_complete_path.find_last_of(".")
    );

    this->referer.addHeader("Content-Length", size_str.str());
    this->referer.addHeader("Content-Type",
        HttpHandler::mime_types[extension]);
    this->referer.setStatusCode(200);

    return true;
}

ssize_t HttpGetStaticMethod::writePacket(uint8_t* io_buffer, size_t buff_length)
{
    ssize_t bytes = ::read(this->_file_fd, io_buffer, buff_length);
    if (bytes == -1) {
        this->referer.setStatusCode(500);
        return -1;
    }
    this->_bytes_sent += bytes;
    return bytes;
}
