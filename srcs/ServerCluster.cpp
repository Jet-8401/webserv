#include "../headers/ServerCluster.hpp"
#include "../headers/WebServ.hpp"
#include <algorithm>
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
#include <utility>

// Constructors / Destructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

std::map<std::string, void (ServerConfig::*)(const std::string&)> ServerCluster::_server_setters;
std::map<std::string, int (Location::*)(const std::string&)> ServerCluster::_location_setters;
std::map<std::string, int (Location::*)(const std::string&)> ServerCluster::_http_location_setters;
std::map<std::string, int (Location::*)(const std::string&)> ServerCluster::_serv_location_setters;

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

size_t	ServerCluster::getNumberOfConnections(void) const
{
	socket_t::const_iterator	it;
	int							count = 0;

	for (it = this->_sockets.begin(); it != this->_sockets.end(); it++) {
		count += it->getConnections().size();
	}
	return (count);
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

struct SocketComparer {
	ServerConfig::address_type::const_iterator _addr_it;

	SocketComparer(ServerConfig::address_type::const_iterator addr_it): _addr_it(addr_it) {}

	bool	operator()(const Socket& socket) const {
		return (socket.getIPV4() == _addr_it->first && socket.getPort() == _addr_it->second);
	}
};

bool	ServerCluster::_addAddress(std::list<ServerConfig>::const_iterator& conf_it,
	ServerConfig::address_type::const_iterator& addr_it)
{
	socket_t::iterator					it;

	it = std::find_if(this->_sockets.begin(), this->_sockets.end(), SocketComparer(addr_it));
	if (it == this->_sockets.end()) {
		this->_sockets.push_front(Socket(addr_it->first, addr_it->second));
		it = this->_sockets.begin();
	}

	return (it->addConfig(&(*conf_it)));
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
	std::map<std::pair<std::string, uint16_t>, std::list<ServerConfig*> >	sockets; // socket ip:port, list of configs for that socket
	std::list<ServerConfig>::const_iterator 	conf_it;
	ServerConfig::address_type::const_iterator	addr_it;

	for (conf_it = this->_configs.begin(); conf_it != this->_configs.end(); conf_it++) {
		const ServerConfig::address_type& addresses = conf_it->getAddresses();
		for (addr_it = addresses.begin(); addr_it != addresses.end(); addr_it++) {
			if (!this->_addAddress(conf_it, addr_it))
				return (-1);
		}
	}

	std::list<Socket>::iterator	it;

	for (it = this->_sockets.begin(); it != this->_sockets.end(); it++) {
		std::cout << it->getIPV4() << ':' << it->getPort() << std::endl;
	}
	return (0);
}

int ServerCluster::parseHttpBlock(std::stringstream& ss)
{
	std::string token;
	ss >> token;
	if (token != "{")
		return (error("Expected '{' after http", false), -1);

	Location http_location;

	if (parseHttpBlockDefault(ss, &http_location) < 0)
		return (-1);

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
	return (error("Unexpected end of http block", false), -1);
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
			std::map<std::string, int (Location::*)(const std::string&)>::iterator it = _http_location_setters.find(token);
			if (it != _http_location_setters.end())
			{
				std::string value;
				std::getline(ss, value, ';'); // Read until semicolon
				if (value.find('\n') != std::string::npos)
					return (error("Missing a ; at the end of the line!", false), -1);
				value = value.substr(value.find_first_not_of(" \t")); // Trim leading whitespace
				if ((http_location->*(it->second))(value) < 0)
					return (-1);
			}
		}
	}
	return (error("Unexpected end of http block", false), -1);
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
			std::map<std::string, int (Location::*)(const std::string&)>::iterator it = _serv_location_setters.find(token);
			if (it != _serv_location_setters.end())
			{
				std::string value;
				std::getline(ss, value, ';');
				if (value.find('\n') != std::string::npos)
					return (error("Missing a ; at the end of the line!", false), -1);
				value = value.substr(value.find_first_not_of(" \t"));
				if ((serv_location->*(it->second))(value) < 0)
					return (-1);
			}
		}
	}
	return (error("Unexpected end of server block", false), -1);
}

int ServerCluster::parseServerBlock(std::stringstream& ss, ServerConfig& config, Location* http_location)
{
	bool has_listen = false;
	std::string token;
	ss >> token;
	if (token != "{")
		return (error("Expected '{' after server", false), -1);

	Location serv_location(*http_location);
	if (parseServerBlockDefault(ss, &serv_location) < 0)
		return (-1);
	while (ss >> token)
	{
		if (token == "}")
		{
			if (!has_listen)
                return (error("Missing listen directive in server block", false), -1);
			if (config.getLocations().empty())
				config.addLocation("/", new Location(serv_location));
			this->_configs.push_back(config);
			return (0);
		}
		else if (token == "location")
		{
			std::string paths;
			std::getline(ss, paths);
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
				if (value.find('\n') != std::string::npos)
					return (error("Missing a ; at the end of the line!", false), -1);
				value = value.substr(value.find_first_not_of(" \t")); // Trim leading whitespace
				(config.*(it->second))(value);
				if (token == "listen")
                    has_listen = true;
			}
		}
	}
	return (error("Unexpected end of server block", false), -1);
}

int ServerCluster::parseLocationBlock(std::stringstream& ss, Location* location)
{
	std::string token;
	ss >> token;
	if (token != "{")
		return (error("Expected '{' after location", false), -1);

	while (ss >> token)
	{
		if (token == "}")
			return (0);
		else
		{
			std::map<std::string, int (Location::*)(const std::string&)>::iterator it = _location_setters.find(token);
			if (it != _location_setters.end())
			{
				std::string value;
				std::getline(ss, value, ';'); // Read until semicolon
				if (value.find('\n') != std::string::npos)
					return (error("Missing a ; at the end of the line!", false), -1);
				value = value.substr(value.find_first_not_of(" \t")); // Trim leading whitespace
				if ((location->*(it->second))(value) < 0)
					return (-1);
			}
		}
	}
	return (error("Unexpected end of location block", false), -1);
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
	socket_t::iterator	it;
	struct epoll_event			incoming_events[MAX_EPOLL_EVENTS];
	event_wrapper_t*			event_wrapper;
	int							events;

	this->_epoll_fd = ::epoll_create(this->_sockets.size());
	if (this->_epoll_fd == -1)
		return (error(ERR_EPOLL_CREATION, true), -1);

	for (it = this->_sockets.begin(); it != this->_sockets.end(); it++)
	{
		struct epoll_event	ep_event;

		if (it->listen() == -1)
			return (-1);
		it->setEpollFD(this->_epoll_fd);
		event_wrapper = this->_events_wrapper.create(REQUEST);
		event_wrapper->casted_value = &(*it);
		ep_event.events = EPOLLIN;
		ep_event.data.ptr = static_cast<void*>( event_wrapper );
		if (::epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, it->getSocketFD(), &ep_event) == -1)
			return (error(ERR_EPOLL_ADD, true), -1);
	}

	// wait for the events pool to trigger
	while (!is_done) {
		::memset(&incoming_events, 0, sizeof(incoming_events));
		events = ::epoll_wait(
			this->_epoll_fd,
			incoming_events,
			MAX_EPOLL_EVENTS,
			this->getNumberOfConnections() > 0 ? MS_TIMEOUT_ROUTINE : -1
		);
		if (events  == -1)
			return (error(ERR_EPOLL_WAIT, true), -1);
		this->_resolveEvents(incoming_events, events);
	}
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
			case REQUEST:
				DEBUG("event[" << i << "]: connection request");
				static_cast<Socket*>(event_wrapper->casted_value)->onEvent(incoming_events[i].events);
				break;
			case CLIENT:
				DEBUG("event[" << i << "]: client package");
				static_cast<Connection*>(event_wrapper->casted_value)->onEvent(incoming_events[i].events);
				break;
			default:
				break;
		}
	}
}
