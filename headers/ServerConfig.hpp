#ifndef SERVER_CONFIG_HPP
# define SERVER_CONFIG_HPP

# include "Location.hpp"
# include <stdint.h>
# include <string>
# include <map>
# include <vector>

class ServerConfig {
	protected:
		std::vector<std::string>			_server_names;
		std::string							_host;
		uint16_t							_port;
		std::map<std::string, Location*>	_locations;

	public:
		ServerConfig(void);
		ServerConfig(const ServerConfig& src);
		virtual ~ServerConfig(void);

		ServerConfig&	operator=(const ServerConfig& rhs);

		const std::vector<std::string>&			getServerNames(void) const;
		const std::string&						getHost(void) const;
		const uint16_t&							getPort(void) const;
		const std::map<std::string, Location*>&	getLocations(void) const;

		void setAdress(const std::string& value);
        void setServerName(const std::string& value);
        void addLocation(const std::string& path, Location* location);
};

#endif

// print "GET / HTTP/1.1\r\n 10.11.2.6" | nc 10.11.2.6 4243
