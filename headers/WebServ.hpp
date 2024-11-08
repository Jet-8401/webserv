#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <string>
# include <iostream>
# include <iomanip>

# define PROG_NAME "webserv"
# define ERR_USAGE "usage: ./webserv <configFile>"
# define ERR_FILE_OPEN "Cannot open file: "
# define ERR_SOCKET_CREATION "Socket create failed"
# define ERR_BINDING_SOCKET "Error while binding to socket"
# define ERR_LISTENING "Error while listening to socket"
# define ERR_EPOLL_CREATION "Cannot create an epoll instance"
# define ERR_EPOLL_NOT_SET "epoll instance not set"
# define ERR_EPOLL_ADD "Cannot add a file descriptor to an epoll instance"

# ifdef DEBUGGER

#  define DEBUG(msg) do { \
    time_t	now = time(0); \
    tm*		ltm = localtime(&now); \
    std::cout << "[" << std::setfill('0') << std::setw(2) << ltm->tm_hour << ":" \
              << std::setfill('0') << std::setw(2) << ltm->tm_min << ":" \
              << std::setfill('0') << std::setw(2) << ltm->tm_sec << "] " \
              << msg << std::endl; \
} while(0)

# else

#  define DEBUG(msg)

# endif

// utils.cpp
void	error(const std::string message, bool perror);

// debugger.cpp
void	debug(const std::string debug_message);

#endif
