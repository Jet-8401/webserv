#include "../headers/ServerCluster.hpp"
#include "../headers/WebServ.hpp"
#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <sys/epoll.h>
#include <fcntl.h>

// Static variables
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

std::map<std::string, void (ServerConfig::*)(const std::string&)>	ServerCluster::serverSetters;
std::map<std::string, void (Location::*)(const std::string&)>		ServerCluster::locationSetters;

// Constructors / Destructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

std::map<std::string, void (ServerConfig::*)(const std::string&)> ServerCluster::_server_setters;
std::map<std::string, void (Location::*)(const std::string&)> ServerCluster::_location_setters;
std::map<std::string, void (Location::*)(const std::string&)> ServerCluster::_http_location_setters;
std::map<std::string, void (Location::*)(const std::string&)> ServerCluster::_serv_location_setters;

void ServerCluster::initDirectives()
{
	// http
	_http_location_setters["root"] = &Location::setRoot;
	_http_location_setters["client_max_body_size"] = &Location::setClientMaxBodySize;
	_http_location_setters["error_page"] = &Location::setErrorPage;
	_http_location_setters["index"] = &Location::setIndex;
	_http_location_setters["autoindex"] = &Location::setAutoindex;
	// serv
	_server_setters["listen"] = &ServerConfig::setAdress;
	_server_setters["server_name"] = &ServerConfig::setServerName;
	_server_setters["max_connections"] = &ServerConfig::setMaxConnections;
	_serv_location_setters["return"] = &Location::setReturn;
	_serv_location_setters["root"] = &Location::setRoot;
	_serv_location_setters["autoindex"] = &Location::setAutoindex;
	_serv_location_setters["index"] = &Location::setIndex;
	_serv_location_setters["client_max_body_size"] = &Location::setClientMaxBodySize;
	_serv_location_setters["error_page"] = &Location::setErrorPage;
	// location
	_location_setters["return"] = &Location::setReturn;
	_location_setters["cgi"] = &Location::setCgis;
	_location_setters["methods"] = &Location::setMethods;
	_location_setters["root"] = &Location::setRoot;
	_location_setters["alias"] = &Location::setAlias;
	_location_setters["autoindex"] = &Location::setAutoindex;
	_location_setters["index"] = &Location::setIndex;
	_location_setters["client_max_body_size"] = &Location::setClientMaxBodySize;
	_location_setters["error_page"] = &Location::setErrorPage;
}

ServerCluster::ServerCluster(void):
	_epoll_fd(-1),
	_running(1)
{
	this->initDirectives();
}

ServerCluster::~ServerCluster(void)
{
	if (this->_epoll_fd != -1)
		close(this->_epoll_fd);
}

// Getters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const ServerCluster::servers_type_t& ServerCluster::getServers(void) const
{
	return (this->_servers);
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

int ServerCluster::importConfig(const std::string& config_path)
{
	std::ifstream file(config_path.c_str());
	if (!file.is_open())
    	return (error(ERR_FILE_OPEN + config_path, true), -1);

	std::stringstream ss;
	ss << file.rdbuf();
	file.close();

    std::string token;
    while (ss >> token)
    {
        if (token == "http")
        {
            if (parseHttpBlock(ss) < 0)
                return (-1);
        }
    }
    return (0);
}

int ServerCluster::parseHttpBlock(std::stringstream& ss)
{
    std::string token;
    ss >> token;
    if (token != "{")
        return (error("Expected '{' after http", true), -1);

    Location http_location;
    parseHttpBlockDefault(ss, &http_location);

    while (ss >> token)
    {
        if (token == "}")
            return (0);
        else if (token == "server")
        {
            ServerConfig config;
            if (parseServerBlock(ss, config, &http_location) < 0)
                return (-1);
        }
    }
    std::cout << "FUCK"<< ss.eof() << std::endl;
    return (error("Unexpected end of http block", true), -1);
}

int ServerCluster::parseHttpBlockDefault(std::stringstream& ss, Location* http_location)
{
	std::streampos pos = ss.tellg();
	std::string token;

	while (ss >> token)
	{
		if (token == "}")
		{
			ss.clear();
			ss.seekg(pos); // we reset the pos in the ss because we read in advance
			return (0);
		}
		else if (token == "server")
		{
			int brace_count = 0;
			while (ss >> token && (brace_count += (token == "{") - (token == "}")) != 0);
		}
		else
		{
			std::map<std::string, void (Location::*)(const std::string&)>::iterator it = _http_location_setters.find(token);
			if (it != _http_location_setters.end())
			{
				std::cout << "#####################################################" << std::endl;
				std::string value;
				std::getline(ss, value, ';'); // Read until semicolon
				value = value.substr(value.find_first_not_of(" \t")); // Trim leading whitespace
				(http_location->*(it->second))(value);
			}
		}
	}
	return (error("Unexpected end of http block", true), -1);
}

int ServerCluster::parseServerBlockDefault(std::stringstream& ss, Location* serv_location)
{
	std::streampos pos = ss.tellg();
	std::string token;
	while (ss >> token)
	{
		if (token == "}")
		{
			ss.clear();
			ss.seekg(pos); // we reset the pos in the ss because we read in advance
			return (0);
		}
		else if (token == "location")
		{
			std::getline(ss, token, '}');
		}
		else
		{
			std::map<std::string, void (Location::*)(const std::string&)>::iterator it = _serv_location_setters.find(token);
			if (it != _serv_location_setters.end())
			{
				std::string value;
				std::getline(ss, value, ';');
				value = value.substr(value.find_first_not_of(" \t"));
				(serv_location->*(it->second))(value);
			}
		}
	}
	return (error("Unexpected end of server block", true), -1);
}

int ServerCluster::parseServerBlock(std::stringstream& ss, ServerConfig& config, Location* http_location)
{
	std::string token;
	ss >> token;
	if (token != "{")
		return (error("Expected '{' after server", true), -1);

	std::cout << "Parsing server block..." << std::endl; // Debug

	Location serv_location(*http_location);
	parseServerBlockDefault(ss, &serv_location);
	std::cout << serv_location.getRoot() << "OOOOOOO" << std::endl;
	while (ss >> token)
	{
		std::cout << "Token: " << token << std::endl; // Debug
		if (token == "}")
		{
			if (config.getLocations().empty())
				config.addLocation("/", new Location(serv_location));
			HttpServer server(config);
			_servers.push_back(server); // Use size as key instead of invalid socket fd
			std::cout << "Added server with port: " << config.getPort() << std::endl; // Debug
			return (0);
		}
		else if (token == "location")
		{
			std::string paths;
			std::getline(ss, paths);
			std::cout << "Parsing location: " << paths  << "|" << std::endl; // Debug
			Location *location = new Location(serv_location);
			if (parseLocationBlock(ss, location) < 0)
				return (-1);
			std::stringstream ss_paths(paths);
			while (ss_paths >> paths)
			{
				config.addLocation(paths, location);
			}
		}
		else
		{
			std::map<std::string, void (ServerConfig::*)(const std::string&)>::iterator it = _server_setters.find(token);
			if (it != _server_setters.end())
			{
				std::string value;
				std::getline(ss, value, ';'); // Read until semicolon
				value = value.substr(value.find_first_not_of(" \t")); // Trim leading whitespace
				//std::cout << "Setting " << token << " to " << value << std::endl; // Debug
				(config.*(it->second))(value);
			}
		}
	}
	return (error("Unexpected end of server block", true), -1);
}

int ServerCluster::parseLocationBlock(std::stringstream& ss, Location* location)
{
	std::string token;
	ss >> token;
	std::cout << "LE token : " << token << std::endl;
	if (token != "{")
		return (error("Expected '{' after location", true), -1);

	while (ss >> token)
	{
		if (token == "}")
			return (0);
		else
		{
			std::map<std::string, void (Location::*)(const std::string&)>::iterator it = _location_setters.find(token);
			if (it != _location_setters.end())
			{
				std::string value;
				std::getline(ss, value, ';'); // Read until semicolon
				value = value.substr(value.find_first_not_of(" \t")); // Trim leading whitespace
				(location->*(it->second))(value);
			}
		}
	}
	return (error("Unexpected end of location block", true), -1);
}

// Make all virtual server listen on their sockets and check for conncetions with epoll.
//
// The data store by epoll_data_t is a pointer to a wrapper class that will be used to keep track
// of the original data-type of the pointer as it can be an HttpServer instance for connections request and
// for already setup connections it is a Connection instance, therefore casting it int the right type.
// This method is one of the most efficient as we can directly access the instance associated to that file descriptor.
// For every events returned we check the enum of the wrapper class for the casting, then onEvent() is called
// on that instance.
int	ServerCluster::run(void)
{
	servers_type_t::iterator	it;
	struct epoll_event			incoming_events[MAX_EPOLL_EVENTS];
	event_wrapper_t*			event_wrapper;
	int							events;

	this->_epoll_fd = ::epoll_create(this->_servers.size());
	if (this->_epoll_fd == -1)
		return (error(ERR_EPOLL_CREATION, false), -1);

	for (it = this->_servers.begin(); it != this->_servers.end(); it++)
	{
		struct epoll_event	ep_event;

		if (it->listen() == -1)
			return (-1);
		it->setEpollFD(this->_epoll_fd);
		event_wrapper = this->_events_wrapper.create(REQUEST);
		event_wrapper->casted_value = &(*it);
		ep_event.events = EPOLLIN | EPOLLET;
		ep_event.data.ptr = static_cast<void*>( event_wrapper );
		if (::epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, it->getSocketFD(), &ep_event) == -1)
			return (error(ERR_EPOLL_ADD, true), -1);
	}

	// wait for the events pool to trigger
	while (this->_running) {
		::memset(&incoming_events, 0, sizeof(incoming_events));
		events = ::epoll_wait(this->_epoll_fd, incoming_events, MAX_EPOLL_EVENTS, -1);
		if (events  == -1)
			return (error(ERR_EPOLL_WAIT, true), -1);
		this->_resolveEvents(incoming_events, events);
	}
	return (0);
}

void	ServerCluster::_resolveEvents(struct epoll_event incoming_events[MAX_EPOLL_EVENTS], int events)
{
	event_wrapper_t*			event_wrapper;

	DEBUG(events << " events received");
	for (int i = 0; i < events; i++) {
		DEBUG((incoming_events[i].events & EPOLLIN ? "EPOLLIN" : "EPOLLOUT / EPOLLHUP") << " event received");
		event_wrapper = static_cast<event_wrapper_t*>(incoming_events[i].data.ptr);
		switch (event_wrapper->socket_type)
		{
		case REQUEST:
			DEBUG("event[" << i << "]: connection request");
			static_cast<HttpServer*>(event_wrapper->casted_value)->onEvent(incoming_events[i].events);
			break ;
		case CLIENT:
			DEBUG("event[" << i << "]: client package");
			static_cast<Connection*>(event_wrapper->casted_value)->onEvent(incoming_events[i].events);
			break ;
		default:
			break;
		}
	}
}
