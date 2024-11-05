#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <string>

# define PROG_NAME "webserv"
# define ERR_USAGE "usage: ./webserv <configFile>"
# define ERR_FILE_OPEN "Cannot open file: "

// utils.cpp
void	error(const std::string message, bool perror);

#endif
