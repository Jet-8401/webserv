#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

# include "HttpRequest.hpp"
# include "Location.hpp"
# include "ServerConfig.hpp"
# include <string>

// Three options:
// 1. Directory listing
// 2. Sending a body/media/file
// 3. Don't send anything more than headers
//
// First thing to do is to check if the location exist
// If not exist throw a 404, else continue.
// Check if the method is allowed.
// Then check for the final location with alias, root, index, etc...
// If nothing is found, throw a 404, else continue.
// Then check what to do with the method.

class HttpResponse {
	public:
		HttpResponse(void);
		virtual ~HttpResponse(void);

		typedef HttpRequest::headers_t headers_t;

		// Gettrs
		const bool&	areHeadersParsed(void) const;
		const bool	isSendingBody(void) const;
		const bool& isComplete(void) const;

		unsigned short	status_code;

		int		handleRequest(const ServerConfig& config, const HttpRequest& request);
		void	setHeader(const std::string key, const std::string value);
		int		sendHeaders(const int socket_fd);
		int		sendBodyPacket(const int socket_fd);

	private:
		typedef enum response_action_e {
			READING_FILE,
			SENDING_FILE,
			DIRECTORY_LISTING
		}	response_action_t;

		std::string		_resolvePath(const Location& location, const HttpRequest& request);
		void			_buildHeaders(std::stringstream& response) const;
		int				_handleLocation(const Location& location, const HttpRequest& request);

		bool				_headers_parsed;
		headers_t			_headers;
		response_action_t	_response_action;
		bool				_is_complete;
		int					_file_fd;
};

#endif
