#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

#include "BytesBuffer.hpp"
#include <string>
#include <map>

class HttpResponse {
    private:
        std::multimap<std::string, std::string>    _headers;
        BytesBuffer                                _content;
        bool                                       _is_sent;

        void    _buildHeaders(std::stringstream& response) const;
        bool    _sendAll(const int socket_fd, const std::string& data) const;

    public:
        HttpResponse(void);
        virtual ~HttpResponse(void);

        void    setHeader(const std::string& key, const std::string& value);
        int     send(const int socket_fd);
        bool    isSent(void) const;
};

#endif
