#ifndef PARSER_DEFINITIONS_HPP
# define PARSER_DEFINITIONS_HPP

enum parsing_state_e {
	READING_HEADERS,
	PARSE_HEADERS,
	VALIDATE_REQUEST,
	NEED_UPGRADE,
	READING_BODY,
	DONE,
	ERROR
};

typedef struct state_obj {
	enum parsing_state_e	flag;
	bool					continue_loop;

	state_obj(parsing_state_e f, bool c): flag(f), continue_loop(c) {}
}	parsing_state_t;

#endif
