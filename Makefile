NAME := ircserv

CC := c++
FLAGS := -std=c++17 -Wall -Wextra -Werror

# Windows-specific flags
ifeq ($(OS),Windows_NT)
    FLAGS += -D_WIN32_WINNT=0x0601
    LIBS := -lws2_32
else
    LIBS :=
endif

# We're replacing join.cpp with ServerChannel.cpp which contains all channel commands
SOURCES := Channel.cpp nick.cpp main_test_join.cpp Server.cpp Client.cpp clientRegistration.cpp ServerChannel.cpp
OBJECTS := $(SOURCES:.cpp=.o)
HEADERS := Server.hpp Client.hpp Channel.hpp

all: $(NAME)

$(NAME): $(OBJECTS)
		$(CC) $(FLAGS) $(OBJECTS) -o $(NAME) $(LIBS)

%.o: %.cpp $(HEADERS)
		$(CC) $(FLAGS) -c $< -o $@

clean:
		rm -f $(OBJECTS)

fclean: clean
		rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

