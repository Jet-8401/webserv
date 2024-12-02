#include "../headers/WebServ.hpp"
#include "../headers/HttpResponse.hpp"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

// Static variables
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpResponse::mime_types_t&	init_mime_types(void)
{
	static HttpResponse::mime_types_t	mime_types;

	mime_types[".html"] = "text/html";
	mime_types[".css"] = "text/css";
	mime_types[".js"] = "application/javascript";
	mime_types[".json"] = "application/json";
	mime_types[".png"] = "image/png";
	mime_types[".jpg"] = "image/jpeg";
	mime_types[".jpeg"] = "image/jpeg";
	mime_types[".gif"] = "image/gif";
	mime_types[".pdf"] = "application/pdf";
	mime_types[".txt"] = "text/plain";
	mime_types[".mp4"] = "video/mp4";
	mime_types[".mp3"] = "audio/mpeg";
	mime_types[".xml"] = "application/xml";
	mime_types[".zip"] = "application/zip";

	return (mime_types);
}

HttpResponse::mime_types_t&	HttpResponse::mime_types = init_mime_types();

// Constructor / Destructor
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpResponse::HttpResponse(void):
	status_code(200),
	_are_headers_sent(false),
	_are_headers_parsed(false),
	_action(NONE),
	_is_done(false),
	_file_fd(-1),
	_dir(0),
	_location(0),
	_are_media_written_to_disk(false)
{
	::memset(&this->_media_stat, 0, sizeof(this->_media_stat));
	this->setHeader("Server", "webserv/1.0");
	this->setHeader("Connection", "close");
}

HttpResponse::~HttpResponse(void)
{
	if (this->_file_fd != -1)
		::close(this->_file_fd);
}

// Getters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const bool&	HttpResponse::areHeadersSent(void) const
{
	return (this->_are_headers_sent);
}

const bool&	HttpResponse::areHeadersParsed(void) const
{
	return (this->_are_headers_parsed);
}

const bool& HttpResponse::areMediaWrittenToDisk(void) const
{
	return (this->_are_media_written_to_disk);
}

const bool& HttpResponse::isDone(void) const
{
	return (this->_is_done);
}

const ::uint8_t&	HttpResponse::getActionBits(void) const
{
	return (this->_action);
}

bool	HttpResponse::isSendingMedia(void) const
{
	return (this->_action & SENDING_MEDIA);
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

// Resolve the path with alias, root, index, etc...
int	HttpResponse::_resolveLocation(std::string& path, struct stat& file_stats, const std::string& request_location)
{
	const std::string&			alias = this->_location->getAlias();
	std::vector<std::string>	indexes = this->_location->getIndexes();

	if (!alias.empty())
		path = joinPath(this->_location->getRoot(), alias);
	else
		path = joinPath(this->_location->getRoot(), request_location);

	// know test for multiples index if there is
	std::string	full_path;
	if (this->_action & SENDING_MEDIA) {
		for (std::vector<std::string>::const_iterator it = indexes.begin(); it != indexes.end(); it++) {
			full_path = joinPath(path, *it);
			::memset(&file_stats, 0, sizeof(file_stats));
			if (::stat(full_path.c_str(), &file_stats) != -1) {
				path = full_path;
				return (0);
			}
		}
	}

	// else take the path as final try
	if (::stat(path.c_str(), &file_stats) == -1)
		return (this->status_code = 404, -1);
	return (0);
}

// Check if the request can be resolved with a matching location.
// Will open directory or file automatically if found.
int	HttpResponse::_resolveRequest(const HttpRequest& request)
{
	if (this->_resolveLocation(this->_complete_path, this->_media_stat, request.getLocation()) == -1)
		return (-1);

	DEBUG("final destination -> " << this->_complete_path);

	// Check for autoindex.
	if (S_ISDIR(this->_media_stat.st_mode)) {
		this->_dir = ::opendir(this->_complete_path.c_str());
		if (!this->_dir)
			return (error(ERR_DIR_OPENING, true), this->status_code = 500, -1);
		if (this->_location->getAutoIndex())
			this->_action |= DIRECTORY_LISTING;
	} else if (this->_action & SENDING_MEDIA) {
		this->_action |= STATIC_FILE;
		this->_file_fd = ::open(this->_complete_path.c_str(), O_RDWR);
		if (this->_file_fd == -1)
			return (error(ERR_FILE_OPEN, true), this->status_code = 500, -1);
	}
	return (0);
}

void HttpResponse::_buildHeaders(std::stringstream& response) const
{
    response << "HTTP/1.1 " << this->status_code << "\r\n";
    for (headers_t::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
        response << it->first << ": " << it->second << "\r\n";
    response << "\r\n";
}
// Location* HttpResponse::findMatchingLocation(const std::string& path, const std::map<std::string, Location*>& locations)
// {
//     std::map<std::string, Location*>::const_iterator root = locations.find("/");
//     Location* match = (root != locations.end()) ? root->second : NULL;
//     std::string longest = "";

//     std::map<std::string, Location*>::const_iterator it;
//     for (it = locations.begin(); it != locations.end(); ++it)
//     {
//         if (path.find(it->first) == 0 && it->first.length() > longest.length())
//         {
//             longest = it->first;
//             match = it->second;
//         }
//     }
//     return match;
// }

// Envoyer un fichier -> SENDING_MEDIA
// - Envoyer un fichier static
// - Exécuter un cgi -> file extension
// - Lister un répertoire -> DIRECTORY_LISTING
// Recevoir un fichier -> POST -> ACCEPT_MEDIA
// Supprimer un fichier -> DELETE -> DELETE_MEDIA

// Entry point for solving an http request.
// That function check for all prerequisites
int	HttpResponse::handleRequest(const ServerConfig& config, const HttpRequest& request)
{
	const std::string&							request_location = request.getLocation();
	ServerConfig::locations_t					server_locations = config.getLocations();
	ServerConfig::locations_t::const_iterator	it;
	ServerConfig::locations_t::const_iterator	matching = server_locations.end();

	this->_are_headers_parsed = true;
	DEBUG("handle request");
	// Take the matching location/route
	for (it = server_locations.begin(); it != server_locations.end(); it++) {
		if (request_location.find(it->first) == 0 &&
			(matching == server_locations.end() || it->first.length() >= matching->first.length()))
			matching = it;
	}

	if (!matching->second)
		return (this->status_code = 500, -1);
	this->_location = matching->second;
	DEBUG("matching: " << matching->first);

	// If request already have an error, it is inherited to response
	if (request.getStatusCode() >= 400) {
		this->_action = SENDING_MEDIA;
		this->status_code = request.getStatusCode();
		return (-1);
	}

	// Checking the method
	const std::string&	method = request.getMethod();
	DEBUG("request method: " << method);
	if (this->_location->getMethods().find(method) == this->_location->getMethods().end())
		return (this->status_code = 405, -1);
	if (method == "POST")
		this->_action = ACCEPTING_MEDIA;
	else if (method == "GET")
		this->_action = SENDING_MEDIA;
	else
		this->_action = DELETING_MEDIA;

	return (this->_resolveRequest(request));
}

void	HttpResponse::setHeader(const std::string key, const std::string value)
{
	this->_headers.insert(std::pair<const std::string, const std::string>(key, value));
}

int	HttpResponse::sendHeaders(const int socket_fd)
{
	DEBUG("Sending Headers !" << " (" << this->status_code << ')');
	std::stringstream	headers;
	this->_buildHeaders(headers);
	if (::write(socket_fd, headers.str().c_str(), headers.str().size()) == -1)
		return (error("Error writing response", true), -1);
	this->_are_headers_sent = true;
	return (0);
}

int	HttpResponse::_sendStaticFile(const int socket_fd)
{
	ssize_t		bytes;
	::uint8_t	packet[PACKETS_SIZE];

	bytes = ::read(this->_file_fd, packet, sizeof(packet));
	if (bytes == -1)
		return (error(ERR_READING_FILE, true), this->_is_done = true, -1);

	DEBUG("sending response packet to client");
	if (::write(socket_fd, packet, bytes) == -1)
		return (error(ERR_SOCKET_WRITE, true), this->_is_done = true, -1);

	if (bytes == 0)
		this->_is_done = true;

	return (0);
}

void	HttpResponse::setStaticMediaHeaders(void)
{
	std::map<std::string, std::string>::const_iterator	it;

	if (this->_action & STATIC_FILE) {
		this->setHeader("Content-Length", unsafe_itoa(this->_media_stat.st_size));
		std::string	ext = this->_complete_path.substr(this->_complete_path.find('.'));
		std::cout << "extension: ["  << ext << ']' << std::endl;
		it = this->mime_types.find(ext);
		if (it == this->mime_types.end())
			return ;
		this->setHeader("Content-Type", it->second);
	}
}

int	HttpResponse::sendMedia(const int socket_fd)
{
	if (this->_action & STATIC_FILE) {
		return (this->_sendStaticFile(socket_fd));
	}
	return (0);
}

int	HttpResponse::handleError(void)
{
	const Location::error_page_t&			error_pages = this->_location->getErrorPages();
	Location::error_page_t::const_iterator	it;
	this->_action = NONE;

	// check if there is an error page that match into the location
	it = error_pages.find(this->status_code);
	if (it == error_pages.end())
		return (-1);

	// check if the path exist
	if (::stat(it->second->c_str(), &this->_media_stat) == -1)
		return (-1);

	if (this->_file_fd != -1)
		::close(this->_file_fd);
	this->_file_fd = ::open(it->second->c_str(), O_RDONLY);
	if (this->_file_fd == -1)
		return (-1);
	this->_action |= SENDING_MEDIA | STATIC_FILE;
	return (0);
}

int HttpResponse::_generateAutoIndex(const int socket_fd, const std::string& uri)
{
    if (!this->_dir || !this->_location)
        return (-1);

    std::stringstream html;
    struct dirent* entry;
    struct stat st;
    std::string path;
    html << "<!doctype html>";
    html << "<html>\r\n<head>\r\n<title>Index of " << uri << "</title>\r\n";
    html << "<style>\r\n";
    html << "body { font-family: monospace; }\r\n";
    html << "td { padding: 0 10px; }\r\n";
    html << "</style>\r\n</head>\r\n<body>\r\n";
    html << "<h1>Index of " << uri << "</h1><hr>\r\n";
    html << "<table>\r\n";
    html << "<tr><th>Name</th><th>Last Modified</th><th>Size</th></tr>\r\n";

    rewinddir(this->_dir);
    while ((entry = readdir(this->_dir))) {
        if (std::string(entry->d_name) == ".")
            continue;

        path = this->_complete_path + "/" + entry->d_name;
        if (stat(path.c_str(), &st) == -1)
            continue;

        html << "<tr><td><a href=\"" << entry->d_name;
        if (S_ISDIR(st.st_mode))
            html << "/";
        html << "\">" << entry->d_name;
        if (S_ISDIR(st.st_mode))
            html << "/";
        html << "</a></td>";

        char timeStr[100];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&st.st_mtime));
        html << "<td>" << timeStr << "</td>";

        if (S_ISDIR(st.st_mode))
            html << "<td>-</td>";
        else
            html << "<td>" << st.st_size << "B</td>";
        html << "</tr>\r\n";
    }

    html << "</table>\r\n<hr>\r\n</body>\r\n</html>\r\n";

    std::string index = html.str();
    this->setHeader("Content-Type", "text/html");
    this->sendHeaders(socket_fd);
    if (write(socket_fd, index.c_str(), index.length()) == -1)
        return (-1);
    this->_is_done = true;
    std::cout << "HERE: " << this->_is_done << std::endl;
    return (0);
}

int	HttpResponse::writeMediaToDisk(HttpRequest& request)
{
	uint8_t	packet[PACKETS_SIZE];
	ssize_t	bytes = 0;

	while ((bytes = request.request_body.consume(packet, sizeof(packet))) > 0) {
		::write(1, packet, bytes);
	}
	DEBUG("WRITING MEDIA TO DISK");
	this->_are_media_written_to_disk = true;
	return (0);
}
