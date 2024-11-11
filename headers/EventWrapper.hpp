#ifndef EVENT_WRAPPER_HPP
# define EVENT_WRAPPER_HPP

# include <list>

enum socket_type_e {
	REQUEST,
	CLIENT
};

typedef struct event_wrapper_s {
	void*	casted_value;
	enum socket_type_e	socket_type;
}	event_wrapper_t;

// EventWrapper is a class that is used just for casting the type of socket epoll will return.
// Epoll will return a struct that have a wrapper as data, for knowing to which instance we have to cast it
// we create a wrapper with an enum that specify how to cast the pointer stored into casting_value.
class EventWrapper {
	private:
		std::list<event_wrapper_t*>	_events;

	public:
		EventWrapper(void);
		virtual ~EventWrapper(void);

		event_wrapper_t*	create(enum socket_type_e socket_type);
};

#endif
