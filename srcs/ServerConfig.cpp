#include "../headers/ServerConfig.hpp"

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

ServerConfig::ServerConfig(void):
	_host(0),
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

const int&	ServerConfig::getHost(void) const
{
	return (this->_host);
}

const uint16_t&	ServerConfig::getPort(void) const
{
	return (this->_port);
}

const std::map<std::string, Location&>&	ServerConfig::getLocations(void) const
{
	return (this->_locations);
}
