#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>

#define ERR_SOCKET_CREATION "Could not create the socket"
#define ERR_SOCKET_BINDING "Error while binding the socket"
#define ERR_SOCKET_LISTENING "Could not listen to host"
#define ERR_SOCKET_REQUEST "Request error"
#define ERR_SENDING_RESPONSE "Could not send the response"

#define LISTENING_PORT 5503

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

int	main(void)
{
	int			sockfd, datafd;
	sockaddr_in	addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
		return (error(ERR_SOCKET_CREATION), 0);

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(LISTENING_PORT);
	addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(sockfd, (sockaddr*) &addr, sizeof(addr)) == -1)
		return (close(sockfd), error(ERR_SOCKET_BINDING), 0);

	if (listen(sockfd, 200) == -1)
		return (close(sockfd), error(ERR_SOCKET_LISTENING), 0);

	std::cout << "listening on localhost:" << LISTENING_PORT << std::endl;
	while (42) {
		std::string	input;

		std::cout << "Pending data" << std::endl;
		datafd = accept(sockfd, NULL, NULL);
		if (datafd < 0)
			return (close(sockfd), error(ERR_SOCKET_REQUEST), 0);

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
	}
	close(sockfd);
	return (0);
}
