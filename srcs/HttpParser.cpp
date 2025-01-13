#include "../headers/HttpParser.hpp"
#include "../headers/WebServ.hpp"
#include "../headers/HttpGetStaticFile.hpp"
#include "../headers/HttpPost.hpp"
#include "../headers/HttpGetDirectory.hpp"
#include "../headers/HttpGetCGI.hpp"
#include "../headers/HttpPostCGI.hpp"
#include "../headers/HttpDelete.hpp"
#include "../headers/HttpCGI.hpp"
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <sstream>
#include <string>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpParser::HttpParser(Socket& socket_referer):
	_need_upgrade(false),
	_s_timeout_value(BASE_TIMEOUT),
	_request(_response, socket_referer),
	_response(_request),
	_socket_referer(socket_referer),
	_state(READING_HEADERS, false),
	_has_error(false),
	_error_page_fd(-1)
{}

HttpParser::HttpParser(const HttpParser& src):
	_need_upgrade(src._need_upgrade),
	_s_timeout_value(src._s_timeout_value),
	_request(src._request),
	_response(src._request),
	_socket_referer(src._socket_referer),
	_state(src._state),
	_has_error(src._has_error),
	_error_page_fd(src._error_page_fd)
{}

HttpParser::~HttpParser(void)
{
	if (this->_error_page_fd != -1)
		::close(this->_error_page_fd);
}

// Setters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

void	HttpParser::_setTimeoutValue(time_t seconds)
{
	DEBUG("Timeout changed to " << seconds << " seconds!");
	this->_s_timeout_value = seconds;
}

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

const enum handler_state_e&	HttpParser::getState(void) const
{
	return (this->_state.flag);
}

const time_t&	HttpParser::getSecTimeoutValue(void) const
{
	return (this->_s_timeout_value);
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

handler_state_t	HttpParser::_sendingErrorPage(uint8_t* io_buffer, const size_t buff_len,
	std::streamsize& bytes_written)
{
	// if there is a valid fd send the error page based on that file
	if (this->_error_page_fd != -1)
		return (this->_response.sendBody(io_buffer, buff_len, bytes_written, this->_error_page_fd));
	// else send that error page based on the cached default error pages
	else if (this->_generated_error_page.tellp() > 0)
		return (this->_response.sendBody(io_buffer, buff_len, bytes_written, this->_generated_error_page));
	return (handler_state_t(DONE, true));
}

bool	HttpParser::_do_custom_error(void)
{
	const Location*								location = this->_request.getMatchingLocation();
	std::map<int, std::string*>::const_iterator	err_page;
	struct stat									file_stats;

	if (!location)
		return (false);

	err_page = location->getErrorPages().find(this->_response.getStatusCode());
	if (err_page != location->getErrorPages().end()) {
		if (!err_page->second) {
			error("Error page str is NULL!", false);
			return (false);
		}

		this->_error_page_path = joinPath(location->getRoot(), *err_page->second);
		DEBUG("error page path: " << this->_error_page_path);
		if (::stat(this->_error_page_path.c_str(), &file_stats) == -1) {
			error(ERR_STAT, true);
			return (false);
		}

		if (S_ISDIR(file_stats.st_mode))
			return (false);

		this->_error_page_fd = ::open(this->_error_page_path.c_str(), O_RDONLY);
		if (this->_error_page_fd == -1) {
			error(ERR_READING_FILE, true);
			return (false);
		}

		return (true);
	}
	return (false);
}

// default hard-coded error page
bool	HttpParser::_generateError(const int status_code)
{
	const char*	message;

	message = HttpMessage::getStatusMessage(status_code);
	this->_generated_error_page << "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">" \
		"<title>" << message << " - " << status_code << "</title>" \
		"<style>* {margin: 0;padding: 0;box-sizing: border-box;font-family: 'Roboto', sans-serif;}" \
		"body {font-size: 140%;text-align: center;}" \
		"@media (prefers-color-scheme: dark) {body {color: rgb(220, 220, 220); background: rgb(5, 5, 5);}}" \
		"h1 {margin: 5vh 0;}" \
		"#credits {border-top: 1px solid grey;margin: 0 20vh;padding: 10px;}" \
		"</style></head><body>" \
		"<h1>" << status_code << " - " << message << "</h1>" \
		"<p id=\"credits\">" << SERVER_VERSION << "</p>" \
		"</body></html>";
	return (true);
}

bool	HttpParser::parse(const uint8_t* packet, const size_t packet_len)
{
	do {
		DEBUG("entering the switch in HttpParser::parse with code: " << this->_state.flag);
		switch (this->_state.flag) {
			case READING_HEADERS:
				this->_state = this->_request.bufferHeaders(packet, packet_len);
				break;
			case PARSE_HEADERS:
				this->_state = this->_request.parseHeaders();
				break;
			case VALIDATE_REQUEST:
				this->_state = this->_request.validateAndInitLocation();
				break;
			case NEED_UPGRADE:
				this->_state = handler_state_t(READING_BODY, false);
				this->_need_upgrade = true;
				break;
			case ERROR:
				// fallthrough
			default:
				this->_request.setEvents(EPOLLOUT);
				this->_state.continue_loop = false;
				break;
		}
	} while (this->_state.continue_loop);
	return (true);
}

// HttpParser::write will handle default errors, redirections and the sending of headers as those tasks are common
// to all request's methods.
ssize_t	HttpParser::write(uint8_t* io_buffer, const size_t buff_len)
{
	std::streamsize	bytes_written = -1;

	// check if an error previously occured into the request, if so the response inherit its status_code
	if (this->_request.isError() && !this->_response.isError())
		this->_state = this->_response.error(this->_request.getStatusCode());

	// check for redirections
	if (this->_request.isRedirection() && !this->_response.isRedirection()) {
		this->_response.setStatusCode(this->_request.getStatusCode());
		this->_response.setHeader("Location", this->_request.getMatchingLocation()->getRedirection().second);
	}

	do {
		DEBUG("entering the switch in HttpParser::write with code -> " << this->_state.flag);
		switch (this->_state.flag) {
			case READY_TO_SEND:
				// fallthrough
			case BUILD_HEADERS:
				this->_state = this->_response.buildHeaders();
				break;
			case SENDING_HEADERS:
				this->_state = this->_response.sendHeaders(io_buffer, buff_len, bytes_written);
				break;
			case SENDING_ERROR_FILE:
				this->_state = this->_sendingErrorPage(io_buffer, buff_len, bytes_written);
				break;
			case ERROR:
				this->_state = this->handleError();
				break;
			case DONE:
				bytes_written = 0;
				// fallthrough
			default:
				this->_state.continue_loop = false;
				break;
		}
	} while (this->_state.continue_loop);
	return (bytes_written);
}

// Set all the properties for the handling of an error base on the matching location.
handler_state_t	HttpParser::handleError(void)
{
	// check if an error previously occured into the request, if so the response inherit its status_code
	if (this->_request.isError() && !this->_response.isError())
		this->_response.error(this->_request.getStatusCode());

	if (!this->_do_custom_error()) {
		this->_generateError(this->_response.getStatusCode());
	}

	if (this->_error_page_fd != -1) {
		HttpResponse::mime_types_t::const_iterator	it;
		std::string	ext = this->_error_page_path.substr(this->_error_page_path.rfind('.'));
		struct stat	file_stats;

		it = HttpResponse::mime_types.find(ext);
		if (it != HttpResponse::mime_types.end())
			this->_response.setHeader("Content-Type", it->second);
		if (::stat(this->_error_page_path.c_str(), &file_stats) == -1)
			error(ERR_STAT, true);
		else
			this->_response.setHeader("Content-Length", unsafe_itoa(file_stats.st_size));
	} else if (this->_generated_error_page.tellp() > 0) {
		this->_response.setHeader("Content-Type", HttpResponse::mime_types[".html"]);
		this->_response.setHeader("Content-Length", unsafe_itoa(this->_generated_error_page.tellp()));
	}

	return (handler_state_t(BUILD_HEADERS, true));
}

HttpParser* HttpParser::upgrade(void)
{
    const std::string&  method = this->_request.getMethod();
    const std::string&  resolved_path = this->_request.getResolvedPath();
    const Location*     location = this->_request.getMatchingLocation();

    if (!this->_need_upgrade)
        return (0);
    this->_need_upgrade = false;

    // Check for CGI first for both GET and POST
    size_t ext_pos = resolved_path.rfind('.');
    if (ext_pos != std::string::npos) {
        std::string extension(resolved_path, ext_pos);
        if (location->getCGIs().find(extension) != location->getCGIs().end()) {
        	return (new HttpCGI(*this));
        }
    }

    if (method == "GET") {
        const struct stat& path_stat = this->_request.getPathStat();
        if (S_ISDIR(path_stat.st_mode) && location->getAutoIndex()) {
            return new HttpGetDirectory(*this);
        } else if (S_ISREG(path_stat.st_mode)) {
            return new HttpGetStaticFile(*this);
        }
    } else if (method == "POST") {
        return new HttpPost(*this);
    } else if (method == "DELETE") {
        return new HttpDelete(*this);
    }
    this->_request.error(404);
	this->_request.setEvents(EPOLLOUT);
    return NULL;
}
