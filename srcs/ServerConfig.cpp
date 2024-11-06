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

void ServerConfig::setPort(const std::string& value)
{
    _port = static_cast<uint16_t>(atoi(value.c_str()));
}

void ServerConfig::setHost(const std::string& value)
{
    _host = inet_addr(value.c_str());
}

void ServerConfig::setServerName(const std::string& value)
{
    _server_names.push_back(value);
}

void ServerConfig::setIndex(const std::string& value)
{
    // Implement based on your needs
    (void)value;
}

void ServerConfig::setRoot(const std::string& value)
{
    // Implement based on your needs
    (void)value;
}

void ServerConfig::setClientMaxBodySize(const std::string& value)
{
    // Parse size with unit (M, K, etc)
    std::string size = value;
    long multiplier = 1;
    if (size.back() == 'M' || size.back() == 'm')
    {
        multiplier = 1024 * 1024;
        size.erase(size.size() - 1);
    }
    else if (size.back() == 'K' || size.back() == 'k')
    {
        multiplier = 1024;
        size.erase(size.size() - 1);
    }
    // Set the value in your class (you'll need to add this member)
    // _client_max_body_size = atol(size.c_str()) * multiplier;
}

void ServerConfig::setErrorPage(const std::string& value)
{
    // Implement based on your needs
    (void)value;
}

void ServerConfig::addLocation(const std::string& path, const Location& location)
{
    _locations[path] = location;
}
