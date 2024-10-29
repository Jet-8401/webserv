NAME		= webserv
SRC			= main.cpp
OBJS		= $(SRC:.cpp=.o)

CXX			= c++
CPPFLAGS	= -Wall -Wextra -Werror -g

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $^ -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
