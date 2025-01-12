#include "../headers/HttpCGI.hpp"
#include "../headers/WebServ.hpp"
#include "../headers/Socket.hpp"
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <string>

extern char** environ;

size_t HttpCGI::_child_procs = 0;

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpCGI::HttpCGI(const HttpParser& parser):
	HttpParser(parser),
	_is_post(this->_request.getMethod() == "POST"),
	_bytes_passed_through(0),
	_cgi_pid(-1),
	_are_headers_processed(false),
	_cgi_eof(false),
	_event(0)
{
	if (!this->_setupPipes())
		return;

	if (this->_is_post) {
		if (!this->_postPreamble())
			return;
	} else {
		this->_request.setEvents(EPOLLOUT);
		this->_state = handler_state_t(READY_TO_SEND, true);
	}

	if (!this->_executeCGI())
		return;
}

HttpCGI::~HttpCGI(void)
{
	DEBUG("HttpCGI Destructor");

	if (this->_event)
		this->_socket_referer.getEventWrapper().remove(this->_event);

	int	ios[4] = { this->_in[0], this->_in[1], this->_out[0], this->_out[1] };
	for (unsigned long i = 0; i < 4; i++)
		if (ios[i] != -1)
			::close(ios[i]);

	if (this->_cgi_pid != -1) {
		int status = 0;

		::waitpid(this->_cgi_pid, &status, 0);
		if (!WIFEXITED(status))
			::kill(this->_cgi_pid, SIGKILL);
		HttpCGI::_child_procs--;
	}
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

bool	HttpCGI::_setupPipes(void)
{
	int*	ios[4] = { &this->_in[0], &this->_in[1], &this->_out[0], &this->_out[1] };
	for (unsigned long i = 0; i < 4; i++)
		*ios[i] = -1;

	// for both set output pipe
	if (::pipe(this->_out) == -1) {
		error(ERR_PIPE_CREATE, true);
		this->_state = this->_response.error(500);
		return (false);
	}

	// setup in pipe for POST method
	if (this->_is_post) {
		if (::pipe(this->_in) == -1) {
			error(ERR_PIPE_CREATE, true);
			this->_state = this->_response.error(500);
			return (false);
		}
	}

	// manage cgi output via epoll
	this->_event = this->_socket_referer.getEventWrapper().create(CGI_OUTPUT);
	struct epoll_event	ep_event;
	if (!this->_event) {
		DEBUG("NULL POINTER DETECTED in HttpCGI");
		this->_state = this->_response.error(500);
		return (false);
	}
	this->_event->casted_value = this;
	this->_event->fd = this->_out[0];
	ep_event.events = EPOLLIN | EPOLLET;
	ep_event.data.ptr = static_cast<void*>( this->_event );
	if (::epoll_ctl(this->_socket_referer.getEpollFD(), EPOLL_CTL_ADD, this->_event->fd, &ep_event) == -1) {
		error(ERR_EPOLL_ADD, true);
		this->_state = this->_response.error(500);
		return (false);
	}
	return (true);
}

bool	HttpCGI::_postPreamble(void)
{
	char			tmp_buffer[512];
	StreamBuffer&	body = this->_request.getBody();
	ssize_t			bytes;

	const Location* location = this->_request.getMatchingLocation();
	if (!location) {
		DEBUG("NULL POINTER DETECTED");
		return (this->_state = this->_response.error(500), false);
	}
	this->_max_bytes_through = location->getClientMaxBodySize();

	// check if we received the full body
	std::string content_length = this->_request.getHeader("Content-Length");
	if (content_length.empty()) {
		return (this->_state = this->_request.error(411), false);
	}

	// unbuffer all the body into the pipe
	this->_bytes_passed_through = body.bytesPassedThrough();
	while((bytes = body.consume(tmp_buffer, sizeof(tmp_buffer))) > 0) {
		std::cout.write(tmp_buffer, bytes);
		if (::write(this->_in[1], tmp_buffer, bytes) == -1)
			return (this->_state = this->_response.error(500), false);
	}

	if (body.bytesPassedThrough() == (size_t) std::atoi(content_length.c_str())) {
		// we have the full body, we can send the executed CGI
		this->_state = handler_state_t(READY_TO_SEND, true);
		this->_request.setEvents(EPOLLOUT);
	} else {
		this->_state = handler_state_t(READING_BODY, false);
	}
	return (true);
}

bool	HttpCGI::_executeCGI(void)
{
	if (HttpCGI::_child_procs >= 20) {
		this->_state = this->_request.error(503);
		return (false);
	}

	this->_cgi_pid = ::fork();
	if (this->_cgi_pid == -1) {
		this->_state = this->_request.error(500);
		return (false);
	}

	if (this->_cgi_pid == 0) { // Child process
		std::string extension(::strrchr(this->_request.getResolvedPath().c_str(), '.'));
		char* const args[] = {
			const_cast<char*>(this->_request.getMatchingLocation()->getCGIs().find(extension)->second.c_str()),
			const_cast<char*>(this->_request.getResolvedPath().c_str()),
			NULL
		};
		char** env = this->_prepCGIEnvironementVariables();

		this->_setupCGIIORedirections();

		if (::execve(args[0], args, env) == -1)
			error("Error while executing CGI", true);
		free_env(env);
		std::exit(1);
	}

	HttpCGI::_child_procs++;
	// closing read end of input and write end of output into the parent
	if (this->_in[0] != -1) {
		::close(this->_in[0]);
		this->_in[0] = -1;
	}
	::close(this->_out[1]);
	this->_out[1] = -1;
	return (true);
}

bool HttpCGI::_processCgiHeader(void)
{
	ssize_t bytes;
	char* dest = 0;

	bytes = this->_cgi_output.consume_until(
		(void **)&dest,
		(char*) HttpRequest::END_SEQUENCE,
		sizeof(HttpRequest::END_SEQUENCE));

	if (bytes < 0) {
		delete [] dest;
		return (error(ERR_BUFF_CONSUME, true), this->_state = this->_response.error(500), false);
	} else if (bytes == 0) {
		return (false);
	}

	DEBUG("number of bytes consume: " << bytes);
	this->_parseAndSetHeaders(dest, bytes);
	if (dest)
		delete [] dest;
	return (true);
}

void	HttpCGI::_parseAndSetHeaders(char* dest, size_t size)
{
	std::stringstream	buffer;
	std::string			line, key, value;

	buffer.write(dest, size);

	while (std::getline(buffer, line)) {
		if (line.empty())
			continue;
		size_t	colon_pos = line.find(':');
		if (colon_pos == std::string::npos)
			continue;

		// separate the key and value, then triming them
		key = line.substr(0, colon_pos);
		string_trim(key);
		value = line.substr(colon_pos + 1);
		string_trim(value);
		this->_response.setHeader(key, value);
	}
}

bool	HttpCGI::_setupCGIIORedirections(void)
{
	// replacing stdout with the write end of output and stdin with read end of input
	if (this->_in[0] != -1)
		::dup2(this->_in[0], STDIN_FILENO);
	else
		::close(0);
	::dup2(this->_out[1], STDOUT_FILENO);
	// closing the unused file descriptors
	if (this->_in[1] != -1)
		::close(this->_in[1]);
	::close(this->_out[0]);
	return (true);
}

char**	HttpCGI::_prepCGIEnvironementVariables(void)
{
	std::map<std::string, std::string>	env;
	const HttpRequest&					req = this->_request;
	const std::string&					path = req.getPath();
	size_t								query_pos = path.find('?');

	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["REQUEST_METHOD"] = req.getMethod();
	env["SCRIPT_NAME"] = req.getConfigLocationStr();
	env["SCRIPT_FILENAME"] = req.getResolvedPath();
	env["SERVER_SOFTWARE"] = SERVER_VERSION;
	env["SERVER_NAME"] = this->_socket_referer.getIPV4();
	env["SERVER_PORT"] = unsafe_itoa(this->_socket_referer.getPort());
	env["QUERY_STRING"] = (query_pos != std::string::npos) ? path.substr(query_pos + 1) : "";
	env["PATH_INFO"] = (query_pos != std::string::npos) ? path.substr(0, query_pos) : path;

	std::string content_type = req.getHeader("Content-Type");
	std::string content_length = req.getHeader("Content-Length");
	std::string cookie = req.getHeader("Cookie");
	DEBUG("Raw Cookie header: [" << cookie << "]");

	if (!content_type.empty())
		env["CONTENT_TYPE"] = content_type;
	if (!content_length.empty())
		env["CONTENT_LENGTH"] = content_length;
	if (!cookie.empty()) {
		std::string combined;
		HttpMessage::headers_range_t range = req.getHeaders("Cookie");
		DEBUG("Processing multiple cookies:");
		for (HttpMessage::headers_t::const_iterator it = range.first; it != range.second; ++it) {
			DEBUG("  Cookie entry: [" << it->second << "]");
			if (!combined.empty())
				combined += "; ";
			combined += it->second;
		}
		env["HTTP_COOKIE"] = combined;
		DEBUG("Final HTTP_COOKIE env: [" << combined << "]");
	}

	std::string extension(::strrchr(req.getResolvedPath().c_str(), '.'));
	if (extension == ".php") {
		env["REDIRECT_STATUS"] = "200";
		env["PHP_SELF"] = req.getConfigLocationStr();
	}
	else if (extension == ".py") {
		env["PYTHONPATH"] = ".:/usr/local/lib/python";
		env["PYTHONIOENCODING"] = "utf-8";
	}

	char** envp = new char*[env.size() + 1];
	size_t i = 0;
	for (std::map<std::string, std::string>::const_iterator it = env.begin(); it != env.end(); ++it)
		envp[i++] = ::strdup((it->first + "=" + it->second).c_str());
	envp[i] = NULL;

	return envp;
}

bool	HttpCGI::parse(const uint8_t* packet, const size_t packet_size)
{
	if (this->_state.flag != READING_BODY)
		return (this->HttpParser::parse(packet, packet_size));

	if (this->_bytes_passed_through + packet_size > this->_max_bytes_through) {
		return (this->_state = this->_request.error(413), false);
	}

	if (::write(this->_in[1], packet, packet_size) == -1) {
		this->_state = this->_request.error(500);
		return false;
	}
	this->_bytes_passed_through += packet_size;
	return (true);
}

ssize_t HttpCGI::write(uint8_t* io_buffer, const size_t buff_len)
{
	if (this->_cgi_pid == -1 && !this->_response.isError()) {
		this->_state = this->_response.error(500);
	}

	if (this->_response.isError() || this->_request.isError())
		return (this->HttpParser::write(io_buffer, buff_len));

	int status = 0, exit_status;
	::waitpid(this->_cgi_pid, &status, WNOHANG);
	if (!WIFEXITED(status)) {
		return (0);
	}

	exit_status = WEXITSTATUS(status);
	if (exit_status != 0) {
		DEBUG("CGI exited with error status of: " << exit_status);
		this->_state = this->_response.error(500);
		return (0);
	}

	if (!this->_cgi_eof)
		return (0);

	if (this->_state.flag != SENDING_BODY)
		return (this->HttpParser::write(io_buffer, buff_len));

	ssize_t bytes = this->_cgi_output.consume(io_buffer, buff_len);
	if (bytes == -1)
		return (error(ERR_BUFF_CONSUME, true), this->_state = this->_response.error(500), -1);
	else if (bytes == 0)
		this->_state = handler_state_t(DONE, true);
	return (bytes);
}

void	HttpCGI::onDataOutput(::uint32_t events)
{
	uint8_t	io_buffer[PACKETS_SIZE];
	ssize_t	bytes = 0;

	if (events & EPOLLHUP) {
		while ((bytes = ::read(this->_out[0], io_buffer, sizeof(io_buffer))) > 0) {
			this->_cgi_output.write(io_buffer, bytes);
		}
	} else if (events & EPOLLIN) {
		bytes = ::read(this->_out[0], io_buffer, sizeof(io_buffer));
		if (bytes > 0)
			this->_cgi_output.write(io_buffer, bytes);
	}

	if (bytes == -1) {
		error("Cannot read through pipe", true);
		this->_state = this->_response.error(500);
		return;
	} else if (bytes == 0) {
		if (::epoll_ctl(this->_socket_referer.getEpollFD(), EPOLL_CTL_DEL, this->_out[0], 0) == -1) {
			error(ERR_EPOLL_DEL, true);
		}
		this->_cgi_eof = true;
		::close(this->_out[0]);
		this->_out[0] = -1;
	}

	// try to parse headers
	if (!this->_are_headers_processed && this->_processCgiHeader()) {
		this->_are_headers_processed = true;
	}
}
