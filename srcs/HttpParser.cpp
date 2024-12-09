#include "../headers/WebServ.hpp"
#include "../headers/HttpParser.hpp"
#include "../headers/HttpGetStaticFile.hpp"
#include <cstddef>
#include <cstring>
#include <sys/epoll.h>
#include <sys/types.h>

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpParser::HttpParser(const ServerConfig& config):
	_need_upgrade(false),
	_request(config, _response),
	_response(_request),
	state(READING_HEADERS, false)
{}

HttpParser::HttpParser(const HttpParser& parser):
	_need_upgrade(parser._need_upgrade),
	_request(parser._request),
	_response(parser._request),
	state(parser.state)
{}

HttpParser::~HttpParser(void)
{}

// Getters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpRequest&	HttpParser::getRequest(void)
{
	return (this->_request);
}

 HttpResponse&	HttpParser::getResponse(void)
{
	return (this->_response);
}

const bool&	HttpParser::checkUpgrade(void) const
{
	return (this->_need_upgrade);
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

bool	HttpParser::parse(const uint8_t* packet, const size_t packet_len)
{
	do {
		DEBUG("entering the switch in HttpParser::parse with code: " << this->state.flag);
		switch (this->state.flag) {
			case READING_HEADERS:
				this->state = this->_request.bufferHeaders(packet, packet_len);
				break;
			case PARSE_HEADERS:
				this->state = this->_request.parseHeaders();
				break;
			case VALIDATE_REQUEST:
				this->state = this->_request.validateAndInitLocation();
				break;
			case NEED_UPGRADE:
				// NEED_UPGRADE is the last state before giving the responsability to the upgraded class to handle
				// the rest of the request because we its not mandatory to wait for a body depending on the method.
				// Expl: If a POST request, its the its responsability to make the state.flag = READING_BODY in its constructor
				this->state = handler_state_t(READY_TO_SEND, false);
				this->_need_upgrade = true;
				break;
			case ERROR:
				DEBUG("error handling not supported");
				// fallthrough
			default:
				this->state.continue_loop = false;
				break;
		}
	} while (this->state.continue_loop);

	// if (iterations >= MAX_ITERATIONS) {
	// 	this->state = handler_state_t(ERROR, false);
	// 	DEBUG("Max iterations reach for HttpParser::parse!");
	// 	return (false);
	// }
	return (true);
}

// HttpParser::write will handle default errors, redirections and the sending of headers as those tasks are common
// to all request's methods.
ssize_t	HttpParser::write(const uint8_t* io_buffer, const size_t buff_len)
{
	std::streamsize	bytes_written = -1;

	if (this->_request.isError() && !this->_response.isError())
		this->state = this->_response.error(this->_request.getStatusCode());

	do {
		DEBUG("entering the switch in HttpParser::write with code: " << this->state.flag);
		switch (this->state.flag) {
			case ERROR:
				this->state = this->_response.handleError();
				break;
			// case REDIRECTION:
			// 	this->state = this->_response.handleRedirection();
			// 	break;
			case READY_TO_SEND:
				// fallthrough
			case BUILD_HEADERS:
				this->state = this->_response.buildHeaders();
				break;
			case SENDING_HEADERS:
				this->state = this->_response.sendHeaders(io_buffer, buff_len, bytes_written);
				break;
			default:
				this->state.continue_loop = false;
				break;
		}
	} while (this->state.continue_loop);

	// if (iterations >= MAX_ITERATIONS) {
	// 	this->state = handler_state_t(ERROR, false);
	// 	DEBUG("Max iterations reach for HttpParser::parse!");
	// 	return (-1);
	// }
	return (bytes_written);
}

HttpParser*	HttpParser::upgrade(void)
{
	const std::string&	method = this->_request.getMethod();

	if (!this->_need_upgrade)
		return (0);

	this->_need_upgrade = false;
	if (method == "GET") {
		return new HttpGetStaticFile(*this);
	}
	return (0);
}
