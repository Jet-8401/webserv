#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

# include "HttpRequest.hpp"
# include "ServerConfig.hpp"

class HttpResponse {
	private:
		bool	_is_ready;

		ServerConfig::locations_t::const_iterator	_matchLocation(
			const ServerConfig::locations_t& server_locations,
			const std::string& request_location
		) const;

	public:
		HttpResponse(void);
		virtual ~HttpResponse(void);

		// Getters
		const bool&	isReady(void) const;

		unsigned short	status_code;

		int	handleRequest(const ServerConfig& config, const HttpRequest& request);
		int	send(const int socket_fd);
};

#endif
