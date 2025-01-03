#include "../headers/BytesBuffer.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>

# define KILO_BYTES_32 32000
# define MEGA_BYTES_4 4000000

// Constructors / Destructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

BytesBuffer::BytesBuffer(void):
	_size(0),
	_max_bytes_size(KILO_BYTES_32)
{
	this->_internal_buff = new uint8_t[this->_max_bytes_size];
	::memset(this->_internal_buff, 0, this->_max_bytes_size);
}

BytesBuffer::BytesBuffer(const size_t max_bytes_size):
	_size(0),
	_max_bytes_size(max_bytes_size)
{
	this->_internal_buff = new uint8_t[this->_max_bytes_size];
	::memset(this->_internal_buff, 0, this->_max_bytes_size);
}

BytesBuffer::BytesBuffer(const BytesBuffer& src, const bool takeOwnership):
	_size(src._size),
	_max_bytes_size(src._max_bytes_size)
{
	if (takeOwnership) {
		this->_internal_buff = src._internal_buff;

		// Clear the source's pointers/FD (since we're taking ownership)
		const_cast<BytesBuffer&>(src)._internal_buff = 0;
		const_cast<BytesBuffer&>(src)._size = 0;
	}  else {
		this->_internal_buff = new uint8_t[this->_max_bytes_size];
		::memcpy(this->_internal_buff, src._internal_buff, _size);
	}
}

BytesBuffer::~BytesBuffer(void)
{
	if (this->_internal_buff)
		delete [] this->_internal_buff;
}

// Function member
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const size_t&	BytesBuffer::size(void) const
{
	return (this->_size);
}

// Return -1 if the data exceed the maxium memory
int	BytesBuffer::write(const uint8_t* data, const size_t size)
{
	if (this->_size + size > this->_max_bytes_size)
		return (-1);
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
