#include "../headers/StreamBuffer.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

# define DEFAULT_CHUNK_BYTES_SIZE 16000

// Constructors / Desctrucors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

StreamBuffer::StreamBuffer(void):
	_allocated_size(0),
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
		delete [] this->_intern_buffer;
}

// Getter
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

size_t	StreamBuffer::size(void) const
{
	return (this->_size);
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

ssize_t StreamBuffer::write(void* data, const size_t size)
{
	if (size > this->_allocated_size - this->_size)
		return (-1);

	uint8_t* src = static_cast<uint8_t*>(data);
	size_t bytes_until_end = this->_allocated_size - this->_tail;

	if (size > bytes_until_end) {
		::memcpy(this->_intern_buffer + this->_tail, src, bytes_until_end);
		::memcpy(this->_intern_buffer, src + bytes_until_end, size - bytes_until_end);
		this->_tail = (size - bytes_until_end) % this->_allocated_size;
	} else {
		::memcpy(this->_intern_buffer + this->_tail, src, size);
		this->_tail = (this->_tail + size) % this->_allocated_size;
	}

	this->_size += size;
	return size;
}

// Might return less that chunk_size
// [l] [r] [d]     [H] [e] [l] [l] [o] [ ] [W] [o]
//          ^tail   ^head
ssize_t	StreamBuffer::consume(void* dest, size_t chunk_size)
{
	size_t	bytes_until_end;
	size_t	bytes_copied;

	if (!dest)
		return (-1);
	if (chunk_size == 0 || this->_size == 0)
		return (0);

	chunk_size = std::min(chunk_size, this->_size);
    bytes_until_end = this->_allocated_size - this->_head;

    if (chunk_size > bytes_until_end) {
        ::memcpy(dest, this->_intern_buffer + this->_head, bytes_until_end);
        ::memcpy(((uint8_t*)dest) + bytes_until_end, this->_intern_buffer,
                 chunk_size - bytes_until_end);
        this->_head = (chunk_size - bytes_until_end) % this->_allocated_size;
        bytes_copied = chunk_size;
    } else {
        ::memcpy(dest, this->_intern_buffer + this->_head, chunk_size);
        this->_head = (this->_head + chunk_size) % this->_allocated_size;
        bytes_copied = chunk_size;
    }

    this->_size -= bytes_copied;
    return bytes_copied;
}

/*
// Note: don't handle circular buffers
void*	StreamBuffer::consume_until(void* value, const size_t length)
{
	void*	chunk;

	if (this->_size < length)
		return (0);
	for (size_t i = 0; i < this->_size; i++) {
		if (::memcmp(this->_intern_buffer + this->_head + i, value, length) != 0)
			continue;
		chunk = new uint8_t[i + length];
		::memcpy(chunk, this->_intern_buffer + this->_head, i + length);
		this->_head += i + length;
		return (chunk);
	}
	return (0);
}
*/
