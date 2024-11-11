#include "../headers/Connection.hpp"
#include <stdint.h>

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

Connection::Connection(const int client_socket_fd, const HttpServer& server_referrer):

	_socket(client_socket_fd),
	_server_referrer(server_referrer)
{}

Connection::~Connection(void)
{}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

void	Connection::onEvent(::uint32_t events)
{

}
