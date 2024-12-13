#include "../headers/HttpGetDirectory.hpp"
#include "../headers/WebServ.hpp"
#include <sys/epoll.h>
#include <sys/stat.h>
#include <iostream>
#include <sstream>
#include <cstring>

HttpGetDirectory::HttpGetDirectory(const HttpParser& parser):
	HttpParser(parser),
	_dir(NULL)
{
	std::cout << "{" << this->_request.getResolvedPath().c_str() << "}" << std::endl;
	this->_dir = opendir(this->_request.getResolvedPath().c_str());
	if (this->_dir == NULL) {
		std::cerr << "could not open directory" << std::endl;
		this->_state = this->_request.error(404);
		return;
	}

	this->_request.setEvents(EPOLLOUT);
	this->_state = handler_state_t(READY_TO_SEND, false);
}

HttpGetDirectory::~HttpGetDirectory()
{
	if (_dir != NULL) {
		closedir(_dir);
		_dir = NULL;
	}
}

bool HttpGetDirectory::parse(const uint8_t* packet, const size_t packet_size)
{
	return (this->HttpParser::parse(packet, packet_size));
}

ssize_t HttpGetDirectory::write(const uint8_t* io_buffer, const size_t buff_length)
{
	static std::string		current_entry;
	static struct dirent* 	entry;
	static bool				headers_sent = false;

	if (this->_state.flag != SENDING_BODY)
		return (this->HttpParser::write(io_buffer, buff_length));

	if (_dir == NULL) {
		error("Dir point to NULL!", false);
		this->_response.error(500);
		return (-1);
	}

	if (!headers_sent) {
		std::stringstream html;
		html << "<!DOCTYPE html>\r\n<html>\r\n<head>\r\n"
		 << "<title>Index of " << this->_request.getPath() << "</title>\r\n"
		 << "</head>\r\n<body>\r\n"
		 << "<h1>Index of " << this->_request.getPath() << "</h1><hr>\r\n";
		current_entry = html.str();
		headers_sent = true;
	}

	if (current_entry.empty()) {
		entry = readdir(_dir);
		if (entry == NULL) {
			std::string footer = "</body>\r\n</html>\r\n";
			::memcpy(const_cast<uint8_t*>(io_buffer), footer.c_str(), footer.length());
			this->_state = handler_state_t(DONE, true);
			return footer.length();
		}

		std::stringstream line;
		line << "<p><a href=\"" << entry->d_name << "\">" << entry->d_name << "</a></p>\r\n";
		current_entry = line.str();
	}

	size_t to_send = std::min(current_entry.length(), buff_length);
	::memcpy(const_cast<uint8_t*>(io_buffer), current_entry.c_str(), to_send);
	current_entry = current_entry.substr(to_send);

	return (to_send);
}
