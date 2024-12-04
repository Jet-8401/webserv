#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include "HttpMessage.hpp"
# include "BytesBuffer.hpp"
# include "ServerConfig.hpp"
# include "StreamBuffer.hpp"
# include "Location.hpp"
# include <string>

class HttpRequest;

class AHttpMethod {
	public:
		AHttpMethod(HttpRequest& referer);
		virtual ~AHttpMethod(void);

		virtual bool	parse(const uint8_t* packet, const size_t packet_size) = 0;
		virtual ssize_t	writePacket(uint8_t* io_buffer, size_t buff_length);

	protected:
		HttpRequest&	referer;
};

class HttpPostMethod : public AHttpMethod {
	public:
		HttpPostMethod(void);
		virtual ~HttpPostMethod(void);

	protected:
		int			_fild_fd;
		std::string	_multipart_key;
};

class HttpRequest : public HttpMessage {
	public:
		HttpRequest(const ServerConfig& config);
		virtual ~HttpRequest(void);
		virtual bool	parse(const uint8_t* packet, const size_t packet_size);

		typedef enum parsing_state_e {
			READING_HEADERS,
			CHECK_METHOD,
			READING_BODY,
			DONE,
			ERROR
		}	parsing_state_t;

		const parsing_state_t&	getState(void) const;

	protected:
		std::string			_method;
		std::string			_path;
		std::string			_version;

		BytesBuffer			_header_buff;
		StreamBuffer		_body;
		parsing_state_e		_state;

		const ServerConfig&	_config_reference;
		std::string			_config_location_str;
		Location*			_matching_location;

		AHttpMethod*		_extanded_method;

	private:
		size_t				_end_header_index;

		bool				_bufferHeaders(const uint8_t* packet, size_t packet_size);
		bool				_checkHeaderSyntax(const std::string& key, const std::string& value) const;
		bool				_parseHeaders(void);
		bool				_findLocation(void);
		bool				_validateAndInitMethod(void);
};

#endif
