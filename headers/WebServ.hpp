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
# define ERR_EPOLL_WAIT "An error occured while epoll was waiting"
# define ERR_ACCEPT_REQUEST "Could not accept the client request"
# define ERR_READING_REQUEST "Impossible to read the request"
# define ERR_EPOLL_MOD "Impossible to change epoll instance"
# define ERR_EPOLL_DEL "Impossible to delete an epoll event"
# define ERR_FD_CLOSE "Cannot close a file descriptor"
# define ERR_TMPFILE_CREATION "Cannot create a temp file"
# define ERR_WRITING_TMPFILE "Cannot write to temp file"
# define ERR_ADDR_VALUE "Error not a valid address"

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
void		error(const std::string message, bool perror);
std::string	unsafe_itoa(const int n);

#endif
