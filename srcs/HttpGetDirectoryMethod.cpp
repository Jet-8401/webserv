#include "../headers/HttpGetDirectoryMethod.hpp"
#include "../headers/HttpHandler.hpp"
#include "../headers/WebServ.hpp"
#include <sstream>
#include <cstring>

HttpGetDirectoryMethod::HttpGetDirectoryMethod(HttpHandler& handler) :
    AHttpMethod(handler),
    _dir(NULL),
    _bytes_sent(0)
{}

HttpGetDirectoryMethod::~HttpGetDirectoryMethod(void)
{
    if (this->_dir)
        ::closedir(this->_dir);
}

bool HttpGetStaticMethod::parse(const uint8_t* packet, const size_t packet_size)
{
    (void)packet;
    (void)packet_size;

    return this->_resolveLocation()
        && S_ISREG(this->_file_stat.st_mode)
        && this->_openFile()
        && this->_setHeaders();
}

bool HttpGetDirectoryMethod::_generateListing(void)
{
    std::stringstream html;
    struct dirent* entry;
    struct stat entry_stat;

    html << "<!DOCTYPE html>\r\n"
         << "<html>\r\n<head>\r\n"
         << "<title>Index of " << this->referer.getPath() << "</title>\r\n"
         << "<style>\r\n"
         << "body { font-family: monospace; }\r\n"
         << "td { padding: 0 10px; }\r\n"
         << "</style>\r\n</head>\r\n<body>\r\n"
         << "<h1>Index of " << this->referer.getPath() << "</h1><hr>\r\n"
         << "<table>\r\n"
         << "<tr><th>Name</th><th>Size</th></tr>\r\n";

    ::rewinddir(this->_dir);
    while ((entry = ::readdir(this->_dir))) {
        if (std::string(entry->d_name) == ".")
            continue;

        std::string entry_path = joinPath(this->_complete_path, entry->d_name);
        if (::stat(entry_path.c_str(), &entry_stat) == -1)
            continue;

        html << "<tr><td><a href=\"" << entry->d_name;
        if (S_ISDIR(entry_stat.st_mode))
            html << "/";
        html << "\">" << entry->d_name;
        if (S_ISDIR(entry_stat.st_mode))
            html << "/";
        html << "</a></td>";

        if (S_ISDIR(entry_stat.st_mode))
            html << "<td>-</td>";
        else
            html << "<td>" << entry_stat.st_size << "B</td>";
        html << "</tr>\r\n";
    }

    html << "</table>\r\n<hr>\r\n</body>\r\n</html>\r\n";
    this->_html_content = html.str();
    return true;
}

bool HttpGetDirectoryMethod::_setHeaders(void)
{
    std::stringstream size_str;
    size_str << this->_html_content.length();

    this->referer.addHeader("Content-Type", "text/html");
    this->referer.addHeader("Content-Length", size_str.str());
    this->referer.setStatusCode(200);

    return true;
}

ssize_t HttpGetDirectoryMethod::writePacket(uint8_t* io_buffer, size_t buff_length)
{
    size_t remaining = this->_html_content.length() - this->_bytes_sent;
    size_t to_send = remaining < buff_length ? remaining : buff_length;

    if (to_send == 0)
        return 0;

    ::memcpy(io_buffer, this->_html_content.c_str() + this->_bytes_sent, to_send);
    this->_bytes_sent += to_send;

    return to_send;
}
