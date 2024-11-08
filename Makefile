NAME = webserv
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
SDIR = srcs
HDIR = headers
SRCS = $(addprefix ${SDIR}/, main.cpp HttpServer.cpp Location.cpp ServerCluster.cpp ServerConfig.cpp utils.cpp)
HDRS = $(addprefix ${HDIR}/, HttpRequest.hpp HttpResponse.hpp HttpServer.hpp Location.hpp ServerCluster.hpp ServerConfig.hpp WebServ.hpp)
TPLS =
ODIR = objs
OBJS = $(SRCS:${SDIR}/%.cpp=${ODIR}/%.o)

all: ${NAME}

${NAME}: ${OBJS}
	c++ ${CXXFLAGS} $^ -o $@

${ODIR}/%.o: ${SDIR}/%.cpp ${HDRS} ${TPLS} | ${ODIR}
	c++ ${CXXFLAGS} -I${HDIR} -c $< -o $@

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
