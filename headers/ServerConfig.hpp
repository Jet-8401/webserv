#ifndef SERVER_CONFIG_HPP
# define SERVER_CONFIG_HPP

# include "Location.hpp"
# include <stdint.h>
# include <string>
# include <map>
# include <vector>

class ServerConfig {
	public:
		ServerConfig(void);
		ServerConfig(const ServerConfig& src);
		virtual ~ServerConfig(void);

		ServerConfig&	operator=(const ServerConfig& rhs);

		typedef std::map<std::string, Location*> locations_t;

		// Getters
		const std::vector<std::string>&	getServerNames(void) const;
		const std::string&				getHost(void) const;
		const uint16_t&					getPort(void) const;
		const locations_t&				getLocations(void) const;
		const unsigned int&				getMaxConnections(void) const;

		// Setters
		void	setAdress(const std::string& value);
	void	setServerName(const std::string& value);
	void	addLocation(const std::string& path, Location* location);
	void	setMaxConnections(const std::string& value);

	protected:
		std::vector<std::string>	_server_names;
		std::string					_host;
		uint16_t					_port;
		locations_t					_locations;
		unsigned int				_max_connections;
};

#endif
