#ifndef HTTP_GET_STATIC_FILE_HPP
# define HTTP_GET_STATIC_FILE_HPP

# include "HttpParser.hpp"
# include <string>

class HttpGetStaticFile : public HttpParser {
    private:
        int			_file_fd;
        std::string	_file_path;

    public:
        HttpGetStaticFile(const HttpParser& parser);
        virtual ~HttpGetStaticFile(void);

        bool	parse(const uint8_t* packet, const size_t packet_size);
        ssize_t write(const uint8_t* io_buffer, const size_t buff_length);
};

#endif
