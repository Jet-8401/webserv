#ifndef HTTP_GET_STATIC_FILE_HPP
# define HTTP_GET_STATIC_FILE_HPP

# include "AHttpMethod.hpp"
# include "HttpRequest.hpp"
# include <string>

class HttpGetStaticFile : public AHttpMethod {
    private:
        int			_file_fd;
        std::string	_file_path;

    public:
        HttpGetStaticFile(HttpRequest* request);
        virtual ~HttpGetStaticFile(void);

        virtual bool parse(const uint8_t* packet, const size_t packet_size);
        virtual ssize_t writePacket(uint8_t* io_buffer, size_t buff_length);

    protected:
        bool _openFile();
        void _closeFile();
};

#endif
