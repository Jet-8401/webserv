#ifndef HTTP_CONFING_HPP
# define HTTP_CONFING_HPP

# include <cstdint>
# include <set>
# include <string>
# include <map>

class GlobalConfig {
	protected:
		std::string	_root;
		std::string	_error_pages;
		bool		_autoindex;
		long		_client_max_body_size;
};

class Location : public GlobalConfig {
	protected:
		std::set<std::string>	_methods;
};

class ServerConfig : public GlobalConfig {
	protected:
		std::string							_server_name;
		int									_host;
		uint16_t							_port;
		std::map<std::string, Location&>	_locations;
};

#endif
