#include <stdint.h>
#include <cstddef>
#include <sys/types.h>

class HttpRequest;
class HttpResponse;

class HttpRequest {
	HttpRequest(const HttpResponse& response);
	const HttpResponse&	response;
};

class HttpResponse {
	HttpResponse(const HttpRequest& response);
	const HttpRequest&	response;
};

class HttpParser {
	HttpRequest		request;
	HttpResponse	response;

	// Handling default parsing of headers

	virtual bool	parse(uint8_t* packet, size_t packet_len);
	virtual ssize_t	write(uint8_t* io_buffer, size_t buff_len);

	void	handleRedirection();
	void	handleError();
};

class HttpGetHandler : public HttpParser {
	// Have method specific implementations
};

class Connection {
	HttpParser*	handler;
};

// in Connection.cpp
// -- -- -- -- -- --
// if (events & EPOLLIN) {
// 	if (handler->checkUpgrade()) {
// 		HttpParser* upgrade = handler->upgrade();
// 		delete this->handler;
// 		this->handler = upgrade;
// 	}
// }
