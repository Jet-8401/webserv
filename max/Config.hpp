#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <string>
# include <vector>
# include <map>
# include "ConfigValue.hpp"


// Location block configuration
struct LocationConfig
{
    ConfigValue<bool> autoindex;
    ConfigValue<std::string> root;
    ConfigValue<std::string> path;
    ConfigValue<size_t> client_max_body_size;
    ConfigValue<std::vector<std::string> > index_files;
    ConfigValue<std::map<int, std::string> > error_pages;
    ConfigValue<std::vector<std::string> > allowed_methods;

    // Constructor
    LocationConfig();
};

// Server block configuration
struct ServerConfig
{
    ConfigValue<int> port;                    // from listen directive
    ConfigValue<std::string> root;
    ConfigValue<size_t> client_max_body_size;  // path -> location config
    ConfigValue<std::vector<std::string> > index_files;
    ConfigValue<std::vector<std::string> > server_names;
    ConfigValue<std::map<int, std::string> > error_pages;

    std::map<std::string, LocationConfig> locations;

    // Constructor
    ServerConfig();
};

// Http block configuration (top level)
struct HttpConfig
{
    ConfigValue<size_t> client_max_body_size;

    std::vector<ServerConfig> servers;

    // Constructor
    HttpConfig();
};

#endif
