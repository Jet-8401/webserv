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
	this->_state = handler_state_t(READY_TO_SEND, true);

	std::cout << "{" << this->_request.getResolvedPath().c_str() << "}" << std::endl;
	this->_file_fd = open(this->_request.getResolvedPath().c_str(), O_RDONLY);
	if (this->_file_fd == -1) {
		std::cerr << "could not open file" << std::endl;
		this->_state = this->_request.error(404);
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

	if (this->_state.flag != SENDING_BODY)
		return (this->HttpParser::write(io_buffer, buff_length));

	if (_file_fd == -1) {
		this->_state = handler_state_t(ERROR, true);
		return (-1);
	}
	bytes_read = read(_file_fd, const_cast<uint8_t*>(io_buffer), buff_length);
	if (bytes_read == 0) {
		this->_state = handler_state_t(DONE, true);
		return (0);
	}
	return (bytes_read);
}
