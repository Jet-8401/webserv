#ifndef AHTTP_METHOD_HPP
# define AHTTP_METHOD_HPP

# include <stdint.h>
# include <sys/types.h>

class AHttpMethod {
	public:
		AHttpMethod(void);
		virtual ~AHttpMethod(void);

		virtual bool	parse(const uint8_t* packet, const size_t packet_size) = 0;
		virtual ssize_t	writePacket(uint8_t* io_buffer, size_t buff_length);
};


#endif
