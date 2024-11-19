#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

# include "BytesBuffer.hpp"
# include "HttpRequest.hpp"
# include "ServerConfig.hpp"
# include <string>
# include <map>

class HttpResponse {
    private:
        std::multimap<std::string, std::string>    _headers;
        BytesBuffer                                _content;
        bool                                       _is_sent;
        int                                        _status_code;
        std::string                                _status_message;

        // Helper methods
        void    _buildHeaders(std::stringstream& response) const;
        bool    _sendAll(const int socket_fd, const std::string& data) const;
        void    _setContentType(const std::string& path);
        bool    _loadErrorPage(const std::string& error_code, const ServerConfig& config);
        void    _setDefaultErrorPage(const std::string& error_code);
        std::string _resolvePath(const std::string& uri, const ServerConfig& config) const;

    public:
        HttpResponse(void);
        virtual ~HttpResponse(void);

        // Core response operations
        void    setHeader(const std::string& key, const std::string& value);
        void    clearHeaders(void);
        int     send(const int socket_fd);

        // Operations using Request and Config
        int     handleRequest(const HttpRequest& request, const ServerConfig& config);
        int     handleError(const HttpRequest& request, const ServerConfig& config);
        int     serveFile(const std::string& uri, const ServerConfig& config);
        int     saveFile(const HttpRequest& request, const ServerConfig& config);
        int     handleCGI(const HttpRequest& request, const ServerConfig& config);
        int     handleDirectory(const std::string& uri, const ServerConfig& config);

        // Getters
        bool    isSent(void) const;
        int     getStatusCode(void) const;
};

#endif
