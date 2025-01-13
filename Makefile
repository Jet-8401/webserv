NAME = webserv
CXXFLAG = -Wall -Werror -Wextra -g -std=c++98
SDIR = srcs
HDIR = headers

SRCS = $(addprefix ${SDIR}/, \
	main.cpp \
	utils.cpp \
	Location.cpp \
	ServerCluster.cpp \
	ServerConfig.cpp \
	EventWrapper.cpp \
	Socket.cpp \
	Connection.cpp \
	HttpRequest.cpp \
	HttpResponse.cpp \
	HttpMessage.cpp \
	HttpParser.cpp \
	BytesBuffer.cpp \
	StreamBuffer.cpp \
	HttpGetStaticFile.cpp \
	HttpGetDirectory.cpp \
	HttpPost.cpp \
	HttpDelete.cpp \
	HttpCGI.cpp)

TPLS =
ODIR = objs
OBJS = $(SRCS:${SDIR}/%.cpp=${ODIR}/%.o)
DEFINES = -DDEBUGGER

CGI_DIR = shared/cgi-bin

all: setup_dirs ${NAME}

${NAME}: ${OBJS}
	c++ ${CXXFLAGS} $^ -o $@

${ODIR}/%.o: ${SDIR}/%.cpp ${TPLS} | ${ODIR}
	c++ ${DEFINES} ${CXXFLAG} -I${HDIR} -c $< -o $@

${ODIR}:
	mkdir -p ${ODIR}

clean:
	rm -rf ${ODIR}

fclean: clean
	rm -f ${NAME}

re: fclean all

help:
	@echo "Targets:"
	@echo "  all:	Build the program"
	@echo "  clean:  Remove object files"
	@echo "  fclean: Remove object files and executable"
	@echo "  re:	 Rebuild the program"

setup_dirs:
	@echo "Setting up directories..."
	@mkdir -p $(CGI_DIR)
	@chmod u-w $(CGI_DIR)  # Read and execute, but not write
	@echo "Directory permissions set"

.PHONY: all clean fclean re help setup_dirs
