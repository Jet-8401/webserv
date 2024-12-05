#ifndef HTTP_GET_STATIC_FILE_REQUEST_HPP
# define HTTP_GET_STATIC_FILE_REQUEST_HPP

# include "HttpRequest.hpp"

class HttpGetStaticFileRequest : public HttpRequest {
    private:
        int _file_fd;
        std::string _file_path;

    public:
        HttpGetStaticFileRequest(const ServerConfig& config);
        virtual ~HttpGetStaticFileRequest();

        virtual bool parse(const uint8_t* packet, const size_t packet_size);
        virtual ssize_t writePacket(uint8_t* io_buffer, size_t buff_length);

    protected:
        bool _openFile();
        void _closeFile();
};

#endif
