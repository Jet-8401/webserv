#include "../headers/HttpResponse.hpp"
#include "../headers/WebServ.hpp"
#include <unistd.h>
#include <sstream>

HttpResponse::HttpResponse(void):
    _headers(),
    _content(),
    _is_sent(false)
{
    this->setHeader("Server", "Webserv/1.0");
    this->setHeader("Connection", "close");
}

HttpResponse::~HttpResponse(void)
{}

void HttpResponse::setHeader(const std::string& key, const std::string& value)
{
    this->_headers.insert(std::make_pair(key, value));
}

void HttpResponse::_buildHeaders(std::stringstream& response) const
{
    response << "HTTP/1.1 200 OK\r\n";
    for (std::multimap<std::string, std::string>::const_iterator it = _headers.begin();
         it != _headers.end(); ++it)
    {
        response << it->first << ": " << it->second << "\r\n";
    }
    response << "\r\n";
}

bool HttpResponse::_sendAll(const int socket_fd, const std::string& data) const
{
    size_t total_sent = 0;
    ssize_t sent;

    while (total_sent < data.length())
    {
        sent = write(socket_fd, data.c_str() + total_sent, data.length() - total_sent);
        if (sent == -1)
            return false;
        total_sent += sent;
    }
    return true;
}

int HttpResponse::send(const int socket_fd)
{
    DEBUG("Starting send()");
    if (this->_is_sent)
    {
        DEBUG("Response already sent");
        return 0;
    }

    std::stringstream headers;
    this->_buildHeaders(headers);
    std::string headers_str = headers.str();
    DEBUG("Headers to send:\n" << headers_str);

    if (!this->_sendAll(socket_fd, headers_str))
    {
        DEBUG("Failed to send headers");
        return -1;
    }
    DEBUG("Headers sent successfully");

    if (this->_content.size() > 0)
    {
        DEBUG("Sending content of size: " << this->_content.size());
        const uint8_t* content = this->_content.read();
        if (write(socket_fd, content, this->_content.size()) == -1)
        {
            DEBUG("Failed to send content");
            return -1;
        }
        DEBUG("Content sent successfully");
    }
    else
    {
        DEBUG("No content to send");
    }

    this->_is_sent = true;
    DEBUG("Send completed");
    return 0;
}

bool HttpResponse::isSent(void) const
{
    return this->_is_sent;
}
