#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

class HttpRequest;

# include "Location.hpp"
# include "ServerConfig.hpp"
# include "AHttpMethod.hpp"
# include "HttpMessage.hpp"
# include "BytesBuffer.hpp"
# include "StreamBuffer.hpp"
# include <string>
# include <stdint.h>

class HttpRequest : public HttpMessage {
	public:
		HttpRequest(const ServerConfig& config);
		virtual ~HttpRequest(void);

		typedef enum parsing_state_e {
			READING_HEADERS,
			CHECK_METHOD,
			READING_BODY,
			DONE,
			ERROR
		}	parsing_state_t;

		const std::string&		getPath(void) const;
		const Location&			getMatchingLocation(void) const;
		AHttpMethod*			getExtandedMethod(void) const;

		bool					hasEventsChanged(void) const;
		uint32_t				events;

		bool	parse(const uint8_t* packet, const size_t packet_size);
		parsing_state_e		state;

	protected:
		std::string			_method;
		std::string			_path;
		std::string			_version;

		BytesBuffer			_header_buff;
		StreamBuffer		_body;

		std::string			_config_location_str;
		Location*			_matching_location;

		const ServerConfig&	_config_reference;
		AHttpMethod*		_extanded_method;

	private:
		bool				_has_events_changed;
		size_t				_end_header_index;

		bool				_bufferHeaders(const uint8_t* packet, size_t packet_size);
		bool				_checkHeaderSyntax(const std::string& key, const std::string& value) const;
		bool				_parseHeaders(void);
		bool				_findLocation(void);
		bool				_validateAndInitMethod(void);
};

#endif
