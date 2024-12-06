#include "../headers/HttpHandler.hpp"
#include "../headers/HttpGetStaticMethod.hpp"
#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include <iostream>	// To remove
#include <sys/socket.h>
#include <sys/types.h>
#include <cstring>

static const char END_SEQUENCE[4] = {'\r', '\n', '\r', '\n'};

void	string_trim(std::string& str)
{
	size_t	i;

	i = str.find_first_not_of(" \t");
	if (i != std::string::npos)
		str.erase(0, i);
	i = str.find_last_not_of(" \t");
	if (i != std::string::npos)
		str.erase(i + 1);
	return ;
}


// Constructors / Destructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpHandler::HttpHandler(const ServerConfig& config):
	HttpMessage(),
	_body(64000),
	_state(READING_HEADERS),
	_config_reference(config),
	_matching_location(0),
	_extanded_method(0)
{}

HttpHandler::~HttpHandler(void)
{
	if (this->_extanded_method)
		delete this->_extanded_method;
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
std::map<std::string, std::string>& HttpHandler::init_mime_types(void)
{
    static std::map<std::string, std::string> mime_types;

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

    return mime_types;
}

std::map<std::string, std::string>& HttpHandler::mime_types = HttpHandler::init_mime_types();

const std::string& HttpHandler::getLocationString(void) const
{
    return this->_config_location_str;
}

bool	HttpHandler::_bufferHeaders(const uint8_t* packet, size_t packet_size)
{
	const uint8_t*	buffer = this->_header_buff.read();
	const uint8_t*	addr;
	size_t			starting_point;

	starting_point = std::max(static_cast<ssize_t>(0), static_cast<ssize_t>(this->_header_buff.size()) - 3);
	if (this->_header_buff.write(packet, packet_size) == -1)
		return (this->_status_code = 431, false);

	addr = std::search(
		buffer + starting_point,
		buffer + this->_header_buff.size(),
		END_SEQUENCE,
		END_SEQUENCE + sizeof(END_SEQUENCE)
	);

	// if the end sequence has not been found
	if (addr == buffer + this->_header_buff.size())
		return (true);

	// If found change to the next state.
	// And check if there is a body and it is inside the given packet, write it to the buffer body.
	this->_end_header_index = (reinterpret_cast<size_t>(addr) - reinterpret_cast<size_t>(buffer)) +
		sizeof(END_SEQUENCE);
	std::cout << "end header index: " << this->_end_header_index << std::endl;
	std::cout << "buffer size: " << this->_header_buff.size() << std::endl;
	std::cout << (char*) buffer;
	if (this->_end_header_index != this->_header_buff.size()) {
		std::cout << "body is inside the packet" << std::endl;
		this->_body.write(buffer + this->_end_header_index, this->_header_buff.size() - this->_end_header_index);
	}
	this->_state = CHECK_METHOD;
	return (true);
}

// check the syntax of the headers, like if their are two unique headers etc...
bool	HttpHandler::_checkHeaderSyntax(const std::string& key, const std::string& value) const
{
	headers_behavior_t::const_iterator	it = HttpMessage::_headers_handeled.find(key);

	if (it == HttpMessage::_headers_handeled.end())
		return (true);
	switch (it->second) {
		case UNIQUE:
			if (this->_headers.find(key) != this->_headers.end())
				return (false);
			// fallthrough
		case SEPARABLE:
			if (value.find(',') != std::string::npos)
				return (false);
			break;
		default:
			break;
	}
	return (true);
}

// Parse the request-line then all the headers, it also check for syntax.
// Check for only HTTP/1.1 version.
bool	HttpHandler::_parseHeaders(void)
{
	std::string			str;
	std::stringstream	parser;

	parser.write(reinterpret_cast<const char*>(this->_header_buff.read()), this->_end_header_index);

	parser >> this->_method;
	parser >> this->_path;
	parser >> this->_version;

	if (this->_version != "HTTP/1.1")
		return (this->_status_code = 505, false);

	std::string	key, value;
	parser.ignore();
	while (std::getline(parser, str)) {
		if (str.empty())
			continue;
		size_t	colon_pos = str.find(':');
		if (colon_pos == std::string::npos)
			continue ;

		// separate the key and value, then triming them
		key = str.substr(0, colon_pos);
		string_trim(key);
		value = str.substr(colon_pos + 1);
		string_trim(value);

		if (!this->_checkHeaderSyntax(key, value))
			return (this->_status_code = 400, false);
		this->_headers.insert(std::pair<std::string, std::string>(key, value));
	}
	return (true);
}

bool	HttpHandler::_findLocation(void)
{
	ServerConfig::locations_t					server_locations = _config_reference.getLocations();
	ServerConfig::locations_t::const_iterator	it;
	ServerConfig::locations_t::const_iterator	matching = server_locations.end();

	// Take the matching location/route
	for (it = server_locations.begin(); it != server_locations.end(); it++) {
		if (this->_path.find(it->first) == 0 &&
			(matching == server_locations.end() || it->first.length() >= matching->first.length()))
			matching = it;
	}
	if (!matching->second)
		return (this->_status_code = 500, false);
	this->_config_location_str = matching->first;
	this->_matching_location = matching->second;
	return (true);
}

bool	HttpHandler::_validateAndInitMethod(void)
{
	std::cout << "_validateAndInitMethod called" << std::endl;
	// only parse the headers
	if (!this->_parseHeaders()) {
		std::cout << "error while parsing headers" << std::endl;
		std::cout << "status_code: " << this->_status_code << std::endl;
		return (false);
	}

	// try to match a location
	if (!this->_findLocation()) {
		std::cout << "didn't find any locations" << std::endl;
		return (false);
	}

	std::cout << this->_config_location_str << " found!" << std::endl;
	std::cout << "Path requested: " << this->_path << std::endl;
	// check if method is allowed
	if (this->_matching_location->getMethods().find(this->_method) == this->_matching_location->getMethods().end())
		return (this->_status_code = 405, false);

	// based onto the headers check which extanded method to get
	if (this->_method == "GET") {
        // Vérifie d'abord si c'est un CGI
        std::string extension = this->_path.substr(this->_path.find_last_of("."));
        if (this->_matching_location->getCGIs().find(extension)
            != this->_matching_location->getCGIs().end()) {
            // this->_extanded_method = new HttpGetCGIMethod(*this);
            return true;
        }

        // Vérifie si c'est un répertoire
        if (this->_path[this->_path.length() - 1] == '/') {
            // if (this->_matching_location->getAutoIndex())
                // this->_extanded_method = new HttpGetDirectoryMethod(*this);
            // else
            //     this->_extanded_method = new HttpGetStaticMethod(*this); // cherchera index
            return true;
        }

        // Fichier statique par défaut
        this->_extanded_method = new HttpGetStaticMethod(*this);
    } else if (this->_method == "POST") {
    	std::cout << "POST" << std::endl;
        // this->_extanded_method = new HttpPostMethod(*this);
    } else if (this->_method == "DELETE") {
   	std::cout << "DELETE" << std::endl;
        // this->_extanded_method = new HttpDeleteMethod(*this);
    } else {
    	std::cout << "DELETE" << std::endl;
        this->_status_code = 501; // Not Implemented
        return false;
    }
	return (true);
}

bool HttpHandler::parse(const uint8_t* packet, const size_t packet_size) {
    switch (this->_state) {
        case READING_HEADERS:
            // Buffer les headers jusqu'à trouver \r\n\r\n
            if (!this->_bufferHeaders(packet, packet_size))
                return false;
            // Si headers complets, passer à l'état suivant

        case CHECK_METHOD:
            // Parse les headers et valide la méthode
            // Crée la spécialisation appropriée de AHttpMethod
            if (!this->_validateAndInitMethod())
                return false;
            this->_state = READING_BODY;
            this->_wstate = WAITING;

        case READING_BODY:
            // Une fois la méthode spécialisée créée, lui déléguer le parsing
            if (this->_extanded_method) {
                if (this->_extanded_method->parse(packet, packet_size)) {
                    this->_state = DONE;  // Met l'état à DONE quand le parsing est terminé
                    return true;
                }
            }
            break;

        default:
            break;
    }
    return true;
}

ssize_t HttpHandler::writePacket(uint8_t* io_buffer, size_t buff_length) {
    ssize_t bytes_written = 0;

    std::cout << "WritePacket called, state: " << this->_wstate
              << ", parsing state: " << this->_state << std::endl;

    switch (this->_wstate) {
        case WAITING:
        	std::cout << "State WAITING" << std::endl;
            // On vérifie que le parsing est terminé et qu'on a une méthode
            if (this->_state != DONE || !this->_extanded_method) {
                std::cout << "Not ready to write yet" << std::endl;
                return 0;
            }
            else
            {
	            std::string headers = this->buildHeadersString();
	            if (headers.length() > buff_length)
	                return -1;
	            ::memcpy(io_buffer, headers.c_str(), headers.length());
	            this->_wstate = HEADERS_SENT;
	            return headers.length();
            }

        case HEADERS_SENT:
        	std::cout << "State HEADERS_SENT" << std::endl;
            this->_wstate = SENDING_BODY;

        case SENDING_BODY:
        	std::cout << "State SENDING_BODY" << std::endl;
            bytes_written = this->_extanded_method->writePacket(io_buffer, buff_length);
            std::cout << "Body sending, bytes written: " << bytes_written << std::endl;
            if (bytes_written == 0)  // Si plus rien à envoyer
                this->_wstate = WDONE;
            return bytes_written;

        case WDONE:
       		std::cout << "State SENDING_BODY" << std::endl;
            std::cout << "Write completed" << std::endl;
            return 0;

        default:
            std::cout << "Unknown write state" << std::endl;
            return -1;
    }
    return bytes_written;
}

Location* HttpHandler::getMatchingLocation(void) const
{
    return this->_matching_location;
}

const std::string& HttpHandler::getPath(void) const
{
    return this->_path;
}

int HttpHandler::getStatusCode(void) const
{
    return this->_status_code;
}

void HttpHandler::setStatusCode(int code)
{
    this->_status_code = code;
}

void HttpHandler::addHeader(const std::string& key, const std::string& value)
{
    this->_headers.insert(std::pair<std::string, std::string>(key, value));
}
