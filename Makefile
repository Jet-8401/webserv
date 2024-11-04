NAME		= webserv
FILES		= main.cpp
SRC			= $(addprefix srcs/,$(FILES))
OBJS		= $(SRC:.cpp=.o)

CXX			= c++
CPPFLAGS	= -Wall -Wextra -Werror -g -O3 -Iheaders/

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $^ -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
