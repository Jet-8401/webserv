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
    if (argc != 2)
    {
        error(ERR_USAGE, false);
        return 1;
    }

    ServerCluster cluster;

    std::cout << "\033[1;32m┌── Starting config import from: \033[0m" << argv[1] << std::endl;

    if (cluster.importConfig(argv[1]) == -1)
    {
        std::cout << "\033[1;31m└── Failed to import config\033[0m" << std::endl;
        return 1;
    }

    std::cout << "\033[1;32m├── Config imported successfully\033[0m" << std::endl;
    std::cout << "\033[1;32m└── Server Configuration Details:\033[0m" << std::endl;

    // Access the private _servers member using a friend declaration or getter
    const std::map<int, HttpServer>& servers = cluster.getServers(); // You might need to add a getter for this

    if (servers.empty()) {
        std::cout << "\033[1;31m    No servers configured!\033[0m" << std::endl;
        return 1;
    }

    int count = 1;
    for (std::map<int, HttpServer>::const_iterator it = servers.begin();
         it != servers.end(); ++it, ++count) {
        std::cout << "\033[1;33m┌── Server #" << count << "\033[0m" << std::endl;
        displayServerInfo(it->second.getConfig()); // You might need to add a getter for _config
    }

    return 0;
}
