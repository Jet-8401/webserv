#include "../headers/HttpPostCGI.hpp"
#include "../headers/WebServ.hpp"
#include <cstdlib>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <string>

extern char** environ;

HttpPostCGI::HttpPostCGI(const HttpParser& parser):
    HttpPost(parser),
    _cgi_pid(-1),
    _child_proc_exited(false)
{
	DEBUG("Creating a HttpPostCGI object !");
	this->_response.setStatusCode(200);
    if (pipe(this->_pipe) == -1) {
        this->_state = this->_request.error(500);
        return;
    }
    makeNonBlocking(this->_pipe[0]);
    makeNonBlocking(this->_pipe[1]);
    this->executeCGI();

    std::string length = this->_request.getHeader("Content-Length");
    if (length.empty()) {
    	this->_state = this->_request.error(411);
		return;
    }

    if (this->_request.getBody().size() == (size_t) std::atoi(length.c_str())) {
		this->_request.setEvents(EPOLLOUT);
    } else {
    	this->_state = handler_state_t(READING_BODY, false);
    }
}

HttpPostCGI::~HttpPostCGI(void)
{
    if (this->_pipe[0] != -1)
        close(this->_pipe[0]);
    if (this->_pipe[1] != -1)
        close(this->_pipe[1]);

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
        // close(this->_pipe[0]);

        std::string extension(::strrchr(this->_request.getResolvedPath().c_str(), '.'));
        char* const args[] = {
            const_cast<char*>(this->_request.getMatchingLocation()->getCGIs().find(extension)->second.c_str()),
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

        dup2(this->_pipe[0], 0);
        dup2(this->_pipe[1], 1);

        execve(args[0], args, new_environ);

        // Clean up if execve fails
        for (size_t j = env_count; new_environ[j] != NULL; j++)
            delete[] new_environ[j];
        delete[] new_environ;
        exit(1);
    }

    // close(this->_pipe[1]);   // Parent closes write end of input
}

bool    HttpPostCGI::parse(const uint8_t* packet, const size_t packet_size)
{
	if (this->_state.flag != READING_BODY)
		return (this->HttpParser::parse(packet, packet_size));
    if (packet && packet_size > 0) {
        if (::write(this->_pipe[1], packet, packet_size) == -1) {
            this->_state = this->_request.error(500);
            return false;
        }
    }
    return (true);
}

ssize_t HttpPostCGI::write(uint8_t* io_buffer, const size_t buff_len)
{
	if (!this->_child_proc_exited && WIFEXITED(waitpid(this->_cgi_pid, NULL, WNOHANG))) {
		this->_child_proc_exited = true;
		this->_state = handler_state_t(READY_TO_SEND, true);
        DEBUG("CGIPost: child process exited !");
        return (this->HttpParser::write(io_buffer, buff_len));	// go to default write for building and sending headers
	}

	DEBUG("this->_state.flag = " << this->_state.flag);
	DEBUG("does child proc exited ? " << (this->_child_proc_exited ? "yes" : "no"));

	if (this->_child_proc_exited && this->_state.flag == SENDING_BODY) {
		ssize_t bytes_read = read(this->_pipe[0], io_buffer, buff_len);

		DEBUG("bytes read -> " << bytes_read);

		if (bytes_read == -1) {
			error("PIPE MON CUL", true);
			// this->_state = this->_request.error(500);
			this->_state = handler_state_t(DONE, true);
		} else {
			return (bytes_read);
		}
	}

    return (this->HttpParser::write(io_buffer, buff_len));
}
