#include "../headers/EventWrapper.hpp"
#include <cstddef>

// Constructors / Destructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

EventWrapper::EventWrapper(void): _events()
{}

EventWrapper::~EventWrapper(void)
{
	std::list<event_wrapper_t*>::iterator	it;

	for (it = this->_events.begin(); it != this->_events.end(); it++) {
		delete *it;
		*it = NULL;
	}
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

event_wrapper_t* EventWrapper::create(enum socket_type_e socket_type)
{
	event_wrapper_t*	wrapper;

	wrapper = new event_wrapper_t;
	wrapper->socket_type = socket_type;
	this->_events.push_back(wrapper);
	return (wrapper);
}

void	EventWrapper::remove(event_wrapper_t* event)
{
	this->_events.remove(event);
	delete event;
}
