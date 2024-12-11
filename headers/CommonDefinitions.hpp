#ifndef PARSER_DEFINITIONS_HPP
# define PARSER_DEFINITIONS_HPP

// Handler state

enum handler_state_e {
	READING_HEADERS,
	PARSE_HEADERS,
	VALIDATE_REQUEST,
	NEED_UPGRADE,
	READING_BODY,
	READY_TO_SEND,
	BUILD_HEADERS,
	SENDING_HEADERS,
	SENDING_BODY,
	DONE,
	ERROR,
	SENDING_ERROR_FILE
};

typedef struct handler_state_obj {
	enum handler_state_e	flag;
	bool					continue_loop;

	handler_state_obj(handler_state_e f, bool c): flag(f), continue_loop(c) {}
}	handler_state_t;

// File uploading state

enum uploading_state_e {
	UP_BUFFER_HEADERS,
	UP_HEADER_RECEIVED,
	UP_FILE_CREATE,
	UP_WRITING_FILE,
	UP_DONE,
	UP_ERROR
};

typedef struct uploading_state_obj {
	enum uploading_state_e	flag;
	bool					continue_loop;

	uploading_state_obj(uploading_state_e f, bool c): flag(f), continue_loop(c) {}
}	uploading_state_t;

#endif
