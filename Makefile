NAME = webserv
CXXFLAGS =-Wall -Wextra -Werror -g -std=c++98
SDIR = srcs
HDIR = headers
SRCS = $(addprefix ${SDIR}/, main.cpp HttpServer.cpp Location.cpp ServerCluster.cpp ServerConfig.cpp EventWrapper.cpp Connection.cpp HttpRequest.cpp HttpResponse.cpp HttpMessage.cpp BytesBuffer.cpp StreamBuffer.cpp utils.cpp)
HDRS = $(addprefix ${HDIR}/, HttpRequest.hpp HttpResponse.hpp HttpServer.hpp Location.hpp ServerCluster.hpp ServerConfig.hpp EventWrapper.hpp WebServ.hpp)
TPLS =
ODIR = objs
OBJS = $(SRCS:${SDIR}/%.cpp=${ODIR}/%.o)
DEFINES = -DDEBUGGER

all: ${NAME}

${NAME}: ${OBJS}
	c++ ${CXXFLAGS} $^ -o $@

${ODIR}/%.o: ${SDIR}/%.cpp ${HDRS} ${TPLS} | ${ODIR}
	c++ ${DEFINES} ${CXXFLAGS} -I${HDIR} -c $< -o $@

${ODIR}:
	mkdir -p ${ODIR}

clean:
	rm -rf ${ODIR}

fclean: clean
	rm -f ${NAME}

re: fclean all

help:
	@echo "Targets:"
	@echo "  all:    Build the program"
	@echo "  clean:  Remove object files"
	@echo "  fclean: Remove object files and executable"
	@echo "  re:     Rebuild the program"

.PHONY: all clean fclean re help
