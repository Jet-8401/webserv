#ifndef HTTP_GET_CGI_HPP
# define HTTP_GET_CGI_HPP

# include "HttpParser.hpp"

class HttpGetCGI : public HttpParser {
	private:
		pid_t		_cgi_pid;
		int			_pipe_out[2];

		char**	_prepare_env(void);
		void	_free_env(char** env);
		void	executeCGI(void);

	public:
		HttpGetCGI(const HttpParser& parser);
		virtual ~HttpGetCGI(void);

		bool	parse(const uint8_t* packet, const size_t packet_size);
		ssize_t	write(uint8_t* io_buffer, const size_t buff_length);
};

#endif
