#include <iostream>
#include <vector>
#include <map>
#include "Parser.hpp"

void printLocation(const LocationConfig& loc) {
    std::cout << "\n    Location: " << loc.path.getValue() << std::endl;
    if (loc.autoindex.isSet())
        std::cout << "      autoindex: " << (loc.autoindex.getValue() ? "on" : "off") << std::endl;
    if (loc.root.isSet())
        std::cout << "      root: " << loc.root.getValue() << std::endl;
    if (loc.allowed_methods.isSet()) {
        std::cout << "      allowed methods: ";
        const std::vector<std::string>& methods = loc.allowed_methods.getValue();
        for (size_t i = 0; i < methods.size(); ++i)
            std::cout << methods[i] << " ";
        std::cout << std::endl;
    }
    // Add error_pages printing
    if (loc.error_pages.isSet()) {
        std::cout << "      error pages:" << std::endl;
        const std::map<int, std::string>& errors = loc.error_pages.getValue();
        std::map<int, std::string>::const_iterator it;
        for (it = errors.begin(); it != errors.end(); ++it) {
            std::cout << "        " << it->first << " -> " << it->second << std::endl;
        }
    }
}

void printServer(const ServerConfig& server) {
    std::cout << "\n  Server:" << std::endl;
    if (server.port.isSet())
        std::cout << "    listen: " << server.port.getValue() << std::endl;
    if (server.root.isSet())
        std::cout << "    root: " << server.root.getValue() << std::endl;
    if (server.server_names.isSet()) {
        std::cout << "    server_names: ";
        const std::vector<std::string>& names = server.server_names.getValue();
        for (size_t i = 0; i < names.size(); ++i)
            std::cout << names[i] << " ";
        std::cout << std::endl;
    }
    // Add error_pages printing
    if (server.error_pages.isSet()) {
        std::cout << "    error pages:" << std::endl;
        const std::map<int, std::string>& errors = server.error_pages.getValue();
        std::map<int, std::string>::const_iterator it;
        for (it = errors.begin(); it != errors.end(); ++it) {
            std::cout << "      " << it->first << " -> " << it->second << std::endl;
        }
    }

    // Print locations
    std::map<std::string,LocationConfig>::const_iterator it;
    for (it = server.locations.begin(); it != server.locations.end(); ++it) {
        printLocation(it->second);
    }
}

void printConfig(const HttpConfig& config) {
    std::cout << "HTTP Configuration:" << std::endl;
    if (config.client_max_body_size.isSet())
        std::cout << "  client_max_body_size: " << config.client_max_body_size.getValue() << std::endl;

    // Print each server
    for (size_t i = 0; i < config.servers.size(); ++i) {
        printServer(config.servers[i]);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }

    try {
        Parser parser(argv[1]);
        HttpConfig config = parser.parseConfig();
        printConfig(config);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
