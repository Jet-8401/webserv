#include "../headers/StreamBuffer.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

# define DEFAULT_CHUNK_BYTES_SIZE 16384

// Constructors / Desctrucors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

StreamBuffer::StreamBuffer(void):
	_allocated_size(DEFAULT_CHUNK_BYTES_SIZE),
	_size(0),
	_head(0),
	_tail(0)
{
	this->_intern_buffer = new uint8_t[this->_allocated_size];
}

StreamBuffer::StreamBuffer(const size_t buffer_size):
	_allocated_size(buffer_size),
	_size(0),
	_head(0),
	_tail(0)
{
	this->_intern_buffer = new uint8_t[this->_allocated_size];
}

StreamBuffer::~StreamBuffer(void)
{
	if (this->_intern_buffer)
		delete this->_intern_buffer;
}

// Getter
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

size_t	StreamBuffer::size(void) const
{
	return (this->_size);
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

ssize_t	StreamBuffer::write(void* data, const size_t size)
{
	uint8_t*	src;
	size_t		bytes_until_end;

	if (this->_size + size > this->_allocated_size)
		return (-1);

	src = static_cast<uint8_t*>(data);
	bytes_until_end = this->_allocated_size - this->_tail;

	if (size > bytes_until_end) {
		::memcpy(this->_intern_buffer + this->_tail, src, bytes_until_end);
		this->_tail = size - bytes_until_end;
		::memcpy(this->_intern_buffer, src + bytes_until_end, this->_tail);
	} else {
		::memcpy(this->_intern_buffer, src + this->_tail, size);
		this->_tail = size;
	}
	this->_size += size;
	return (size);
}
