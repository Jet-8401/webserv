#ifndef HTTP_POST_REQUEST_HPP
# define HTTP_POST_REQUEST_HPP

# include "HttpRequest.hpp"
# include <string>
# include <sstream>

class HttpPostRequest : public HttpRequest {
    private:
        enum ContentType {
            MULTIPART_FORM,
            URL_ENCODED,
            UNKNOWN
        };

        ContentType _content_type;
        std::string _boundary;
        std::string _upload_path;
        int _file_fd;
        size_t _content_length;
        size_t _bytes_received;
        bool _headers_parsed;
        bool _headers_sent;
        std::stringstream _response_headers;

    public:
        HttpPostRequest(const ServerConfig& config);
        virtual ~HttpPostRequest();

        virtual bool parse(const uint8_t* packet, const size_t packet_size);
        virtual ssize_t writePacket(uint8_t* io_buffer, size_t buff_length);

    protected:
        bool _parseContentType();
        bool _handleMultipartForm(const uint8_t* data, size_t size);
        bool _handleUrlEncoded(const uint8_t* data, size_t size);
        bool _createUploadFile();
        void _closeUploadFile();
        void _prepareResponseHeaders();
        ssize_t _sendHeaders(uint8_t* io_buffer, size_t buff_length);
};

#endif
