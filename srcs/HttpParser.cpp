#include "../headers/WebServ.hpp"
#include "../headers/HttpParser.hpp"
#include "../headers/HttpGetStaticFile.hpp"
#include <cstddef>
#include <cstring>
#include <sys/epoll.h>
#include <sys/types.h>

#define MAX_ITERATIONS 100000

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
	size_t	iterations = 0;

	// For debugs
	if (packet_len == 0)
		DEBUG("RECEIVED A PACKET OF SIZE 0, (end of transmission)");

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
				this->state = parsing_state_t(DONE, true);
				this->_need_upgrade = true;
				break;
			case DONE:
				this->_request.setEvents(EPOLLOUT);
				this->state.continue_loop = false;
				break;
			case ERROR:
				// fallthrough
				this->state.continue_loop = false;
				DEBUG("error handling not supported");
				break;
			default:
				this->state.continue_loop = false;
				break;
		}
	} while (this->state.continue_loop && iterations++ < MAX_ITERATIONS);

	if (iterations >= MAX_ITERATIONS) {
		DEBUG("Max iterations reach for HttpParser::parse!");
		return (false);
	}

	return (true);
}

ssize_t	HttpParser::write(const uint8_t* io_buffer, const size_t buff_len)
{
	(void) io_buffer;
	(void) buff_len;
	std::cout << "gjkqgmqzjmgoqizjmgoqzigjqzoeigjqzmoegijqzemgjqzgjiqmzogimzqoiegjmqzoigjmzqoigjemzoqigj \
	qzegiqzjgeùzqijegqziejgmzqgjiemzqogijqzmgoijzqmgjezq \
	gqzgeoziqjgzjioqegijqz" << std::endl;
	return (0);
}

HttpParser*	HttpParser::upgrade(void)
{
	const std::string&	method = this->_request.getMethod();

	if (!this->_need_upgrade)
		return (0);

	if (method == "GET") {
		return new HttpGetStaticFile(*this);
	}
	return (0);
}
