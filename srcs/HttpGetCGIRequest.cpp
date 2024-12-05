#include "../headers/HttpGetCGIRequest.hpp"
#include <unistd.h>
#include <sys/wait.h>

HttpGetCGIRequest::HttpGetCGIRequest(const ServerConfig& config)
    : HttpRequest(config), _cgi_pid(-1), _cgi_output(4096), _cgi_running(false)
{
    _cgi_pipe[0] = -1;
    _cgi_pipe[1] = -1;
}

HttpGetCGIRequest::~HttpGetCGIRequest()
{
    _cleanupCGI();
}

bool HttpGetCGIRequest::parse(const uint8_t* packet, const size_t packet_size)
{
    if (!HttpRequest::parse(packet, packet_size))
        return false;

    if (this->_state == CHECK_METHOD) {
        if (!_setupCGI())
            return false;
        if (!_executeCGI())
            return false;
        this->_state = DONE;
    }
    return true;
}

ssize_t HttpGetCGIRequest::writePacket(uint8_t* io_buffer, size_t buff_length)
{
    if (_cgi_running) {
        int status;
        pid_t result = waitpid(_cgi_pid, &status, WNOHANG);
        if (result == 0)
            return 0; // CGI still running
        _cgi_running = false;
    }

    return _cgi_output.consume(io_buffer, buff_length);
}

bool HttpGetCGIRequest::_setupCGI()
{
    if (pipe(_cgi_pipe) == -1)
        return false;

    _setupEnvironment();
    return true;
}

void HttpGetCGIRequest::_setupEnvironment()
{
    _env_vars["SCRIPT_FILENAME"] = _matching_location->getRoot() + _path;
    _env_vars["REQUEST_METHOD"] = "GET";
    _env_vars["QUERY_STRING"] = ""; // Parse from URL if needed
    _env_vars["SERVER_PROTOCOL"] = "HTTP/1.1";
    // Add other environment variables as needed
}

bool HttpGetCGIRequest::_executeCGI()
{
    _cgi_pid = fork();
    if (_cgi_pid == -1)
        return false;

    if (_cgi_pid == 0) {
        // Child process
        dup2(_cgi_pipe[1], STDOUT_FILENO);
        close(_cgi_pipe[0]);
        close(_cgi_pipe[1]);

        // Convert environment variables to proper format
        char** env = new char*[_env_vars.size() + 1];
        int i = 0;
        for (std::map<std::string, std::string>::iterator it = _env_vars.begin();
             it != _env_vars.end(); ++it) {
            std::string env_str = it->first + "=" + it->second;
            env[i++] = strdup(env_str.c_str());
        }
        env[i] = NULL;

        // Execute CGI script
        execle(_env_vars["SCRIPT_FILENAME"].c_str(),
               _env_vars["SCRIPT_FILENAME"].c_str(),
               NULL, env);
        exit(1);
    }

    // Parent process
    close(_cgi_pipe[1]);
    _cgi_pipe[1] = -1;
    _cgi_running = true;
    return true;
}

void HttpGetCGIRequest::_cleanupCGI()
{
    if (_cgi_pipe[0] != -1) {
        close(_cgi_pipe[0]);
        _cgi_pipe[0] = -1;
    }
    if (_cgi_pipe[1] != -1) {
        close(_cgi_pipe[1]);
        _cgi_pipe[1] = -1;
    }
    if (_cgi_pid != -1) {
        kill(_cgi_pid, SIGTERM);
        waitpid(_cgi_pid, NULL, 0);
        _cgi_pid = -1;
    }
}
