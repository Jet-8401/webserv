#ifndef HTTP_DELETE_REQUEST_HPP
# define HTTP_DELETE_REQUEST_HPP

# include "HttpRequest.hpp"

class HttpDeleteRequest : public HttpRequest {
    private:
        std::string _target_path;
        bool _is_directory;

    public:
        HttpDeleteRequest(const ServerConfig& config);
        virtual ~HttpDeleteRequest();

        virtual bool parse(const uint8_t* packet, const size_t packet_size);
        virtual ssize_t writePacket(uint8_t* io_buffer, size_t buff_length);

    protected:
        bool _checkPath();
        bool _deleteFile();
        bool _deleteDirectory();
};

#endif
