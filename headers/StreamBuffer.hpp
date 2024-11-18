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
		uint8_t*	_intern_buffer;	// Work as a circular buffer

	public:
		StreamBuffer(void); // default size at 16Kb
		StreamBuffer(const size_t buffer_size);
		virtual ~StreamBuffer(void);

		// Getter
		size_t	size(void) const;

		// I/O opearations
		ssize_t	write(void* data, const size_t size);
		void*	consume(const size_t chunk_size);
		// Consume until the requested value is found.
		// `value` can be a character and/or a sequence delimted by length.
		void*	consume_until(void* value, const size_t length);
};

#endif
