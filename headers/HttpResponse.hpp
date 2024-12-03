#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

# include "HttpRequest.hpp"
# include "Location.hpp"
# include "ServerConfig.hpp"
# include <fcntl.h>
# include <string>
# include <stdint.h>
# include <dirent.h>

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
			DIRECTORY_LISTING	= 0b00001000,
			EXECUTE_CGI			= 0b00010000,
			STATIC_FILE			= 0b00100000
		}	response_action_t;

		// Gettrs
		const bool&			areHeadersParsed(void) const;
		const bool&			areHeadersSent(void) const;
		const bool&			areMediaWrittenToDisk(void) const;
		const bool& 		isDone(void) const;
		const ::uint8_t&	getActionBits(void) const;
		bool				isSendingMedia(void) const;

		int		handleRequest(const ServerConfig& config, const HttpRequest& request);
		void	setHeader(const std::string key, const std::string value);
		int		sendHeaders(const int socket_fd);
		int		sendMedia(const int socket_fd);
		void	setStaticMediaHeaders(void);
		int		handleError(void);
		int		_generateAutoIndex(const int socket_fd, const std::string& uri);
		int		writeMediaToDisk(HttpRequest& request);

		typedef std::map<std::string, std::string> mime_types_t;
		static mime_types_t&	mime_types;

		unsigned short	status_code;

	private:
		void	_buildHeaders(std::stringstream& response) const;
		int		_resolveRequest(const HttpRequest& request);
		int		_resolveLocation(std::string& path, struct stat& file_stats, const std::string& request_location);
		int		_sendStaticFile(const int socket_fd);

		headers_t	_headers;
		bool		_are_headers_sent;
		bool		_are_headers_parsed;

		::uint8_t	_action;
		bool		_is_done;

		std::string _complete_path;
		int			_file_fd;
		DIR*		_dir;
		Location*	_location;
		std::string	_location_string;

		struct stat	_media_stat;
		bool		_are_media_written_to_disk;

		//int			_buffer_fd_in;	// -1 if empty
};

#endif
