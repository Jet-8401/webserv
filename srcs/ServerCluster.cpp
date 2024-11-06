#include "../headers/ServerCluster.hpp"
#include <cstdio>
#include <unistd.h>
#include <sstream>
#include "../headers/WebServ.hpp"

// Constructors / Destructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

std::map<std::string, void (ServerConfig::*)(const std::string&)> ServerCluster::serverDirectives;
std::map<std::string, void (Location::*)(const std::string&)> ServerCluster::locationDirectives;

void ServerCluster::initDirectives()
{
    serverDirectives["listen"] = &ServerConfig::setPort;
    serverDirectives["host"] = &ServerConfig::setHost;
    serverDirectives["server_name"] = &ServerConfig::setServerName;
    serverDirectives["index"] = &ServerConfig::setIndex;
    serverDirectives["root"] = &ServerConfig::setRoot;
    serverDirectives["client_max_body_size"] = &ServerConfig::setClientMaxBodySize;
    serverDirectives["error_page"] = &ServerConfig::setErrorPage;

    locationDirectives["autoindex"] = &Location::setAutoindex;
    locationDirectives["methods"] = &Location::setMethods;
    locationDirectives["root"] = &Location::setRoot;
    locationDirectives["error_page"] = &Location::setErrorPage;
    locationDirectives["client_max_body_size"] = &Location::setClientMaxBodySize;
}

ServerCluster::ServerCluster(void) : _epoll_fd(-1)
{
    initDirectives();
}

ServerCluster::~ServerCluster(void) {}

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

    while (iss >> token)
    {
        if (token == "}")
        {
            int* sock_fd = new int(-1);
            _servers[*sock_fd] = HttpServer(config);
            return (0);
        }
        else if (token == "location")
        {
            std::string path;
            iss >> path;
            Location location;
            if (parseLocationBlock(iss, location) < 0)
                return (-1);
            config.addLocation(path, location);
        }
        else
        {
            std::map<std::string, void (ServerConfig::*)(const std::string&)>::iterator it = serverDirectives.find(token);
            if (it != serverDirectives.end())
            {
                std::string value;
                iss >> value;
                if (value.back() == ';')
                    value.erase(value.size() - 1);
                (config.*(it->second))(value);
            }
        }
    }
    return (error("Unexpected end of server block", true), -1);
}

int ServerCluster::parseLocationBlock(std::istringstream& iss, Location& location)
{
    std::string token;
    iss >> token;
    if (token != "{")
        return (error("Expected '{' after location", true), -1);

    while (iss >> token)
    {
        if (token == "}")
            return (0);
        else
        {
            std::map<std::string, void (Location::*)(const std::string&)>::iterator it = locationDirectives.find(token);
            if (it != locationDirectives.end())
            {
                std::string value;
                iss >> value;
                if (value.back() == ';')
                    value.erase(value.size() - 1);
                (location.*(it->second))(value);
            }
        }
    }
    return (error("Unexpected end of location block", true), -1);
}

int	ServerCluster::listenAll(void) const {

	servers_type_t::const_iterator it;

    // create epoll

	for (it = this->_servers.begin(); it != this->_servers.end(); it++)
		if ((*it).second.listen() == -1)
			return (-1);
	return (0);

}
