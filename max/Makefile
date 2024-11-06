NAME = webserv
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
SRCS = main.cpp Config.cpp Parser.cpp
HDRS = Config.hpp ConfigValue.hpp Parser.hpp
TPLS = ConfigValue.tpp
ODIR = objs
OBJS = $(addprefix ${ODIR}/, ${SRCS:.cpp=.o})

all: ${NAME}

${NAME}: ${OBJS}
	c++ ${CXXFLAGS} $^ -o $@

${ODIR}/%.o: %.cpp ${HDRS} ${TPLS} | ${ODIR}
	c++ ${CXXFLAGS} -c $< -o $@

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
