#ifndef HTTP_POST_METHOD_HPP
# define HTTP_POST_METHOD_HPP

# include "AHttpMethod.hpp"
# include "HttpRequest.hpp"
# include <string>

class HttpPostMethod : public AHttpMethod {
    public:
        HttpPostMethod(HttpRequest& referer);
        virtual ~HttpPostMethod(void);

        virtual bool parse(const uint8_t* packet, const size_t packet_size);
        virtual ssize_t writePacket(uint8_t* io_buffer, size_t buff_length);

    protected:
        int _fild_fd;
        std::string _multipart_key;
};

#endif
