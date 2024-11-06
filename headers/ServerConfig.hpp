#ifndef SERVER_CONFIG_HPP
# define SERVER_CONFIG_HPP

# include "Location.hpp"
# include <cstdint>
# include <string>
# include <map>
# include <vector>

class ServerConfig {
	protected:
		std::vector<std::string>			_server_names;
		int									_host;
		uint16_t							_port;
		std::map<std::string, Location&>	_locations;

	public:
		ServerConfig(void);
		ServerConfig(const ServerConfig& src);
		virtual ~ServerConfig(void);

		ServerConfig&	operator=(const ServerConfig& rhs);

		const std::vector<std::string>&			getServerNames(void) const;
		const int&								getHost(void) const;
		const uint16_t&							getPort(void) const;
		const std::map<std::string, Location&>&	getLocations(void) const;
};

#endif

// print "GET / HTTP/1.1\r\n 10.11.2.6" | nc 10.11.2.6 4243
