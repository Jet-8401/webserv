#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

# include "HttpRequest.hpp"
# include "Location.hpp"
# include "ServerConfig.hpp"
# include <fcntl.h>
# include <string>
# include <stdint.h>
# include <dirent.h>

// Three options:
// 1. Directory listing
// 2. Sending a body/media/file
// 3. Don't send anything more than headers
//
// Check order:
// - Method
// - Resolve the path based on root, alias, etc...
// - Check for client_max_size_body for static files

class HttpResponse {
	public:
		HttpResponse(void);
		virtual ~HttpResponse(void);

		typedef HttpRequest::headers_t headers_t;
		typedef enum response_action_e {
			NONE				= 0b00000000,
			ACCEPTING_MEDIA		= 0b00000001,
			SENDING_MEDIA		= 0b00000010,
			DELETING_MEDIA		= 0b00000100,
			DIRECTORY_LISTING	= 0b00001000
		}	response_action_t;

		// Gettrs
		const bool&			areHeadersParsed(void) const;
		bool				isSendingMedia(void) const;
		const bool& 		isComplete(void) const;
		const ::uint8_t&	getActionBits(void) const;

		unsigned short	status_code;

		int		handleRequest(const ServerConfig& config, const HttpRequest& request);
		void	setHeader(const std::string key, const std::string value);
		int		sendHeaders(const int socket_fd);
		int		sendBodyPacket(const int socket_fd);

	private:
		void	_buildHeaders(std::stringstream& response) const;
		int		_resolveRequest(const HttpRequest& request);
		int		_resolveLocation(std::string& path, struct stat& file_stats, const std::string& request_location);

		bool		_headers_parsed;
		headers_t	_headers;
		::uint8_t	_action;
		bool		_is_complete;
		int			_file_fd;
		DIR*		_dir;
		Location*	_location;
		//int			_buffer_fd_in;	// -1 if empty
};

#endif
