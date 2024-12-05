#include "../headers/HttpPostMethod.hpp"

HttpPostMethod::HttpPostMethod(HttpRequest& referer)
    : AHttpMethod(referer), _fild_fd(-1) {}

HttpPostMethod::~HttpPostMethod(void) {}

bool HttpPostMethod::parse(const uint8_t* packet, const size_t packet_size)
{
    (void)packet;
    (void)packet_size;
    return true;
}

ssize_t HttpPostMethod::writePacket(uint8_t* io_buffer, size_t buff_length)
{
    (void)io_buffer;
    (void)buff_length;
    return 0;
}
