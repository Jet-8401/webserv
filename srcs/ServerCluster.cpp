#include "../headers/ServerCluster.hpp"
#include <cstdio>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include "../headers/WebServ.hpp"

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

ServerCluster::ServerCluster(void) : _epoll_fd(-1)
{
    initDirectives();
}

ServerCluster::~ServerCluster(void) {}

const ServerCluster::servers_type_t& ServerCluster::getServers() const
{
	return _servers;
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
        if (token == "http")
        {
            if (parseHttpBlock(ss) < 0)
                return (-1);
        }
    }
    return (0);
}

void ServerCluster::parseHttpBlockDefault(std::stringstream& original_ss, Location* http_location)
{
	std::stringstream ss(original_ss.str());
	std::string token;

	while (ss >> token)
	{
		if (token == "}")
		{
			return;
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
				std::string value;
				std::getline(ss, value, ';'); // Read until semicolon
				value = value.substr(value.find_first_not_of(" \t")); // Trim leading whitespace
				(http_location->*(it->second))(value);
			}
		}
	}
}

int ServerCluster::parseHttpBlock(std::stringstream& ss)
{
    std::string token;
    ss >> token;
    if (token != "{")
        return (error("Expected '{' after http", true), -1);


    while (ss >> token)
    {
        if (token == "}")
            return (0);
        else if (token == "server")
        {
            ServerConfig config;
            Location http_location;
            if (parseServerBlock(ss, config, &http_location) < 0)
                return (-1);
        }
    }
    return (error("Unexpected end of http block", true), -1);
}

void ServerCluster::parseServerBlockDefault(std::stringstream& original_ss, Location* serv_location)
{
	std::stringstream ss(original_ss.str());
	std::string token;
	while (ss >> token)
	{
		if (token == "}")
		{
			return;
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
				std::getline(ss, value, ';'); // Read until semicolon
				value = value.substr(value.find_first_not_of(" \t")); // Trim leading whitespace
				(serv_location->*(it->second))(value);
			}
		}
	}
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

int	ServerCluster::listenAll(void) const {

	servers_type_t::const_iterator it;

    // create epoll

	for (it = this->_servers.begin(); it != this->_servers.end(); it++)
		if (it->listen() == -1)
			return (-1);
	return (0);

}
