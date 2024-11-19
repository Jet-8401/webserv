#ifndef BYTES_BUFFER_HPP
# define BYTES_BUFFER_HPP

# include <cstddef>
# include <stdint.h>
# include <stdio.h>

// BytesBuffer is a buffer management class that if the buffered data
// exceed a certain treshold, will change from storing data into the program memory
// to the file system.
class BytesBuffer {
	private:
		uint8_t*		_internal_buff;
		int				_file_buff_fd;			// tmpfile
		size_t			_size;
		bool			_buffered_as_file;
		const size_t	_max_bytes_size;
		const size_t	_bytes_threshold;

		int	_switchBufferingMode(void);

	public:
		BytesBuffer(void); // set max_bytes_size at 32Kb
		BytesBuffer(const size_t max_bytes_size); // set threshold by default at 4Mb
		BytesBuffer(const size_t max_bytes_size, const size_t bytes_threshold);
		virtual ~BytesBuffer(void);

		// Getters
		const size_t&	size(void) const;

		int	write(const uint8_t* data, const size_t size);
		const uint8_t*	read(void) const;
};

// Note: see mmap for convenience and performance

#endif
