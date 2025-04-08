NAME := ircserv
TEST := test_channels

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

# Test sources
TEST_SOURCES := Channel.cpp Server.cpp Client.cpp clientRegistration.cpp ServerChannel.cpp nick.cpp test_channels.cpp
TEST_OBJECTS := $(TEST_SOURCES:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJECTS)
		$(CC) $(FLAGS) $(OBJECTS) -o $(NAME) $(LIBS)

$(TEST): $(TEST_OBJECTS)
		$(CC) $(FLAGS) $(TEST_OBJECTS) -o $(TEST) $(LIBS)

%.o: %.cpp $(HEADERS)
		$(CC) $(FLAGS) -c $< -o $@

clean:
		rm -f $(OBJECTS) $(TEST_OBJECTS)

fclean: clean
		rm -f $(NAME) $(TEST)

re: fclean all

.PHONY: all clean fclean re

