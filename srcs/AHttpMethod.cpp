#include "../headers/AHttpMethod.hpp"
#include "../headers/HttpRequest.hpp"

AHttpMethod::AHttpMethod(HttpRequest& referer) : referer(referer) {}

AHttpMethod::~AHttpMethod(void) {}

ssize_t AHttpMethod::writePacket(uint8_t* io_buffer, size_t buff_length)
{
    (void)io_buffer;
    (void)buff_length;
    return 0;
}
