#include "../headers/WebServ.hpp"
#include "../headers/BytesBuffer.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <stdint.h>

# define KILO_BYTES_32 32768
# define MEGA_BYTES_4 4194304

// Constructors / Destructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

BytesBuffer::BytesBuffer(void):
	_file_buff_fd(-1),
	_size(0),
	_buffered_as_file(false),
	_max_bytes_size(KILO_BYTES_32),
	_bytes_threshold(MEGA_BYTES_4)
{
	this->_internal_buff = new uint8_t[this->_max_bytes_size];
	::memset(this->_internal_buff, 0, this->_max_bytes_size);
}

BytesBuffer::BytesBuffer(const size_t max_bytes_size):
	_file_buff_fd(-1),
	_size(0),
	_buffered_as_file(false),
	_max_bytes_size(max_bytes_size),
	_bytes_threshold(MEGA_BYTES_4)
{
	this->_internal_buff = new uint8_t[this->_max_bytes_size];
	::memset(this->_internal_buff, 0, this->_max_bytes_size);
}

BytesBuffer::BytesBuffer(const size_t max_bytes_size, const size_t bytes_threshold):
	_file_buff_fd(-1),
	_size(0),
	_buffered_as_file(false),
	_max_bytes_size(max_bytes_size),
	_bytes_threshold(bytes_threshold)
{
	this->_internal_buff = new uint8_t[this->_max_bytes_size];
	::memset(this->_internal_buff, 0, this->_max_bytes_size);
}

BytesBuffer::~BytesBuffer(void)
{
	if (this->_file_buff_fd != -1)
		close(this->_file_buff_fd);
	if (this->_internal_buff)
		delete [] this->_internal_buff;
}

// Function member
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

// Switch buffering mode and write all existing data from buffer to the tempfile.
int	BytesBuffer::_switchBufferingMode(void)
{
	char	fileName[] = "/tmp/webserv-buffer-XXXXXX";

	(void) this->_bytes_threshold;
	this->_file_buff_fd = mkstemp(fileName);
	if (this->_file_buff_fd == -1)
		return (error(ERR_TMPFILE_CREATION, true), -1);
	if (this->_size == 0)
		return (0);
	if (::write(this->_file_buff_fd, this->_internal_buff, this->_size) == -1)
		return (error(ERR_WRITING_TMPFILE, true), -1);
	this->_buffered_as_file = true;
	delete this->_internal_buff;
	this->_internal_buff = 0;
	return (0);
}

const size_t&	BytesBuffer::size(void) const
{
	return (this->_size);
}

// Return -1 if the data exceed the maxium memory
int	BytesBuffer::write(const uint8_t* data, const size_t size)
{
	if (this->_size + size > this->_max_bytes_size)
		return (-1);
	if (this->_size + size > this->_bytes_threshold)
		this->_switchBufferingMode();
	if (this->_buffered_as_file) {
		if (::write(this->_file_buff_fd, this->_internal_buff, size) == -1)
			return (error(ERR_WRITING_TMPFILE, true),  -1);
		this->_size += size;
		return (0);
	}
	for (size_t i = 0; i < size; i++) {
		this->_internal_buff[this->_size + i] = data[i];
	}
	this->_size += size;
	return (0);
}

uint8_t*	BytesBuffer::read(void) const
{
	return (this->_internal_buff);
}
