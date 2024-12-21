#include "../headers/HttpPostCGI.hpp"
#include "../headers/WebServ.hpp"
#include <sys/epoll.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <fcntl.h>
#include <cstring>
#include <string>

extern char** environ;

HttpPostCGI::HttpPostCGI(const HttpParser& parser):
    HttpPost(parser),
    _cgi_pid(-1)
{
    if (pipe(this->_pipe_in) == -1 || pipe(this->_pipe_out) == -1) {
        this->_state = this->_request.error(500);
        return;
    }
    this->_state = handler_state_t(READING_BODY, true);
    this->_request.setEvents(EPOLLOUT);
    this->executeCGI();
}

HttpPostCGI::~HttpPostCGI(void)
{
    if (this->_pipe_in[0] != -1)
        close(this->_pipe_in[0]);
    if (this->_pipe_in[1] != -1)
        close(this->_pipe_in[1]);
    if (this->_pipe_out[0] != -1)
        close(this->_pipe_out[0]);
    if (this->_pipe_out[1] != -1)
        close(this->_pipe_out[1]);

    if (this->_cgi_pid != -1) {
        kill(this->_cgi_pid, SIGTERM);
        waitpid(this->_cgi_pid, NULL, 0);
    }
}

void    HttpPostCGI::executeCGI(void)
{
    this->_cgi_pid = fork();
    if (this->_cgi_pid == -1) {
        this->_state = this->_request.error(500);
        return;
    }

    if (this->_cgi_pid == 0) {  // Child process
        close(this->_pipe_in[1]);
        close(this->_pipe_out[0]);

        std::string extension(::strrchr(this->_request.getResolvedPath().c_str(), '.'));
        char* const args[] = {
            const_cast<char*>(this->_request.getMatchingLocation().getCGIs().find(extension)->second.c_str()),
            const_cast<char*>(this->_request.getResolvedPath().c_str()),
            NULL
        };

        // Create CGI environment variables
        std::string env_vars[] = {
            "GATEWAY_INTERFACE=CGI/1.1",
            "REQUEST_METHOD=POST",
            "CONTENT_TYPE=" + this->_request.getHeader("Content-Type"),
            "CONTENT_LENGTH=" + this->_request.getHeader("Content-Length"),
            ""
        };

        // Count existing environment variables
        size_t env_count = 0;
        while (environ[env_count] != NULL)
            env_count++;

        // Create new environment array
        char** new_environ = new char*[env_count + 4 + 1];

        // Copy existing environment
        size_t i = 0;
        while (environ[i] != NULL) {
            new_environ[i] = environ[i];
            i++;
        }

        // Add our CGI variables
        for (size_t j = 0; !env_vars[j].empty(); j++) {
            new_environ[i] = new char[env_vars[j].length() + 1];
            strcpy(new_environ[i], env_vars[j].c_str());
            i++;
        }
        new_environ[i] = NULL;

        dup2(this->_pipe_in[0], STDIN_FILENO);
        dup2(this->_pipe_out[1], STDOUT_FILENO);

        execve(args[0], args, new_environ);

        // Clean up if execve fails
        for (size_t j = env_count; new_environ[j] != NULL; j++)
            delete[] new_environ[j];
        delete[] new_environ;
        exit(1);
    }

    close(this->_pipe_in[0]);   // Parent closes read end of input
    close(this->_pipe_out[1]);  // Parent closes write end of output
}

bool    HttpPostCGI::parse(const uint8_t* packet, const size_t packet_size)
{
	if (this->_state.flag != READING_BODY)
		return (this->HttpParser::parse(packet, packet_size));
    if (packet && packet_size > 0) {
        if (::write(this->_pipe_in[1], packet, packet_size) == -1) {
            this->_state = this->_request.error(500);
            return false;
        }
    }
    return (true);
}

ssize_t HttpPostCGI::write(const uint8_t* io_buffer, const size_t buff_len)
{
    if (WIFEXITED(waitpid(this->_cgi_pid, NULL, WNOHANG)))
        this->_state = handler_state_t(DONE, true);
    else
    	return (0);
    return (this->HttpParser::write(io_buffer, buff_len));


}
