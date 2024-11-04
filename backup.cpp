#include <cstdio>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>

#define ERR_SOCKET_CREATION "Could not create the socket"
#define ERR_SOCKET_BINDING "Error while binding the socket"
#define ERR_SOCKET_LISTENING "Could not listen to host"
#define ERR_SOCKET_REQUEST "Request error"
#define ERR_SENDING_RESPONSE "Could not send the response"
#define ERR_EPOLL_CREATE "Could not create an epoll instance"

#define LISTENING_PORT 5500

void	error(const std::string& errMessage)
{
	perror(errMessage.c_str());
}

void send_response(int& client_socket) {
    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<html><body><h1>Hello, World!</h1></body></html>";

    if (write(client_socket, response, strlen(response)) == -1)
    	error(ERR_SENDING_RESPONSE);
}

int	create_socket(void)
{
	int			socket_fd;
	sockaddr_in	addr;

	socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (socket_fd == -1)
		return (error(ERR_SOCKET_CREATION), 0);

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(LISTENING_PORT);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if (bind(socket_fd, (sockaddr*) &addr, sizeof(addr)) == -1)
		return (close(socket_fd), error(ERR_SOCKET_BINDING), -1);

	if (listen(socket_fd, 200) == -1)
		return (close(socket_fd), error(ERR_SOCKET_LISTENING), -1);
	return (socket_fd);
}

int	main(void)
{
	int	socket_fd, epoll_fd, client_fd;
	struct epoll_event	event, client_event;

	epoll_fd = epoll_create(1);
	if (epoll_fd == -1)
		return (error(ERR_EPOLL_CREATE), 0);
	socket_fd = create_socket();
	if (socket_fd == -1)
		return (0);
	event.events = EPOLLIN;
	event.data.fd = socket_fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event);

	std::cout << "listening on localhost:" << LISTENING_PORT << std::endl;
	while (42) {
		std::cout << "Pending request..." << std::endl;
		epoll_wait(epoll_fd, &client_event, 1, -1);

	}
	close(socket_fd);
	close(epoll_fd);
	return (0);
}

/*
std::string	input;

		std::cout << "Pending data" << std::endl;
		datafd = accept(socket_fd, NULL, NULL);
		if (datafd < 0)
			return (close(socket_fd), error(ERR_SOCKET_REQUEST), 0);

		char		buffer[1024];
		size_t		bytes_read;
		std::string	request;

		while ((bytes_read = read(datafd, buffer, sizeof(buffer) - 1)) > 0) {
          buffer[bytes_read] = '\0';
          request += buffer;

          // Check if we've reached the end of the HTTP headers
          if (request.find("\r\n\r\n") != std::string::npos)
              break;
      }
		std::cout << std::endl;

		std::cout << "returning basic response" << std::endl;
		send_response(datafd);
		close(datafd);

		std::cout << "Continue: ";
		std::cin >> input;
		if (input != "yes") {
			close(datafd);
			break ;
		}
*/
