#include "../headers/HttpGetDirectory.hpp"
#include "../headers/WebServ.hpp"
#include <sys/epoll.h>
#include <sys/stat.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <ctime>

HttpGetDirectory::HttpGetDirectory(const HttpParser& parser):
    HttpParser(parser),
    _headers_sent(false),
    _dir(NULL)
{
    this->_request.setEvents(EPOLLOUT);
    this->_dir = opendir(this->_request.getResolvedPath().c_str());
    if (this->_dir == NULL) {
        std::cerr << "could not open directory" << std::endl;
        this->_state = this->_request.error(404);
    }
	this->_request.setEvents(EPOLLOUT);
	this->_state = handler_state_t(READY_TO_SEND, false);
}

HttpGetDirectory::~HttpGetDirectory()
{
	if (_dir != NULL) {
		closedir(_dir);
		_dir = NULL;
	}
}

bool HttpGetDirectory::parse(const uint8_t* packet, const size_t packet_size)
{
	return (this->HttpParser::parse(packet, packet_size));
}

std::string formatSize(off_t size)
{
    std::stringstream ss;
    if (size < 1024)
        ss << size << "B";
    else if (size < 1024 * 1024)
        ss << (size / 1024) << "KB";
    else if (size < 1024 * 1024 * 1024)
        ss << (size / (1024 * 1024)) << "MB";
    else
    	ss << (size / (1024 * 1024 * 1024)) << "GB";
    return ss.str();
}

ssize_t HttpGetDirectory::write(const uint8_t* io_buffer, const size_t buff_length)
{
	static std::string		current_entry;
	static struct dirent* 	entry;

    if (this->_state.flag != SENDING_BODY)
        return (this->HttpParser::write(io_buffer, buff_length));

    if (_dir == NULL) {
        this->_state = handler_state_t(ERROR, true);
        return (-1);
    }

    if (!_headers_sent) {
        std::stringstream html;
        html << "<!DOCTYPE html>\r\n<html>\r\n<head>\r\n"
             << "<title>Index of " << this->_request.getPath() << "</title>\r\n"
             << "<style>\r\n"
             << "body { font-family: monospace; }\r\n"
             << "table { width: 100%; border-collapse: collapse; }\r\n"
             << "th, td { padding: 8px; text-align: left; border-bottom: 1px solid #ddd; }\r\n"
             << "tr:hover { background-color: #f5f5f5; }\r\n"
             << "a { text-decoration: none; }\r\n"
             << "a:hover { text-decoration: underline; }\r\n"
             << "</style>\r\n"
             << "</head>\r\n<body>\r\n"
             << "<h1>Index of " << this->_request.getPath() << "</h1>\r\n"
             << "<table>\r\n<tr><th>Name</th><th>Size</th><th>Last Modified</th></tr>\r\n";

        // Add parent directory link
        if (this->_request.getPath() != "/") {
            html << "<tr><td><a href=\"..\">..</a></td><td>-</td><td>-</td></tr>\r\n";
        }

        current_entry = html.str();
        _headers_sent = true;
    }

    if (current_entry.empty()) {
        entry = readdir(_dir);
        if (entry == NULL) {
            std::string footer = "</table>\r\n</body>\r\n</html>\r\n";
            ::memcpy(const_cast<uint8_t*>(io_buffer), footer.c_str(), footer.length());
            this->_state = handler_state_t(DONE, true);
            return footer.length();
        }

        // Skip . and .. entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            return 0;
        }

        std::string entry_path = this->_request.getResolvedPath() + "/" + entry->d_name;
        struct stat entry_stat;
        if (stat(entry_path.c_str(), &entry_stat) == 0) {
            std::stringstream line;
            char timeStr[64];
            struct tm* timeinfo = localtime(&entry_stat.st_mtime);
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);

            line << "<tr><td><a href=\"" << joinPath(this->_request.getConfigLocationStr(), entry->d_name);
            if (S_ISDIR(entry_stat.st_mode))
                line << "/";
            line << "\">" << entry->d_name;
            if (S_ISDIR(entry_stat.st_mode))
                line << "/";
            line << "</a></td><td>";

            if (S_ISDIR(entry_stat.st_mode))
                line << "-";
            else
                line << formatSize(entry_stat.st_size);

            line << "</td><td>" << timeStr << "</td></tr>\r\n";
            current_entry = line.str();
        }
    }

    size_t to_send = std::min(current_entry.length(), buff_length);
    ::memcpy(const_cast<uint8_t*>(io_buffer), current_entry.c_str(), to_send);
    current_entry = current_entry.substr(to_send);

    return to_send;
}
