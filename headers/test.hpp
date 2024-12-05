/*

#include "BytesBuffer.hpp"
#include "Connection.hpp"
#include "StreamBuffer.hpp"
#include <cstdio>
#include <map>
#include <sstream>
#include <string>

class HttpGetRequest : public HttpRequest {
	public:
		HttpGetRequest(void);
		virtual ~HttpGetRequest(void);
};

class HttpPostRequest : public HttpRequest {
	public:
		HttpPostRequest(void);
		virtual ~HttpPostRequest(void);

	private:
};
*/
/*

Connection::onEvent(::uint32_t events)
{
	uint8_t	io_buffer[2048];

	if (events & EPOLLIN) {
		ssize_t bytes = recv(this->socket_fd, io_buffer, sizeof(io_buffer));
		request->parse(io_buffer, bytes);
	}

	if (events & EPOLLOUT) {
		this->response.writePacket(io_buffer, sizeof(io_buffer));
		//  write to socket while checking for unsend data (write return less than asked)
	}
}

*/
