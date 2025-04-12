NAME := ircserv

CC := c++
FLAGS := -std=c++20 -Wall -Wextra -Werror

SOURCES := \
	ircserv.cpp \
	Server.cpp \
	Client.cpp \
	Channel.cpp \
	clientRegistration.cpp \
	commands/nick.cpp \
	commands/join.cpp \
	regexRules.cpp

OBJECTS := $(SOURCES:.cpp=.o)

HEADERS := \
	Server.hpp \
	Client.hpp \
	Channel.hpp \
	commands/nick.hpp \
	commands/join.hpp \
	regexRules.hpp

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
