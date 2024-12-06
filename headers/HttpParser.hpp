#ifndef HTTP_PARSER_HPP
# define HTTP_PARSER_HPP

# include "ParserDefinitions.hpp"
# include "HttpResponse.hpp"
# include "HttpRequest.hpp"
# include <stdint.h>
# include <sys/types.h>

class HttpParser {
	private:
		bool			_need_upgrade;

	protected:
		HttpParser(const HttpParser& parser);

		HttpRequest		_request;
		HttpResponse	_response;

	public:
		HttpParser(const ServerConfig& config);
		virtual ~HttpParser(void);

		parsing_state_t	state;

		virtual bool	parse(const uint8_t* packet, const size_t packet_len);
		virtual ssize_t	write(const uint8_t* io_buffer, const size_t buff_len);

		HttpResponse&	getResponse(void);
		HttpRequest&	getRequest(void);
		const bool&		checkUpgrade(void) const;

		HttpParser*		upgrade(void);

		void			handleRedirection(void);
		void			handleError(void);
};

#endif
