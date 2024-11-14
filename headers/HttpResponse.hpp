#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

class HttpResponse {
	public:
		HttpResponse(void);
		virtual ~HttpResponse(void);

		unsigned short	status_code;

		int	send(const int socket_fd);
};

#endif
