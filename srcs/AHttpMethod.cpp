#include "../headers/AHttpMethod.hpp"
#include "../headers/HttpRequest.hpp"
#include "../headers/HttpResponse.hpp"

AHttpMethod::AHttpMethod(HttpRequest* request)
{
	this->reference._request = request;
}

AHttpMethod::AHttpMethod(HttpResponse* response)
{
	this->reference._response = response;
}

AHttpMethod::~AHttpMethod(void)
{}

ssize_t	AHttpMethod::writePacket(uint8_t* io_buffer, size_t buff_length)
{
	(void) io_buffer;
	(void) buff_length;
	return (0);
}
