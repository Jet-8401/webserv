#include "../headers/ServerCluster.hpp"
#include <cstdio>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include "../headers/WebServ.hpp"

// Constructors / Destructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

std::map<std::string, void (ServerConfig::*)(const std::string&)> ServerCluster::serverSetters;
std::map<std::string, void (Location::*)(const std::string&)> ServerCluster::locationSetters;

void ServerCluster::initDirectives()
{
    serverSetters["listen"] = &ServerConfig::setPort;
    serverSetters["host"] = &ServerConfig::setHost;
    serverSetters["server_name"] = &ServerConfig::setServerName;
    serverSetters["index"] = &ServerConfig::setIndex;
    serverSetters["root"] = &ServerConfig::setRoot;
    serverSetters["client_max_body_size"] = &ServerConfig::setClientMaxBodySize;
    serverSetters["error_page"] = &ServerConfig::setErrorPage;

    locationSetters["autoindex"] = &Location::setAutoindex;
    locationSetters["methods"] = &Location::setMethods;
    locationSetters["root"] = &Location::setRoot;
    locationSetters["error_page"] = &Location::setErrorPage;
    locationSetters["client_max_body_size"] = &Location::setClientMaxBodySize;
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
    FILE* file = fopen(config_path.c_str(), "r");
    if (!file)
        return (error(ERR_FILE_OPEN + config_path, true), -1);

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char* buffer = new char[size + 1];
    fread(buffer, 1, size, file);
    buffer[size] = '\0';
    fclose(file);

    std::istringstream iss(buffer);
    delete[] buffer;

    std::string token;
    while (iss >> token)
    {
        if (token == "http")
        {
            if (parseHttpBlock(iss) < 0)
                return (-1);
        }
    }
    return (0);
}

int ServerCluster::parseHttpBlock(std::istringstream& iss)
{
    std::string token;
    iss >> token;
    if (token != "{")
        return (error("Expected '{' after http", true), -1);

    while (iss >> token)
    {
        if (token == "}")
            return (0);
        else if (token == "server")
        {
            ServerConfig config;
            if (parseServerBlock(iss, config) < 0)
                return (-1);
        }
    }
    return (error("Unexpected end of http block", true), -1);
}

int ServerCluster::parseServerBlock(std::istringstream& iss, ServerConfig& config)
{
    std::string token;
    iss >> token;
    if (token != "{")
        return (error("Expected '{' after server", true), -1);

    std::cout << "Parsing server block..." << std::endl; // Debug

    while (iss >> token)
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
	        std::getline(iss, paths);
            std::cout << "Parsing location: " << paths  << "|" << std::endl; // Debug
            Location *location = new Location();
            if (parseLocationBlock(iss, location) < 0)
                return (-1);
            std::istringstream iss_paths(paths);
            while (iss_paths >> paths)
            {
                config.addLocation(paths, location);
            }
        }
        else
        {
            std::map<std::string, void (ServerConfig::*)(const std::string&)>::iterator it = serverSetters.find(token);
            if (it != serverSetters.end())
            {
                std::string value;
                std::getline(iss, value, ';'); // Read until semicolon
                value = value.substr(value.find_first_not_of(" \t")); // Trim leading whitespace
                //std::cout << "Setting " << token << " to " << value << std::endl; // Debug
                (config.*(it->second))(value);
            }
        }
    }
    return (error("Unexpected end of server block", true), -1);
}

int ServerCluster::parseLocationBlock(std::istringstream& iss, Location* location)
{
    std::string token;
    iss >> token;
    std::cout << "LE token : " << token << std::endl;
    if (token != "{")
        return (error("Expected '{' after location", true), -1);

    while (iss >> token)
    {
        if (token == "}")
            return (0);
        else
        {
            std::map<std::string, void (Location::*)(const std::string&)>::iterator it = locationSetters.find(token);
            if (it != locationSetters.end())
            {
                std::string value;
                std::getline(iss, value, ';'); // Read until semicolon
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
