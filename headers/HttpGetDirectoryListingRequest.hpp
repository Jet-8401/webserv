#ifndef HTTP_GET_DIRECTORY_LISTING_REQUEST_HPP
# define HTTP_GET_DIRECTORY_LISTING_REQUEST_HPP

# include "HttpRequest.hpp"
# include <dirent.h>
# include <sstream>

class HttpGetDirectoryListingRequest : public HttpRequest {
    private:
        DIR* _dir;
        std::stringstream _directory_content;
        bool _content_generated;

    public:
        HttpGetDirectoryListingRequest(const ServerConfig& config);
        virtual ~HttpGetDirectoryListingRequest();

        virtual bool parse(const uint8_t* packet, const size_t packet_size);
        virtual ssize_t writePacket(uint8_t* io_buffer, size_t buff_length);

    protected:
        bool _openDirectory();
        void _closeDirectory();
        bool _generateDirectoryListing();
};

#endif
