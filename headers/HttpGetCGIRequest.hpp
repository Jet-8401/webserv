#ifndef HTTP_GET_CGI_REQUEST_HPP
# define HTTP_GET_CGI_REQUEST_HPP

# include "HttpRequest.hpp"
# include <map>

class HttpGetCGIRequest : public HttpRequest {
    private:
        int _cgi_pipe[2];
        pid_t _cgi_pid;
        std::map<std::string, std::string> _env_vars;
        StreamBuffer _cgi_output;
        bool _cgi_running;

    public:
        HttpGetCGIRequest(const ServerConfig& config);
        virtual ~HttpGetCGIRequest();

        virtual bool parse(const uint8_t* packet, const size_t packet_size);
        virtual ssize_t writePacket(uint8_t* io_buffer, size_t buff_length);

    protected:
        bool _setupCGI();
        bool _executeCGI();
        void _setupEnvironment();
        void _cleanupCGI();
};

#endif
