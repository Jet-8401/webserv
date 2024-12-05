#include "../headers/HttpGetDirectoryListingRequest.hpp"
#include <sys/stat.h>
#include <string>

HttpGetDirectoryListingRequest::HttpGetDirectoryListingRequest(const ServerConfig& config)
    : HttpRequest(config), _dir(NULL), _content_generated(false)
{}

HttpGetDirectoryListingRequest::~HttpGetDirectoryListingRequest()
{
    _closeDirectory();
}

bool HttpGetDirectoryListingRequest::parse(const uint8_t* packet, const size_t packet_size)
{
    if (!HttpRequest::parse(packet, packet_size))
        return false;

    if (this->_state == CHECK_METHOD) {
        if (!_openDirectory())
            return false;
        if (!_generateDirectoryListing())
            return false;
        this->_state = DONE;
    }
    return true;
}

ssize_t HttpGetDirectoryListingRequest::writePacket(uint8_t* io_buffer, size_t buff_length)
{
    if (!_content_generated)
        return -1;

    std::string content = _directory_content.str();
    size_t remaining = content.length() - _directory_content.tellg();
    size_t to_copy = std::min(remaining, buff_length);

    if (to_copy == 0)
        return 0;

    memcpy(io_buffer, content.c_str() + _directory_content.tellg(), to_copy);
    _directory_content.seekg(to_copy, std::ios::cur);

    return to_copy;
}

bool HttpGetDirectoryListingRequest::_openDirectory()
{
    std::string dir_path = _matching_location->getRoot() + _path;
    _dir = opendir(dir_path.c_str());
    return _dir != NULL;
}

void HttpGetDirectoryListingRequest::_closeDirectory()
{
    if (_dir) {
        closedir(_dir);
        _dir = NULL;
    }
}

bool HttpGetDirectoryListingRequest::_generateDirectoryListing()
{
    if (!_dir)
        return false;

    _directory_content << "<html><head><title>Index of " << _path
                      << "</title></head><body><h1>Index of " << _path
                      << "</h1><hr><pre>";

    struct dirent* entry;
    while ((entry = readdir(_dir)) != NULL) {
        std::string name = entry->d_name;
        if (entry->d_type == DT_DIR)
            name += "/";
        _directory_content << "<a href=\"" << name << "\">" << name << "</a>\n";
    }

    _directory_content << "</pre><hr></body></html>";
    _content_generated = true;
    return true;
}
