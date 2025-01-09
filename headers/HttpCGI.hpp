#ifndef HTTP_CGI_HPP
# define HTTP_CGI_HPP

# include "../headers/HttpParser.hpp"
# include "../headers/EventWrapper.hpp"
# include <cstddef>

class HttpCGI : public HttpParser {
	private:
		bool				_is_post;
		size_t				_bytes_passed_through;
		size_t				_max_bytes_through;

		pid_t   			_cgi_pid;

		int					_in[2];
		int					_out[2];
		StreamBuffer		_cgi_output;

		bool				_are_headers_processed;

		event_wrapper_t*	_event;

		bool	_setupPipes(void);
		bool	_postPreamble(void);
		bool	_executeCGI(void);
		bool	_processCgiHeader();
		void	_parseAndSetHeaders(char* dest, size_t size);

	public:
		HttpCGI(const HttpParser& parser);
		virtual ~HttpCGI();

		virtual bool	parse(const uint8_t* packet, const size_t packet_size);
		virtual ssize_t write(uint8_t* io_buffer, const size_t buff_len);

		void	onDataOutput(void);
};

#endif
