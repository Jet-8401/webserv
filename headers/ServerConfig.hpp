#ifndef SERVER_CONFIG_HPP
# define SERVER_CONFIG_HPP

# include <cstdint>
# include <set>
# include <string>
# include <map>
# include <vector>

class Location {
	protected:
		std::set<std::string>	_methods;
		std::string				_root;
		std::string				_error_pages;
		bool					_autoindex;
		long					_client_max_body_size;

	public:
		const std::set<std::string>&	getMethods(void) const;
		const std::string&				getRoot(void) const;
		const std::string&				getErrorPages(void) const;
		const bool&						getAutoIndex(void) const;
		const long&						getClientMaxBodySize(void) const;
};

class ServerConfig {
	protected:
		std::vector<std::string>			_server_names;
		int									_host;
		uint16_t							_port;
		std::map<std::string, Location&>	_locations;

	public:

		const std::string&	getServerName(void) const;
		const int&			getHost(void) const;
		const uint16_t&		getPort(void) const;
		const std::map<std::string, Location&>&	getLocations(void) const;
};

#endif
