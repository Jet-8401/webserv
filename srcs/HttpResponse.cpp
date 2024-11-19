#include "../headers/HttpResponse.hpp"
#include "../headers/WebServ.hpp"
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

// Constructor / Destructor
HttpResponse::HttpResponse(void):
    _headers(),
    _content(),
    _is_sent(false),
    _status_code(200),
    _status_message("OK")
{
    this->setHeader("Server", "Webserv/1.0");
}

HttpResponse::~HttpResponse(void)
{}

// Private Helper Methods
void HttpResponse::_buildHeaders(std::stringstream& response) const
{
    response << "HTTP/1.1 " << _status_code << " " << _status_message << "\r\n";

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

void HttpResponse::_setContentType(const std::string& path)
{
    size_t dot_pos = path.find_last_of(".");
    if (dot_pos == std::string::npos) {
        this->setHeader("Content-Type", "application/octet-stream");
        return;
    }

    std::string ext = path.substr(dot_pos);

    // Text files
    if (ext == ".html" || ext == ".htm")
        this->setHeader("Content-Type", "text/html");
    else if (ext == ".css")
        this->setHeader("Content-Type", "text/css");
    else if (ext == ".js")
        this->setHeader("Content-Type", "text/javascript");
    else if (ext == ".txt")
        this->setHeader("Content-Type", "text/plain");
    else if (ext == ".csv")
        this->setHeader("Content-Type", "text/csv");

    // Images
    else if (ext == ".jpg" || ext == ".jpeg")
        this->setHeader("Content-Type", "image/jpeg");
    else if (ext == ".png")
        this->setHeader("Content-Type", "image/png");
    else if (ext == ".gif")
        this->setHeader("Content-Type", "image/gif");
    else if (ext == ".svg")
        this->setHeader("Content-Type", "image/svg+xml");
    else if (ext == ".ico")
        this->setHeader("Content-Type", "image/x-icon");

    // Audio/Video
    else if (ext == ".mp3")
        this->setHeader("Content-Type", "audio/mpeg");
    else if (ext == ".wav")
        this->setHeader("Content-Type", "audio/wav");
    else if (ext == ".mp4")
        this->setHeader("Content-Type", "video/mp4");
    else if (ext == ".webm")
        this->setHeader("Content-Type", "video/webm");

    // Documents
    else if (ext == ".pdf")
        this->setHeader("Content-Type", "application/pdf");
    else if (ext == ".xml")
        this->setHeader("Content-Type", "application/xml");
    else if (ext == ".json")
        this->setHeader("Content-Type", "application/json");

    // Archives
    else if (ext == ".zip")
        this->setHeader("Content-Type", "application/zip");
    else if (ext == ".gz")
        this->setHeader("Content-Type", "application/gzip");
    else if (ext == ".tar")
        this->setHeader("Content-Type", "application/x-tar");

    // Default binary
    else
        this->setHeader("Content-Type", "application/octet-stream");
}

bool HttpResponse::_loadErrorPage(const std::string& error_code, const ServerConfig& config)
{
    const std::map<std::string, Location*>& locations = config.getLocations();

    for (std::map<std::string, Location*>::const_iterator it = locations.begin();
         it != locations.end(); ++it) {
        const std::map<std::string, std::string*>& error_pages = it->second->getErrorPages();
        std::map<std::string, std::string*>::const_iterator page = error_pages.find(error_code);
        if (page != error_pages.end() && page->second) {
            std::string error_page_path = *(page->second);
            std::ifstream file(error_page_path.c_str());
            if (file.is_open()) {
                char buffer[4096];
                file.seekg(0, std::ios::end);
                std::streamsize file_size = file.tellg();
                file.seekg(0, std::ios::beg);

                // Get max body size from location if set
                long max_size = it->second->getClientMaxBodySize();
                if (max_size > 0 && file_size > max_size) {
                    file.close();
                    return false;
                }

                while (file.read(buffer, sizeof(buffer))) {
                    if (_content.write(reinterpret_cast<const uint8_t*>(buffer), file.gcount()) == -1) {
                        file.close();
                        return false;
                    }
                }

                // Handle last chunk if any
                if (file.gcount() > 0) {
                    if (_content.write(reinterpret_cast<const uint8_t*>(buffer), file.gcount()) == -1) {
                        file.close();
                        return false;
                    }
                }

                file.close();

                // Set Content-Length header
                this->setHeader("Content-Length", unsafe_itoa(_content.size()));

                // Set appropriate Content-Type
                this->_setContentType(error_page_path);

                return true;
            }
        }
    }
    return false;
}

void HttpResponse::_setDefaultErrorPage(const std::string& error_code)
{
    std::stringstream body;
    body << "<html><head><title>Error " << error_code << "</title></head>"
         << "<body><h1>Error " << error_code << "</h1>"
         << "<p>Error occurred while processing your request.</p></body></html>";

    this->setHeader("Content-Type", "text/html");
    // Set body content in BytesBuffer
    // ... implementation
}

std::string HttpResponse::_resolvePath(const std::string& uri, const ServerConfig& config) const
{
    const std::map<std::string, Location*>& locations = config.getLocations();
    std::string longest_match = "";
    Location* matching_location = NULL;

    // Find matching location block
    for (std::map<std::string, Location*>::const_iterator it = locations.begin();
         it != locations.end(); ++it) {
        if (uri.find(it->first) == 0 && it->first.length() > longest_match.length()) {
            longest_match = it->first;
            matching_location = it->second;
        }
    }

    if (!matching_location)
        return "";

    // Apply root/alias rules
    std::string path;
    if (!matching_location->getAlias().empty())
        path = matching_location->getAlias() + uri.substr(longest_match.length());
    else
        path = matching_location->getRoot() + uri.substr(longest_match.length());

    return path;
}

// Public Methods
void HttpResponse::setHeader(const std::string& key, const std::string& value)
{
    this->_headers.insert(std::make_pair(key, value));
}

void HttpResponse::clearHeaders(void)
{
    this->_headers.clear();
    this->setHeader("Server", "Webserv/1.0");
}

int HttpResponse::handleRequest(const HttpRequest& request, const ServerConfig& config)
{
    if (request.haveFailed())
        return this->handleError(request, config);

    const std::string& method = request.getMethod();
    const std::string& uri = request.getLocation();

    // Check for redirections in matching location
    // Handle different methods
    if (method == "GET")
        return this->serveFile(uri, config);
    else if (method == "POST")
        return this->saveFile(request, config);
    // Add other methods...

    return 0;
}

int HttpResponse::handleError(const HttpRequest& request, const ServerConfig& config)
{
    const std::string& error_code = request.getErrorCode();

    _status_code = std::atoi(error_code.c_str());
    _status_message = "Error"; // Set appropriate message

    if (!_loadErrorPage(error_code, config))
        _setDefaultErrorPage(error_code);

    return 0;
}

int HttpResponse::serveFile(const std::string& uri, const ServerConfig& config)
{
    std::string path = _resolvePath(uri, config);
    if (path.empty())
        return -1;

    // Handle directory listing if needed
    struct stat st;
    if (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
        return handleDirectory(uri, config);

    // Read and serve file
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open())
        return -1;

    // Read file into _content
    // Set appropriate headers
    return 0;
}

int HttpResponse::send(const int socket_fd)
{
    if (this->_is_sent)
        return 0;

    std::stringstream headers;
    this->_buildHeaders(headers);

    if (!this->_sendAll(socket_fd, headers.str()))
        return -1;

    // Send content if any exists
    if (this->_content.size() > 0) {
        const uint8_t* content = this->_content.read();
        if (write(socket_fd, content, this->_content.size()) == -1)
            return -1;
    }

    this->_is_sent = true;
    return 0;
}

// Getters
bool HttpResponse::isSent(void) const
{
    return this->_is_sent;
}

int HttpResponse::getStatusCode(void) const
{
    return this->_status_code;
}
