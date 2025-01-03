#ifndef BYTES_BUFFER_HPP
# define BYTES_BUFFER_HPP

# include <cstddef>
# include <stdint.h>
# include <stdio.h>

class BytesBuffer {
	private:
		uint8_t*		_internal_buff;
		size_t			_size;
		const size_t	_max_bytes_size;

	public:
		BytesBuffer(void); // set max_bytes_size at 32KB
		BytesBuffer(const size_t max_bytes_size); // set threshold by default at 4MB
		BytesBuffer(const BytesBuffer& src, const bool takeOwnership);
		virtual ~BytesBuffer(void);

		// Getters
		const size_t&	size(void) const;

		int			write(const uint8_t* data, const size_t size);
		uint8_t*	read(void) const;
};

#endif
