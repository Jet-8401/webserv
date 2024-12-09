#include "../headers/HttpGetStaticFile.hpp"
#include <fcntl.h>
#include <ios>
#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>

HttpGetStaticFile::HttpGetStaticFile(const HttpParser& parser):
	HttpParser(parser),
	_file_fd(-1)
{
	// switching directly to EPOLLOUT
	this->_request.setEvents(EPOLLOUT);

	_file_path = _request.getMatchingLocation().getRoot() + _request.getPath();
	std::cout << _file_path << std::endl;
	_file_fd = open(_file_path.c_str(), O_RDONLY);
	if (this->_file_fd == -1) {
		std::cerr << "could not open file" << std::endl;
		this->state = this->_request.error(404);
	}
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
	(void) packet;
	(void) packet_size;
	return (true);
}

ssize_t HttpGetStaticFile::write(const uint8_t* io_buffer, const size_t buff_length)
{
	std::streamsize	bytes_read;

	if (this->state.flag != SENDING_BODY)
		return (this->HttpParser::write(io_buffer, buff_length));

	_response.error(404);

	if (_file_fd == -1) {
		this->state = handler_state_t(DONE, true);
		return (0);
	}
	bytes_read = read(_file_fd, const_cast<uint8_t*>(io_buffer), buff_length);
	if (bytes_read == 0) {
		this->state = handler_state_t(DONE, true);
		return (0);
	}
	return (bytes_read);
}
