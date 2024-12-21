#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

class HttpResponse;
class Socket;

# include "Location.hpp"
# include "HttpMessage.hpp"
# include "BytesBuffer.hpp"
# include "StreamBuffer.hpp"
# include <string>
# include <stdint.h>
# include <fcntl.h>

class HttpRequest : public HttpMessage {
	public:
		HttpRequest(const HttpResponse& response, const Socket& socket_referer);
		HttpRequest(const HttpRequest& src);
		virtual ~HttpRequest(void);

		const std::string&	getMethod(void) const;
		const std::string&	getPath(void) const;
		const Location&		getMatchingLocation(void) const;
		const bool&			hasEventsChanged(void) const;
		const uint32_t&		getEvents(void);
		const std::string&	getResolvedPath(void) const;
		const std::string&	getConfigLocationStr(void) const;
		StreamBuffer&		getBody(void);
		const struct stat&	getPathStat(void) const;

		void				setEvents(const uint32_t events);

		handler_state_t		bufferHeaders(const uint8_t* packet, size_t packet_len);
		handler_state_t		parseHeaders(void);
		handler_state_t		validateAndInitLocation(void);

		const static uint8_t	END_SEQUENCE[4];

	protected:
		std::string			_method;
		std::string			_path;
		std::string			_version;

		BytesBuffer			_header_buff;
		StreamBuffer		_body;

		std::string			_resolved_path;
		std::string			_config_location_str;
		Location*			_matching_location;
		struct stat			_path_stat;

		const HttpResponse&	_response;
		const Socket&		_socket_referer;

		uint32_t			_events;

	private:
		bool				_has_events_changed;
		size_t				_end_header_index;

		bool				_checkHeaderSyntax(const std::string& key, const std::string& value) const;
		bool				_findLocation(void);
		bool				_resolveLocation(void);
};

#endif
