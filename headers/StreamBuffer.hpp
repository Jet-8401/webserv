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
		size_t		_bytes_passed_through;
		size_t		_max_bytes_passed_through;
		uint8_t*	_intern_buffer;

	public:
		StreamBuffer(void);
		StreamBuffer(const size_t buffer_size, const size_t max_bytes_through);
		StreamBuffer(const StreamBuffer& src, bool take_ownership);
		virtual ~StreamBuffer(void);

		void			setMaxBytesThrough(size_t bytes);

		// Getter
		const size_t&	size(void) const;
		const size_t&	allocatedSize(void) const;

		// I/O opearations
		ssize_t	write(const void* data, const size_t size);
		ssize_t	consume(void* dest, size_t chunk_size);
		ssize_t	consume_until(void** dest, const void* key, const size_t key_lenght);
};

#endif
