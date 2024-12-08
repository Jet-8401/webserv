#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

class HttpResponse;

# include "ParserDefinitions.hpp"
# include "Location.hpp"
# include "ServerConfig.hpp"
# include "HttpMessage.hpp"
# include "BytesBuffer.hpp"
# include "StreamBuffer.hpp"
# include <string>
# include <stdint.h>

class HttpRequest : public HttpMessage {
	public:
		HttpRequest(const ServerConfig& config, const HttpResponse& response);
		HttpRequest(const HttpRequest& src);
		virtual ~HttpRequest(void);

		const std::string&	getMethod(void) const;
		const std::string&	getPath(void) const;
		const Location&		getMatchingLocation(void) const;
		const uint32_t&		getEvents(void);

		void				setEvents(const uint32_t events);

		bool				hasEventsChanged(void) const;

		handler_state_t		bufferHeaders(const uint8_t* packet, size_t packet_len);
		handler_state_t		parseHeaders(void);
		handler_state_t		validateAndInitLocation(void);

	protected:
		std::string			_method;
		std::string			_path;
		std::string			_version;

		BytesBuffer			_header_buff;
		StreamBuffer		_body;

		std::string			_config_location_str;
		Location*			_matching_location;

		const ServerConfig&	_config_reference;
		const HttpResponse&	_response;

		uint32_t			_events;

	private:
		bool				_has_events_changed;
		size_t				_end_header_index;

		bool				_checkHeaderSyntax(const std::string& key, const std::string& value) const;
		bool				_findLocation(void);
};

#endif
