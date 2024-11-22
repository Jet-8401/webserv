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
		unsigned int						_max_connections;

	public:
		ServerConfig(void);
		ServerConfig(const ServerConfig& src);
		virtual ~ServerConfig(void);

		ServerConfig&	operator=(const ServerConfig& rhs);

		// Getters
		const std::vector<std::string>&			getServerNames(void) const;
		const std::string&						getHost(void) const;
		const uint16_t&							getPort(void) const;
		std::map<std::string, Location*>&		getLocations(void);
		const unsigned int&						getMaxConnections(void) const;

		// Setters
		void	setAdress(const std::string& value);
        void	setServerName(const std::string& value);
        void	addLocation(const std::string& path, Location* location);
        void	setMaxConnections(const std::string& value);
};

#endif
