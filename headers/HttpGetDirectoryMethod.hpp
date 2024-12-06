#ifndef HTTP_GET_DIRECTORY_METHOD_HPP
# define HTTP_GET_DIRECTORY_METHOD_HPP

# include "AHttpMethod.hpp"
# include <dirent.h>
# include <string>

class HttpGetDirectoryMethod : public AHttpMethod {
private:
    DIR*            _dir;
    std::string     _html_content;
    size_t          _bytes_sent;

    bool    _generateListing(void);
    bool    _setHeaders(void);

public:
    HttpGetDirectoryMethod(HttpHandler& handler);
    virtual ~HttpGetDirectoryMethod(void);

    virtual bool parse(const uint8_t* packet, const size_t packet_size);
    virtual ssize_t writePacket(uint8_t* io_buffer, size_t buff_length);
};

#endif
