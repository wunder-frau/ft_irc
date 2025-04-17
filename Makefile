NAME := ircserv
TEST := test_channels
TEST_CLIENT := test_client
TEST_JOIN := test_join
TEST_NICK := test_nick
TEST_CHANNEL := test_channel
TEST_SERVER := test_server

CC := c++
FLAGS := -std=c++20 -Wall -Wextra -Werror

SOURCES := \
	ircserv.cpp \
	Server.cpp \
	Client.cpp \
	Channel.cpp \
	clientRegistration.cpp \
	commands/nick.cpp \
	commands/notice.cpp \
	commands/quit.cpp \
	commands/privmsg.cpp \
	commands/join.cpp \
	utils.cpp \
	ServerChannel.cpp \
	regexRules.cpp

# Windows-specific flags
ifeq ($(OS),Windows_NT)
    FLAGS += -D_WIN32_WINNT=0x0601
    LIBS := -lws2_32
else
    LIBS :=
endif

# We're replacing join.cpp with ServerChannel.cpp which contains all channel commands
# SOURCES := Channel.cpp nick.cpp main_test_join.cpp Server.cpp Client.cpp clientRegistration.cpp ServerChannel.cpp
OBJECTS := $(SOURCES:.cpp=.o)
HEADERS := Server.hpp Client.hpp Channel.hpp commands/quit.hpp commands/privmsg.hpp commands/notice.hpp commands/nick.hpp commands/join.hpp utils.hpp regexRules.hpp

# Test sources
TEST_SOURCES := Channel.cpp Server.cpp Client.cpp clientRegistration.cpp ServerChannel.cpp nick.cpp test_channels.cpp
TEST_OBJECTS := $(TEST_SOURCES:.cpp=.o)

# Client test sources
TEST_CLIENT_SOURCES := Client.cpp main_test_client.cpp
TEST_CLIENT_OBJECTS := $(TEST_CLIENT_SOURCES:.cpp=.o)

# Join test sources
TEST_JOIN_SOURCES := Channel.cpp Server.cpp Client.cpp clientRegistration.cpp ServerChannel.cpp nick.cpp main_test_join.cpp
TEST_JOIN_OBJECTS := $(TEST_JOIN_SOURCES:.cpp=.o)

# Nick test sources
TEST_NICK_SOURCES := Channel.cpp Server.cpp Client.cpp clientRegistration.cpp ServerChannel.cpp nick.cpp main_test_nick.cpp
TEST_NICK_OBJECTS := $(TEST_NICK_SOURCES:.cpp=.o)

# Channel test sources
TEST_CHANNEL_SOURCES := Channel.cpp Server.cpp Client.cpp clientRegistration.cpp ServerChannel.cpp nick.cpp main_test_channel.cpp
TEST_CHANNEL_OBJECTS := $(TEST_CHANNEL_SOURCES:.cpp=.o)

# Server test sources
TEST_SERVER_SOURCES := Channel.cpp Server.cpp Client.cpp clientRegistration.cpp ServerChannel.cpp nick.cpp main_test_server.cpp
TEST_SERVER_OBJECTS := $(TEST_SERVER_SOURCES:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJECTS)
		$(CC) $(FLAGS) $(OBJECTS) -o $(NAME) $(LIBS)

$(TEST): $(TEST_OBJECTS)
		$(CC) $(FLAGS) $(TEST_OBJECTS) -o $(TEST) $(LIBS)

$(TEST_CLIENT): $(TEST_CLIENT_OBJECTS)
		$(CC) $(FLAGS) $(TEST_CLIENT_OBJECTS) -o $(TEST_CLIENT) $(LIBS)

$(TEST_JOIN): $(TEST_JOIN_OBJECTS)
		$(CC) $(FLAGS) $(TEST_JOIN_OBJECTS) -o $(TEST_JOIN) $(LIBS)

$(TEST_NICK): $(TEST_NICK_OBJECTS)
		$(CC) $(FLAGS) $(TEST_NICK_OBJECTS) -o $(TEST_NICK) $(LIBS)

$(TEST_CHANNEL): $(TEST_CHANNEL_OBJECTS)
		$(CC) $(FLAGS) $(TEST_CHANNEL_OBJECTS) -o $(TEST_CHANNEL) $(LIBS)

$(TEST_SERVER): $(TEST_SERVER_OBJECTS)
		$(CC) $(FLAGS) $(TEST_SERVER_OBJECTS) -o $(TEST_SERVER) $(LIBS)

%.o: %.cpp $(HEADERS)
	$(CC) $(FLAGS) -c $< -o $@

clean:
		rm -f $(OBJECTS) $(TEST_OBJECTS) $(TEST_CLIENT_OBJECTS) $(TEST_JOIN_OBJECTS) $(TEST_NICK_OBJECTS) $(TEST_CHANNEL_OBJECTS) $(TEST_SERVER_OBJECTS)

fclean: clean
		rm -f $(NAME) $(TEST) $(TEST_CLIENT) $(TEST_JOIN) $(TEST_NICK) $(TEST_CHANNEL) $(TEST_SERVER)

re: fclean all

# Target to compile all tests
all_tests: $(TEST_CLIENT) $(TEST_JOIN) $(TEST_NICK) $(TEST_CHANNEL) $(TEST_SERVER)

# Target to run all tests
run_tests: all_tests
		@echo "Running Client Test..."
		@./$(TEST_CLIENT)
		@echo "\nRunning Join Test..."
		@./$(TEST_JOIN)
		@echo "\nRunning Nick Test..."
		@./$(TEST_NICK)
		@echo "\nRunning Channel Test..."
		@./$(TEST_CHANNEL)
		@echo "\nRunning Server Test..."
		@./$(TEST_SERVER)

.PHONY: all clean fclean re all_tests run_tests

