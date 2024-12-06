#ifndef AHTTPMETHOD_HPP
# define AHTTPMETHOD_HPP

# include <string>
# include <sys/stat.h>
# include <stdint.h>
# include <sys/types.h>

class HttpHandler;

class AHttpMethod {
protected:
    HttpHandler& referer;
    std::string _complete_path;
    struct stat _file_stat;
    bool _headers_sent;

    // Fonctions communes
    virtual bool _resolveLocation(void);
    virtual bool _setHeaders(void) = 0;

public:
    AHttpMethod(HttpHandler& handler);
    virtual ~AHttpMethod(void);

    // Interface pure virtuelle
    virtual bool parse(const uint8_t* packet, const size_t packet_size) = 0;
    virtual ssize_t writePacket(uint8_t* io_buffer, size_t buff_length) = 0;
};

#endif
