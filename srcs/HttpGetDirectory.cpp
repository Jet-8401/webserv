#include "../headers/HttpGetDirectory.hpp"
#include "../headers/WebServ.hpp"
#include <dirent.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <ctime>

HttpGetDirectory::HttpGetDirectory(const HttpParser& parser):
    HttpParser(parser),
    _dir(NULL)
{
    this->_dir = opendir(this->_request.getResolvedPath().c_str());
    if (this->_dir == NULL) {
        error(ERR_DIR_OPENING, true);
        this->_state = this->_request.error(404);
    }
	this->_request.setEvents(EPOLLOUT);
	this->_state = handler_state_t(READY_TO_SEND, false);
	this->_createPage();
	this->_response.setHeader("Content-Type", this->_response.mime_types[".html"]);
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

void	HttpGetDirectory::_createPage(void)
{
	// Create base html page
	this->_current_entry << "<!DOCTYPE html>\r\n<html>\r\n<head>\r\n"
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
        this->_current_entry << "<tr><td><a href=\"..\">..</a></td><td>-</td><td>-</td></tr>\r\n";
    }

    struct dirent*	_entry;

    // Add directory entries
    while ((_entry = readdir(_dir)) != 0) {
		// Skip . and .. entries
		if (strcmp(_entry->d_name, ".") == 0 || strcmp(_entry->d_name, "..") == 0) {
			continue;
		}

		std::string entry_path = this->_request.getResolvedPath() + "/" + _entry->d_name;
		struct stat entry_stat;

		if (stat(entry_path.c_str(), &entry_stat) == 0) {
			char timeStr[64];
			struct tm* timeinfo = localtime(&entry_stat.st_mtime);
			std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);

			this->_current_entry << "<tr><td><a href=\"" << joinPath(this->_request.getConfigLocationStr(), _entry->d_name);
			if (S_ISDIR(entry_stat.st_mode))
				this->_current_entry << "/";
			this->_current_entry << "\">" << _entry->d_name;
			if (S_ISDIR(entry_stat.st_mode))
				this->_current_entry << "/";
			this->_current_entry << "</a></td><td>";
			if (S_ISDIR(entry_stat.st_mode))
				this->_current_entry << "-";
			else
				this->_current_entry << formatSize(entry_stat.st_size);
			this->_current_entry << "</td><td>" << timeStr << "</td></tr>\r\n";
		}
    }

    this->_current_entry << "</table>\r\n</body>\r\n</html>\r\n";
}

ssize_t HttpGetDirectory::write(uint8_t* io_buffer, const size_t buff_length)
{
    if (this->_state.flag != SENDING_BODY)
        return (this->HttpParser::write(io_buffer, buff_length));

    if (_dir == NULL) {
        this->_state = handler_state_t(ERROR, true);
        return (-1);
    }

	if (this->_current_entry.eof())
		this->_state = handler_state_t(DONE, true);
	this->_current_entry.read((char*) io_buffer, buff_length);
    return (this->_current_entry.gcount());
}
