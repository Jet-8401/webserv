#include "../headers/ServerCluster.hpp"
#include "../headers/WebServ.hpp"
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <map>
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

ServerCluster::ServerCluster(void):
	_epoll_fd(-1),
	_running(1)
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

ServerCluster::~ServerCluster(void)
{
	if (this->_epoll_fd != -1)
		close(this->_epoll_fd);
}

// Getters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const std::list<ServerConfig>	ServerCluster::getConfigs(void) const
{
	return (this->_configs);
}


// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

// size_t	count = sockets.count(addr_it->second);
// 	std::cout << count << " founds for port " << addr_it->second << std::endl;

// 	if (count == 0 || (addr_it->first != "127.0.0.1" && addr_it->first != "0.0.0.0")) {
// 		std::cout << "added: " << addr_it->first << ':' << addr_it->second << std::endl;
// 		sockets.insert(std::pair<uint16_t, std::string>(addr_it->second, addr_it->first));
// 		return;
// 	}

// 	// if same port found check the precedence of 0.0.0.0 and 127.0.0.1
// 	std::pair< std::multimap<uint16_t, std::string>::iterator, std::multimap<uint16_t, std::string>::iterator > ret;
// 	ret = sockets.equal_range(addr_it->second);
// 	for (std::multimap<uint16_t, std::string>::iterator it = ret.first; it != ret.second; it++) {
// 		if (it->second != "0.0.0.0" && it->second != "127.0.0.1") {
// 			std::cout << "ignoring: " << it->second << ':' << it->first << std::endl;
// 			continue;
// 		}

// 		if (it->second == "0.0.0.0") {
// 			std::cerr << "\033[31mWARNING: address (" << addr_it->first
// 				<< ':' << addr_it->second << ") will be ignored (already selected)\033[0m" << std::endl;
// 		} else {
// 			// delete the 127.0.0.1 to make a socket listening on 0.0.0.0
// 			std::cerr << "\033[31mWARNING: address (" <<  it->second
// 				<< ':' << it->first << ") will be deleted (0.0.0.0 precedence)\033[0m" << std::endl;
// 			sockets.erase(it);
// 			sockets.insert(std::pair<uint16_t, std::string>(addr_it->second, addr_it->first));
// 			break;
// 		}
// 	}

void	addAddress(ServerConfig::address_type::const_iterator& addr_it, std::multimap<uint16_t, std::string>& sockets)
{
	size_t	count = sockets.count(addr_it->second);
	std::cout << count << " founds for port " << addr_it->second << std::endl;

	if (count == 0) {
		std::cout << "added: " << addr_it->first << ':' << addr_it->second << std::endl;
		sockets.insert(std::pair<uint16_t, std::string>(addr_it->second, addr_it->first));
		return;
	}

	// if same port found check that they are the same
	std::pair< std::multimap<uint16_t, std::string>::iterator, std::multimap<uint16_t, std::string>::iterator > ret;
	ret = sockets.equal_range(addr_it->second);
	for (std::multimap<uint16_t, std::string>::iterator it = ret.first; it != ret.second; it++) {
		if (it->second == addr_it->first)
			continue;
		sockets.insert(std::pair<uint16_t, std::string>(addr_it->second, addr_it->first));
	}
}

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
		if (token == "http" && parseHttpBlock(ss) < 0)
			return (-1);
	}

	// iterate through configurations for setting the sockets
	std::multimap<uint16_t, std::string>		sockets; // possible sockets (post, host)
	std::list<ServerConfig>::const_iterator 	conf_it;
	ServerConfig::address_type::const_iterator	addr_it;
	for (conf_it = this->_configs.begin(); conf_it != this->_configs.end(); conf_it++) {
		const ServerConfig::address_type& addresses = conf_it->getAddresses();
		for (addr_it = addresses.begin(); addr_it != addresses.end(); addr_it++) {
			addAddress(addr_it, sockets);
		}
	}

	std::cout << "At end there is " << sockets.size() << " sockets" << std::endl;
	std::multimap<uint16_t, std::string>::const_iterator it;
	for (it = sockets.begin(); it != sockets.end(); it++) {
		std::cout << it->second << ':' << it->first << std::endl;
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
	while (ss >> token)
	{
		std::cout << "Token: " << token << std::endl; // Debug
		if (token == "}")
		{
			if (config.getLocations().empty())
				config.addLocation("/", new Location(serv_location));
			this->_configs.push_back(config);
			// HttpServer server(config);
			// _servers.push_back(server); // Use size as key instead of invalid socket fd
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

// First check if the port exist.

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
	// servers_type_t::iterator	it;
	// struct epoll_event			incoming_events[MAX_EPOLL_EVENTS];
	// event_wrapper_t*			event_wrapper;
	// int							events;

	// this->_epoll_fd = ::epoll_create(this->_servers.size());
	// if (this->_epoll_fd == -1)
	// 	return (error(ERR_EPOLL_CREATION, false), -1);

	// for (it = this->_servers.begin(); it != this->_servers.end(); it++)
	// {
	// 	struct epoll_event	ep_event;

	// 	if (it->listen() == -1)
	// 		return (-1);
	// 	it->setEpollFD(this->_epoll_fd);
	// 	event_wrapper = this->_events_wrapper.create(REQUEST);
	// 	event_wrapper->casted_value = &(*it);
	// 	ep_event.events = EPOLLIN;
	// 	ep_event.data.ptr = static_cast<void*>( event_wrapper );
	// 	if (::epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, it->getSocketFD(), &ep_event) == -1)
	// 		return (error(ERR_EPOLL_ADD, true), -1);
	// }

	// // wait for the events pool to trigger
	// while (!is_done) {
	// 	::memset(&incoming_events, 0, sizeof(incoming_events));
	// 	events = ::epoll_wait(this->_epoll_fd, incoming_events, MAX_EPOLL_EVENTS, MS_TIMEOUT_ROUTINE);
	// 	if (events  == -1)
	// 		return (error(ERR_EPOLL_WAIT, true), -1);
	// 	this->_resolveEvents(incoming_events, events);
	// }
	return (0);
}

void	ServerCluster::_resolveEvents(struct epoll_event incoming_events[MAX_EPOLL_EVENTS], int events)
{
	event_wrapper_t*			event_wrapper;

	if (events != 0)
		DEBUG(events << " events received");
	for (int i = 0; i < events; i++) {
		if (incoming_events[i].events & EPOLLIN)
			DEBUG("EPOLLIN");
		if (incoming_events[i].events & EPOLLOUT)
			DEBUG("EPOLLOUT");
		if (incoming_events[i].events & EPOLLHUP)
			DEBUG("EPOLLHUP");
		event_wrapper = static_cast<event_wrapper_t*>(incoming_events[i].data.ptr);
		switch (event_wrapper->socket_type)
		{
			// case REQUEST:
			// 	DEBUG("event[" << i << "]: connection request");
			// 	static_cast<HttpServer*>(event_wrapper->casted_value)->onEvent(incoming_events[i].events);
			// 	break ;
			// case CLIENT:
			// 	DEBUG("event[" << i << "]: client package");
			// 	static_cast<Connection*>(event_wrapper->casted_value)->onEvent(incoming_events[i].events);
			// 	break ;
			// default:
			// 	break;
		}
	}
}
