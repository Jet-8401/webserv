#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

class HttpRequest {
	private:
		bool	_parsing_done;
		bool	_write_available;

	public:
		void	parse(const int socket_fd);
};

#endif
