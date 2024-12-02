#ifndef STREAMBUFFER_HPP
# define STREAMBUFFER_HPP

# include <cstddef>
# include <cstdio>
# include <stdint.h>

class StreamBuffer {
	private:
		size_t		_allocated_size;
		size_t		_size;
		size_t		_head;
		size_t		_tail;
		uint8_t*	_intern_buffer;

	public:
		StreamBuffer(void); // default size at 0
		StreamBuffer(const size_t buffer_size);
		virtual ~StreamBuffer(void);

		// Getter
		size_t	size(void) const;

		// I/O opearations
		ssize_t	write(void* data, const size_t size);
		ssize_t	consume(void* dest, size_t chunk_size);
};

#endif
