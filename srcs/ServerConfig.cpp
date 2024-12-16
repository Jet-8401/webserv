#include "../headers/ServerConfig.hpp"
#include <sstream>
#include <string>

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

ServerConfig::ServerConfig(void):
	_max_connections(1024)
{}

ServerConfig::ServerConfig(const ServerConfig& src):
	_server_names(src._server_names),
	_addresses(src._addresses),
	_max_connections(src._max_connections)
{
	// deep copy of locations
	std::map<Location*, Location*>	ptr_copy;
	locations_t::const_iterator		it;

	for (it = src._locations.begin(); it != src._locations.end(); it++) {
		if (ptr_copy.find(it->second) != ptr_copy.end())
			continue ;
		ptr_copy[it->second] = new Location(*(it->second));
	}

	for (it = src._locations.begin(); it != src._locations.end(); it++) {
		this->_locations[it->first] = ptr_copy[it->second];
	}
}

ServerConfig::~ServerConfig(void)
{
	std::map<std::string, Location*>::iterator	it;
	std::set<Location*>							pointers;
	std::set<Location*>::iterator				pointer_it;

	for (it = this->_locations.begin(); it != this->_locations.end(); it++) {
		pointers.insert(it->second);
	}

	for (pointer_it = pointers.begin(); pointer_it != pointers.end(); pointer_it++)
		delete *pointer_it;
}

// Operator overloads
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

ServerConfig&	ServerConfig::operator=(const ServerConfig& rhs)
{
	this->_server_names = rhs._server_names;
	this->_locations = rhs._locations;
	return (*this);
}

// Getter
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const std::vector<std::string>&	ServerConfig::getServerNames(void) const
{
	return (this->_server_names);
}

// const std::string&	ServerConfig::getHost(void) const
// {
// 	return (this->_host);
// }

// const uint16_t&	ServerConfig::getPort(void) const
// {
// 	return (this->_port);
// }

const ServerConfig::address_type&	ServerConfig::getAddresses(void) const
{
	return (this->_addresses);
}

const std::map<std::string, Location*>&	ServerConfig::getLocations(void) const
{
	return (this->_locations);
}

const unsigned int&	ServerConfig::getMaxConnections(void) const
{
	return (this->_max_connections);
}

// Setters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

#include <iostream>

void	ServerConfig::setAdress(const std::string& value)
{
	size_t		colonPos = value.find(':');
	std::string	host("0.0.0.0");
	uint16_t	port;

	if (colonPos != std::string::npos)
	{
		host = value.substr(0, colonPos);
		port = static_cast<uint16_t>(atoi(value.c_str() + colonPos + 1));
	}
	port = static_cast<uint16_t>(atoi(value.c_str()));
	this->_addresses.push_back(std::pair<std::string, uint16_t>(host, port));
}

void	ServerConfig::setServerName(const std::string& value)
{
	std::istringstream iss(value);
	std::string name;
	while (iss >> name) {
		_server_names.push_back(name);
	}
}

#include <iostream>

void	ServerConfig::addLocation(const std::string& path, Location* location)
{
	std::string	new_path = path;
	if (path.length() > 1 && new_path[path.length() - 1] == '/')
		new_path.resize(path.length() - 1);
	_locations[new_path] = location;
}

void	ServerConfig::setMaxConnections(const std::string& value)
{
	std::istringstream	iss(value);

	iss >> _max_connections;
}

// Try to check for a matching location.
// Will never return false unless one the pointer is null, therefore an error occured previously.
ServerConfig::locations_t::const_iterator ServerConfig::findLocation(const std::string& path) const
{
	ServerConfig::locations_t::const_iterator	it;
	ServerConfig::locations_t::const_iterator	matching = this->_locations.end();

	// Take the matching location/route
	for (it = this->_locations.begin(); it != this->_locations.end(); it++) {
		if (path.find(it->first) == 0 &&
			(matching == this->_locations.end() || it->first.length() >= matching->first.length()))
			matching = it;
	}
	if (!matching->second)
		return (this->_locations.end());
	return (matching);
}
