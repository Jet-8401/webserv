#ifndef HTTP_PARSER_HPP
# define HTTP_PARSER_HPP

class Socket;

# include "CommonDefinitions.hpp"
# include "HttpResponse.hpp"
# include "HttpRequest.hpp"
# include <ios>
# include <sstream>
# include <stdint.h>
# include <sys/types.h>

class HttpParser {
	private:
		bool						_need_upgrade;

	protected:
		HttpParser(const HttpParser& parser);

		HttpRequest					_request;
		HttpResponse				_response;
		Socket&						_socket_referer;

		handler_state_t				_state;

		handler_state_t				_sendingErrorPage(const uint8_t* io_buffer, const size_t buff_len,
										std::streamsize& bytes_written);
		bool						_has_error;
		std::string					_error_page_path;
		int							_error_page_fd;
		// std::stringstream			_generated_error_page;

	public:
		HttpParser(Socket& socket_referer);
		virtual ~HttpParser(void);

		virtual bool				parse(const uint8_t* packet, const size_t packet_len);
		virtual ssize_t				write(const uint8_t* io_buffer, const size_t buff_len);
		handler_state_t				handleError(void);

		HttpResponse&				getResponse(void);
		HttpRequest&				getRequest(void);
		const bool&					checkUpgrade(void) const;
		const enum handler_state_e&	getState(void) const;

		HttpParser*					upgrade(void);
};

#endif
