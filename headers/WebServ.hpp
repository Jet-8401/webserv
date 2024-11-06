#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <string>

# define PROG_NAME "webserv"
# define ERR_USAGE "usage: ./webserv <configFile>"
# define ERR_FILE_OPEN "Cannot open file: "
# define ERR_SOCKET_CREATION "Socket create failed"
# define ERR_BINDING_SOCKET "Error while binding to socket"
# define ERR_LISTENING "Error while listening to socket"

// utils.cpp
void	error(const std::string message, bool perror);

#endif
