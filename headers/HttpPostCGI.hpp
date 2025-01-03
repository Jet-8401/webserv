#ifndef HTTP_POST_CGI_HPP
# define HTTP_POST_CGI_HPP

# include "HttpPost.hpp"

class HttpPostCGI : public HttpPost {
    private:
        pid_t   _cgi_pid;
        int     _pipe[2];
        bool	_child_proc_exited;

        void    executeCGI(void);

    public:
        HttpPostCGI(const HttpParser& parser);
        virtual ~HttpPostCGI();

        virtual bool    parse(const uint8_t* packet, const size_t packet_size);
        virtual ssize_t write(uint8_t* io_buffer, const size_t buff_len);
};

#endif
