#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include "HttpMessage.hpp"
# include "BytesBuffer.hpp"
# include "ServerConfig.hpp"
# include "StreamBuffer.hpp"
# include "Location.hpp"
# include <string>
# include <sstream>  // Add this include for stringstream

class HttpRequest : public HttpMessage {
    public:
        typedef enum parsing_state_e {
            READING_HEADERS,
            CHECK_METHOD,
            READING_BODY,
            DONE,
            ERROR
        } parsing_state_t;

        typedef enum response_state_e {
            WAITING,        // Waiting for request to complete
            SENDING_HEADER, // Sending response headers
            SENDING_BODY,   // Sending response body
            RESPONSE_DONE   // Response completed
        } response_state_t;

        HttpRequest(const ServerConfig& config);
        virtual ~HttpRequest(void);

        static std::string getStatusMessage(int status_code);
        virtual bool parse(const uint8_t* packet, const size_t packet_size);
        virtual ssize_t writePacket(uint8_t* io_buffer, size_t buff_length);

        const parsing_state_t& getState(void) const;

    protected:
        std::string _method;
        std::string _path;
        std::string _version;

        BytesBuffer _header_buff;
        StreamBuffer _body;
        parsing_state_e _state;
        response_state_t _response_state;

        const ServerConfig& _config_reference;
        std::string _config_location_str;
        Location* _matching_location;

        HttpRequest* _extanded_method;

        // Response related members
        std::stringstream _response_headers;
        bool _headers_sent;

        // Helper methods
        virtual void _prepareResponseHeaders();
        virtual ssize_t _sendHeaders(uint8_t* io_buffer, size_t buff_length);
        virtual ssize_t _sendBody(uint8_t* io_buffer, size_t buff_length);

    private:
        size_t _end_header_index;
        static std::map<int, std::string> _status_messages;

        bool _bufferHeaders(const uint8_t* packet, size_t packet_size);
        bool _checkHeaderSyntax(const std::string& key, const std::string& value) const;
        bool _parseHeaders(void);
        bool _findLocation(void);
        bool _validateAndInitMethod(void);
};

#endif
