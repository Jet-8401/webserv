#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

#include "../headers/BytesBuffer.hpp"
#include "../headers/HttpRequest.hpp"
#include "../headers/ServerConfig.hpp"
#include "Location.hpp"
#include <string>
#include <map>

class HttpResponse {
	private:
		std::multimap<std::string, std::string>	_headers;
		BytesBuffer								_content;
		bool									_is_sent;
		Location*								_current_location;
		void	_buildHeaders(std::stringstream& response) const;
		bool	_sendAll(const int socket_fd, const std::string& data) const;

public:
	HttpResponse(void);
	virtual ~HttpResponse(void);

	void	setHeader(const std::string& key, const std::string& value);
	int		send(const int socket_fd);
	bool	isSent(void) const;
	bool	handleRequest(const ServerConfig& conf, const HttpRequest& request);
	Location*	findMatchingLocation(const std::string& path, const std::map<std::string, Location*>& locations);
};

#endif
