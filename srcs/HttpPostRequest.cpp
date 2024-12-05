#include "../headers/HttpPostRequest.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sstream>

HttpPostRequest::HttpPostRequest(const ServerConfig& config)
    : HttpRequest(config),
      _content_type(UNKNOWN),
      _file_fd(-1),
      _content_length(0),
      _bytes_received(0),
      _headers_parsed(false),
      _headers_sent(false)
{}

HttpPostRequest::~HttpPostRequest()
{
    _closeUploadFile();
}

bool HttpPostRequest::parse(const uint8_t* packet, const size_t packet_size)
{
    if (!HttpRequest::parse(packet, packet_size))
        return false;

    if (!_headers_parsed && this->_state == CHECK_METHOD) {
        if (!_parseContentType())
            return false;

        // Changed this part
        headers_t::const_iterator it = _headers.find("Content-Length");
        if (it == _headers.end())
            return false;

        std::string content_length_str = it->second;
        _content_length = std::atol(content_length_str.c_str());
        if (_content_length > _matching_location->getClientMaxBodySize())
            return false;

        _headers_parsed = true;
        this->_state = READING_BODY;
    }

    if (this->_state == READING_BODY) {
        switch (_content_type) {
            case MULTIPART_FORM:
                if (!_handleMultipartForm(packet, packet_size))
                    return false;
                break;
            case URL_ENCODED:
                if (!_handleUrlEncoded(packet, packet_size))
                    return false;
                break;
            default:
                return false;
        }

        if (_bytes_received >= _content_length) {
            this->_state = DONE;
            _prepareResponseHeaders();
        }
    }

    return true;
}

bool HttpPostRequest::_parseContentType()
{
    // Changed this part too
    headers_t::const_iterator it = _headers.find("Content-Type");
    if (it == _headers.end())
        return false;

    std::string content_type = it->second;
    if (content_type.find("multipart/form-data") != std::string::npos) {
        _content_type = MULTIPART_FORM;
        size_t boundary_pos = content_type.find("boundary=");
        if (boundary_pos != std::string::npos)
            _boundary = content_type.substr(boundary_pos + 9);
    }
    else if (content_type.find("application/x-www-form-urlencoded") != std::string::npos)
        _content_type = URL_ENCODED;
    else
        return false;

    return true;
}

bool HttpPostRequest::_handleMultipartForm(const uint8_t* data, size_t size)
{
    if (_file_fd == -1 && !_createUploadFile())
        return false;

    // Write data to file
    if (write(_file_fd, data, size) == -1)
        return false;

    _bytes_received += size;
    return true;
}

bool HttpPostRequest::_handleUrlEncoded(const uint8_t* data, size_t size)
{
    if (_file_fd == -1 && !_createUploadFile())
        return false;

    // Write data to file
    if (write(_file_fd, data, size) == -1)
        return false;

    _bytes_received += size;
    return true;
}

bool HttpPostRequest::_createUploadFile()
{
    _upload_path = _matching_location->getRoot() + "/uploads/";
    // Create unique filename
    std::string filename = _upload_path + "upload_" + std::to_string(time(NULL));
    _file_fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    return _file_fd != -1;
}

void HttpPostRequest::_closeUploadFile()
{
    if (_file_fd != -1) {
        close(_file_fd);
        _file_fd = -1;
    }
}

void HttpPostRequest::_prepareResponseHeaders()
{
    _response_headers.str("");
    _response_headers << "HTTP/1.1 201 Created\r\n";
    _response_headers << "Content-Type: text/plain\r\n";
    _response_headers << "Content-Length: 13\r\n";
    _response_headers << "\r\n";
    _response_headers << "File uploaded\n";
}

ssize_t HttpPostRequest::_sendHeaders(uint8_t* io_buffer, size_t buff_length)
{
    std::string headers = _response_headers.str();
    size_t to_copy = std::min(headers.length(), buff_length);

    memcpy(io_buffer, headers.c_str(), to_copy);
    _headers_sent = true;

    return to_copy;
}

ssize_t HttpPostRequest::writePacket(uint8_t* io_buffer, size_t buff_length)
{
    if (!_headers_sent)
        return _sendHeaders(io_buffer, buff_length);

    return 0; // All data sent
}
