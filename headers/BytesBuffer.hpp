#ifndef BYTES_BUFFER_HPP
# define BYTES_BUFFER_HPP

# include <cstddef>
# include <vector>
# include <stdint.h>
# include <stdio.h>
# include <stdint.h>

// BytesBuffer is a buffer management class that if the buffered data
// exceed a certain treshold, will change from storing data into the program memory
// to the file system.
class BytesBuffer {
	private:
		std::vector<uint8_t>	_internal_buff;
		FILE*					_file_buff;
		bool					_buffered_as_file;
		const size_t			_bytes_threshold;
		const size_t			_max_bytes_size;

	public:
		BytesBuffer(void); // set max_bytes_size at 8Kb
		BytesBuffer(const size_t max_bytes_size); // set threshold by default at 4Mb
		BytesBuffer(const size_t max_bytes_size, const size_t bytes_threshold);
		virtual ~BytesBuffer(void);
};

// Note: see mmap for convenience and performance

#endif
