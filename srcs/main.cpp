#include "../headers/ServerCluster.hpp"
#include "../headers/WebServ.hpp"
#include <iostream>
#include <iomanip>

void displayErrorPages(const std::map<int, std::string*>& error_pages) {
    if (!error_pages.empty()) {
        std::cout << "\033[1;34m│   │  Error Pages:\033[0m" << std::endl;
        for (std::map<int, std::string*>::const_iterator it = error_pages.begin();
             it != error_pages.end(); ++it) {
            std::cout << "\033[1;34m│   │     └─\033[0m " << it->first
                     << " → " << (it->second ? *it->second : "null") << std::endl;
        }
    }
}

void displayCGIs(const std::map<std::string, std::string>& cgis) {
    if (!cgis.empty()) {
        std::cout << "\033[1;34m│   │  CGI Configurations:\033[0m" << std::endl;
        for (std::map<std::string, std::string>::const_iterator it = cgis.begin();
             it != cgis.end(); ++it) {
            std::cout << "\033[1;34m│   │     └─\033[0m " << it->first
                     << " → " << it->second << std::endl;
        }
    }
}

void displayIndexes(const std::vector<std::string>& indexes) {
    if (!indexes.empty()) {
        std::cout << "\033[1;34m│   │  Indexes:\033[0m ";
        for (std::vector<std::string>::const_iterator it = indexes.begin();
             it != indexes.end(); ++it) {
            std::cout << *it;
            if (it + 1 != indexes.end())
                std::cout << ", ";
        }
        std::cout << std::endl;
    }
}

void displayRedirection(const std::pair<std::string, std::string>& redirection) {
    if (!redirection.first.empty() && !redirection.second.empty()) {
        std::cout << "\033[1;34m│   │  Redirection:\033[0m "
                 << redirection.first << " → " << redirection.second << std::endl;
    }
}

void displayServerInfo(const ServerConfig& config) {
    std::cout << "\033[1;34m│   Port:\033[0m " << config.getPort() << std::endl;
    std::cout << "\033[1;34m│   Host:\033[0m " << config.getHost() << std::endl;

    std::cout << "\033[1;34m│   Server Names:\033[0m ";
    const std::vector<std::string>& names = config.getServerNames();
    for (std::vector<std::string>::const_iterator it = names.begin();
         it != names.end(); ++it) {
        std::cout << *it;
        if (it + 1 != names.end())
            std::cout << ", ";
    }
    std::cout << std::endl;

    std::cout << "\033[1;34m│   Max Connections:\033[0m " << config.getMaxConnections() << std::endl;

    std::cout << "\033[1;34m│   Locations:\033[0m" << std::endl;
    const std::map<std::string, Location*>& locations = config.getLocations();
    for (std::map<std::string, Location*>::const_iterator it = locations.begin();
         it != locations.end(); ++it) {
        std::cout << "\033[1;34m│   ├─ Path:\033[0m " << it->first << std::endl;
        std::cout << "\033[1;34m│   │  Root:\033[0m " << it->second->getRoot() << std::endl;
        std::cout << "\033[1;34m│   │  Alias:\033[0m " << it->second->getAlias() << std::endl;
        std::cout << "\033[1;34m│   │  Autoindex:\033[0m "
                 << (it->second->getAutoIndex() ? "on" : "off") << std::endl;
        std::cout << "\033[1;34m│   │  Max Body Size:\033[0m "
                 << std::setw(10) << it->second->getClientMaxBodySize() << " bytes" << std::endl;

        const std::set<std::string>& methods = it->second->getMethods();
        std::cout << "\033[1;34m│   │  Methods:\033[0m ";
        for (std::set<std::string>::const_iterator mit = methods.begin();
             mit != methods.end(); ++mit) {
            std::cout << *mit;
            std::set<std::string>::const_iterator next = mit;
            ++next;
            if (next != methods.end())
                std::cout << ", ";
        }
        std::cout << std::endl;

        displayIndexes(it->second->getIndexes());
        displayErrorPages(it->second->getErrorPages());
        displayCGIs(it->second->getCGIs());
        displayRedirection(it->second->getRedirection());
        std::cout << "\033[1;34m│   │\033[0m" << std::endl;
    }
    std::cout << "\033[1;34m│\033[0m" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        error(ERR_USAGE, false);
        return 1;
    }

    ServerCluster	cluster;

    std::cout << "\033[1;32m┌── Starting config import from: \033[0m" << argv[1] << std::endl;

    if (cluster.importConfig(argv[1]) == -1) {
        std::cout << "\033[1;31m└── Failed to import config\033[0m" << std::endl;
        return 1;
    }

    std::cout << "\033[1;32m├── Config imported successfully\033[0m" << std::endl;
    std::cout << "\033[1;32m└── Server Configuration Details:\033[0m" << std::endl;

    const std::vector<HttpServer>& servers = cluster.getServers();

    if (servers.empty()) {
        std::cout << "\033[1;31m    No servers configured!\033[0m" << std::endl;
        return 1;
    }

    int count = 1;
    for (std::vector<HttpServer>::const_iterator it = servers.begin();
         it != servers.end(); ++it, ++count) {
        std::cout << "\033[1;33m┌── Server #" << count << "\033[0m" << std::endl;
        displayServerInfo(it->getConfig());
    }

    cluster.run();

    return (0);
}
