#include "../headers/ServerConfig.hpp"
#include <sstream>
#include <string>

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

ServerConfig::ServerConfig(void):
	_host(),
	_port(0)
{}

ServerConfig::ServerConfig(const ServerConfig& src):
	_server_names(src._server_names),
	_host(src._host),
	_port(src._port),
	_locations(src._locations)
{}

ServerConfig::~ServerConfig(void)
{}

// Operator overloads
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

ServerConfig&	ServerConfig::operator=(const ServerConfig& rhs)
{
	this->_server_names = rhs._server_names;
	this->_host = rhs._host;
	this->_port = rhs._port;
	this->_locations = rhs._locations;
	return (*this);
}

// Getter
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const std::vector<std::string>&	ServerConfig::getServerNames(void) const
{
	return (this->_server_names);
}

const std::string&	ServerConfig::getHost(void) const
{
	return (this->_host);
}

const uint16_t&	ServerConfig::getPort(void) const
{
	return (this->_port);
}

const std::map<std::string, Location*>&	ServerConfig::getLocations(void) const
{
	return (this->_locations);
}

void ServerConfig::setAdress(const std::string& value)
{
	size_t colonPos = value.find(':');

    if (colonPos != std::string::npos)
    {
        _host = value.substr(0, colonPos);
        _port = static_cast<uint16_t>(atoi(value.c_str() + colonPos + 1));
    }
    else
    {
    	_host = "0.0.0.0";
     	_port = static_cast<uint16_t>(atoi(value.c_str()));
    }
}

void ServerConfig::setServerName(const std::string& value)
{
 	std::istringstream iss(value);
    std::string name;
    while (iss >> name) {
        _server_names.push_back(name);
    }
}

void ServerConfig::addLocation(const std::string& path, Location* location)
{
    _locations[path] = location;
}
