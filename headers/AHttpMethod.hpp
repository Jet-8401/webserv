#ifndef A_HTTP_METHOD_HPP
# define A_HTTP_METHOD_HPP

# include <stdint.h>
#include <sys/types.h>

class HttpRequest;

class AHttpMethod {
    public:
        AHttpMethod(HttpRequest& referer);
        virtual ~AHttpMethod(void);

        virtual bool parse(const uint8_t* packet, const size_t packet_size) = 0;
        virtual ssize_t writePacket(uint8_t* io_buffer, size_t buff_length);

    protected:
        HttpRequest& referer;
};

#endif
