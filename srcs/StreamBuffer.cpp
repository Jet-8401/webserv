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

StreamBuffer::StreamBuffer(const StreamBuffer& src, bool takeOwnership):
	_allocated_size(src._allocated_size),
	_size(src._size),
	_head(src._head),
	_tail(src._tail)
{
	if (takeOwnership) {
		this->_intern_buffer = src._intern_buffer;

		const_cast<StreamBuffer&>(src)._intern_buffer = 0;
		const_cast<StreamBuffer&>(src)._size = 0;
		const_cast<StreamBuffer&>(src)._head = 0;
		const_cast<StreamBuffer&>(src)._tail = 0;
	} else {
		this->_intern_buffer = new uint8_t[this->_allocated_size];
		::memcpy(this->_intern_buffer, src._intern_buffer, this->_size);
	}
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

ssize_t StreamBuffer::write(const void* data, const size_t size)
{
	if (size > this->_allocated_size - this->_size)
		return (-1);

	const uint8_t* src = static_cast<const uint8_t*>(data);
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
// [l] [r] [d]	 [H] [e] [l] [l] [o] [ ] [W] [o]
//	  ^tail   ^head
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

ssize_t	StreamBuffer::consume_until(void* dest, void* key, size_t key_length)
{
	if (!dest || !key || key_length == 0 || _size == 0)
		return -1;

	unsigned char* dest_ptr = reinterpret_cast<unsigned char*>(dest);
	unsigned char* key_ptr = reinterpret_cast<unsigned char*>(key);
	size_t pos = _head;

	for (size_t remaining = _size;remaining >= key_length;pos = (pos + 1) % _allocated_size, remaining--) {
		if (_intern_buffer[pos] == key_ptr[0])
			continue;
		size_t i;
		for (i = 1; i < key_length; i++) {
			if (_intern_buffer[(pos + i) % _allocated_size] != key_ptr[i])
				break;
		}
		if (i == key_length) {
			// Key found - copy data up to key position
			size_t bytes_to_copy = (pos - _head + _allocated_size) % _allocated_size;
			for (i = 0; i < bytes_to_copy; i++)
				dest_ptr[i] = _intern_buffer[(_head + i) % _allocated_size];
			_head = (pos + key_length) % _allocated_size;
			_size -= (bytes_to_copy + key_length);
			return bytes_to_copy;
		}
	}
	return -1;
}
