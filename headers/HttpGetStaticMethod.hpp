#ifndef HTTP_GET_STATIC_METHOD_HPP
# define HTTP_GET_STATIC_METHOD_HPP

# include "AHttpMethod.hpp"

class HttpGetStaticMethod : public AHttpMethod {
private:
    int     _file_fd;
    size_t  _bytes_sent;

    bool    _openFile(void);
    bool    _setHeaders(void);

public:
    HttpGetStaticMethod(HttpHandler& handler);
    virtual ~HttpGetStaticMethod(void);

    virtual bool parse(const uint8_t* packet, const size_t packet_size);
    virtual ssize_t writePacket(uint8_t* io_buffer, size_t buff_length);
};

#endif
