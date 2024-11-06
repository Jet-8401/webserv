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

		void setPort(const std::string& value);
        void setHost(const std::string& value);
        void setServerName(const std::string& value);
        void setIndex(const std::string& value);
        void setRoot(const std::string& value);
        void setClientMaxBodySize(const std::string& value);
        void setErrorPage(const std::string& value);
        void addLocation(const std::string& path, const Location& location);
};

#endif

// print "GET / HTTP/1.1\r\n 10.11.2.6" | nc 10.11.2.6 4243
