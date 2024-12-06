#include "../headers/HttpGetStaticFile.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

HttpGetStaticFile::HttpGetStaticFile(const HttpParser& parser):
	HttpParser(parser),
	_file_fd(-1)
{
	_file_path = _request.getMatchingLocation().getRoot() + _request.getPath();
	std::cout << _file_path << std::endl;
    _file_fd = open(_file_path.c_str(), O_RDONLY);
    if (this->_file_fd == -1)
    	std::cerr << "could not open file" << std::endl;
}

HttpGetStaticFile::~HttpGetStaticFile()
{
	if (_file_fd != -1) {
        close(_file_fd);
        _file_fd = -1;
    }
}

bool HttpGetStaticFile::parse(const uint8_t* packet, const size_t packet_size)
{
	std::cout << "c'est DONE enculer" << std::endl;
	(void) packet;
	(void) packet_size;
	return (true);
}

ssize_t HttpGetStaticFile::write(const uint8_t* io_buffer, const size_t buff_length)
{
	std::cout << "poihfpzeofhzep" << std::endl;
    if (_file_fd == -1)
        return -1;
    return read(_file_fd, const_cast<uint8_t*>(io_buffer), buff_length);
}
