#ifndef EVENTS_HPP
# define EVENTS_HPP

typedef struct epoll_event_s {
	void*	data;

	enum socket_type_e {
		NONE,
		REQUEST,
		CLIENT
	};

}	epoll_events_t;

#endif
