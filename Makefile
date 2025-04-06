NAME := ircserv

CC := c++
FLAGS := -std=c++17 -Wall -Wextra -Werror

SOURCES := Channel.cpp nick.cpp main_test_channel.cpp Server.cpp Client.cpp clientRegistration.cpp
OBJECTS := $(SOURCES:.cpp=.o)
HEADERS := Server.hpp Client.hpp Channel.hpp

all: $(NAME)

$(NAME): $(OBJECTS)
		$(CC) $(FLAGS) $(OBJECTS) -o $(NAME)

%.o: %.cpp $(HEADERS)
		$(CC) $(FLAGS) -c $< -o $@

clean:
		rm -f $(OBJECTS)

fclean: clean
		rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

