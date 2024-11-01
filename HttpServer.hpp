#ifndef HTTP_SERVER
# define HTTP_SERVER

# include "HttpResponse.hpp"
# include "HttpRequest.hpp"
# include <sstream>

class HttpServer {
	private:
		HttpServer(void);

		HttpServer&	operator=(const HttpServer& src);

	public:
		typedef class HttpResponse Response;
		typedef class HttpRequest Request;

		HttpServer(std::stringstream& config); // or ifstream
		virtual ~HttpServer(void);
};

#endif
