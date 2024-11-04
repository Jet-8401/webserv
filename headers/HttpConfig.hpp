#ifndef HTTP_CONFING_HPP
# define HTTP_CONFING_HPP

# include <cstdint>
# include <string>

class HttpConfig {
	private:
		static HttpConfig	_global;	// global configuration

        std::string	_server_name;
		std::string	_host;				// listen address
        std::string	_default_root;
        std::string	_root_error;
        uint16_t	_port;

	public:
		HttpConfig(void);
		HttpConfig(const HttpConfig& src);

		HttpConfig&	operator=(const HttpConfig& rhs);

		virtual	~HttpConfig(void);
};

#endif
