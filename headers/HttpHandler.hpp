#ifndef HTTP_HANDLER_HPP
# define HTTP_HANDLER_HPP

# include "HttpMessage.hpp"
# include "BytesBuffer.hpp"
# include "ServerConfig.hpp"
# include "StreamBuffer.hpp"
# include "Location.hpp"
# include <map>
# include <string>

class AHttpMethod;

class HttpHandler : public HttpMessage {

public:
    enum parsing_state_e {
        READING_HEADERS,
        CHECK_METHOD,
        READING_BODY,
        DONE,
        ERROR
    };

    enum writing_state_e {
        WAITING,
        HEADERS_SENT,
        SENDING_BODY,
        WDONE,
        WERROR
    };

    HttpHandler(const ServerConfig& config);
    ~HttpHandler(void);

    bool parse(const uint8_t* packet, const size_t packet_size);
    ssize_t writePacket(uint8_t* io_buffer, size_t buff_length);

    // Getters
    Location* getMatchingLocation(void) const;
    const std::string& getPath(void) const;
    const std::string& getLocationString(void) const;
    int getStatusCode(void) const;

    // Setters
    void setStatusCode(int code);
    void addHeader(const std::string& key, const std::string& value);
    void buildHeaders(std::stringstream& response) const;

    friend class AHttpMethod;
    static std::map<std::string, std::string>& mime_types;
private:
    static std::map<std::string, std::string>& init_mime_types(void);

    BytesBuffer         _header_buff;
    StreamBuffer        _body;
    parsing_state_e     _state;
    writing_state_e     _wstate;
    const ServerConfig& _config_reference;
    std::string         _config_location_str;
    Location*           _matching_location;
    AHttpMethod*        _extanded_method;
    size_t              _end_header_index;
    std::string         _method;
    std::string         _path;
    std::string         _version;
    int                 _status_code;

    bool _bufferHeaders(const uint8_t* packet, size_t packet_size);
    bool _checkHeaderSyntax(const std::string& key, const std::string& value) const;
    bool _parseHeaders(void);
    bool _findLocation(void);
    bool _validateAndInitMethod(void);
};

#endif
