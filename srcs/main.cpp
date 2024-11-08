#include "../headers/WebServ.hpp"
#include "../headers/ServerCluster.hpp"
#include <iostream>
#include <iomanip>

void displayServerInfo(const ServerConfig& config) {
    std::cout << "\033[1;34m│   Port:\033[0m " << config.getPort() << std::endl;
    std::cout << "\033[1;34m│   Host:\033[0m " << config.getHost() << std::endl;

    std::cout << "\033[1;34m│   Server Names:\033[0m ";
    const std::vector<std::string>& names = config.getServerNames();
    std::vector<std::string>::const_iterator it = names.begin();
    std::cout << names.size() << std::endl;
    while (it != names.end())
    {
        std::cout << *it;
    	it++;
    }
    std::cout << std::endl;

    std::cout << "\033[1;34m│   Locations:\033[0m" << std::endl;
    const std::map<std::string, Location>& locations = config.getLocations();
    for (std::map<std::string, Location>::const_iterator it = locations.begin();
         it != locations.end(); ++it) {
        std::cout << "\033[1;34m│   ├─ Path:\033[0m " << it->first << std::endl;
        std::cout << "\033[1;34m│   │  Root:\033[0m " << it->second.getRoot() << std::endl;
        std::cout << "\033[1;34m│   │  Autoindex:\033[0m " << (it->second.getAutoIndex() ? "on" : "off") << std::endl;
        std::cout << "\033[1;34m│   │  Max Body Size:\033[0m " << it->second.getClientMaxBodySize() << std::endl;

        const std::set<std::string>& methods = it->second.getMethods();
        std::cout << "\033[1;34m│   │  Methods:\033[0m ";
        for (std::set<std::string>::const_iterator mit = methods.begin();
             mit != methods.end(); ++mit) {
            std::cout << *mit << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "\033[1;34m│\033[0m" << std::endl;
}

int main(int argc, char* argv[])
{
	(void) argc;
	(void) argv;

	ServerConfig	configuration;
	configuration.setHost("127.0.0.1");
	configuration.setPort("5500");
	configuration.setServerName("jullopez.42.fr");

	HttpServer	server(configuration);

	server.listen();

    return 0;
}
