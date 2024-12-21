#include "../headers/HttpDelete.hpp"
#include "../headers/WebServ.hpp"
#include <sys/epoll.h>
#include <sys/stat.h>

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpDelete::HttpDelete(const HttpParser& src):
	HttpParser(src)
{
	const struct stat& file_stat = this->_request.getPathStat();

	this->_state = handler_state_t(READY_TO_SEND, false);
	this->_request.setEvents(EPOLLOUT);

	if (S_ISDIR(file_stat.st_mode)) {
		this->_response.error(403);
		return;
	} else if (S_ISREG(file_stat.st_mode)) {
		const std::string& resolved_path = this->_request.getResolvedPath();
		if (std::remove(resolved_path.c_str()) != 0) {
			this->_response.error(500);
			DEBUG("could not delete" << resolved_path);
			error("Could not delete file", true);
			return;
		}
		this->_response.setStatusCode(204);
	}
}

HttpDelete::~HttpDelete(void)
{}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

bool	HttpDelete::parse(const uint8_t* packet, const size_t packet_len)
{
	return (HttpParser::parse(packet, packet_len));
}

ssize_t	HttpDelete::write(const uint8_t* io_buffer, const size_t buff_len)
{
	return (HttpParser::write(io_buffer, buff_len));
}
