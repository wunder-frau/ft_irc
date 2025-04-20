NAME := ircserv
TEST := test_channels
TEST_CLIENT := test_client
TEST_JOIN := test_join
TEST_NICK := test_nick
TEST_CHANNEL := test_channel
TEST_SERVER := test_server

CC := g++
FLAGS := -std=c++20 -Wall -Wextra -Werror
INCLUDES := -I. -Imodes

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
    commands/mode.cpp \
    utils.cpp \
    ServerChannel.cpp \
    regexRules.cpp \
    ServerModes.cpp \
    modes/ModeHandler.cpp \
    modes/ModeUtils.cpp

# Windows-specific flags
ifeq ($(OS),Windows_NT)
    FLAGS += -D_WIN32_WINNT=0x0601
    LIBS := -lws2_32
else
    LIBS :=
endif

OBJECTS := $(SOURCES:.cpp=.o)
HEADERS := \
    Server.hpp \
    Client.hpp \
    Channel.hpp \
    commands/quit.hpp \
    commands/privmsg.hpp \
    commands/notice.hpp \
    commands/nick.hpp \
    commands/join.hpp \
    commands/mode.cpp \
    utils.hpp \
    regexRules.hpp \
    modes/ModeHandler.hpp \
    modes/ModeUtils.hpp

# Exclude ircserv.o (contains main()) from test builds.
TEST_APP_OBJECTS := $(filter-out ircserv.o, $(OBJECTS))

# Test sources and objects
TEST_SOURCES := Channel.cpp Server.cpp Client.cpp clientRegistration.cpp ServerChannel.cpp nick.cpp test_channels.cpp
TEST_OBJECTS := $(TEST_SOURCES:.cpp=.o)

TEST_CLIENT_SOURCES := Client.cpp main_test_client.cpp
TEST_CLIENT_OBJECTS := $(TEST_CLIENT_SOURCES:.cpp=.o)

TEST_JOIN_SOURCES := Channel.cpp Server.cpp Client.cpp clientRegistration.cpp ServerChannel.cpp nick.cpp main_test_join.cpp
TEST_JOIN_OBJECTS := $(TEST_JOIN_SOURCES:.cpp=.o)

TEST_NICK_SOURCES := Channel.cpp Server.cpp Client.cpp clientRegistration.cpp ServerChannel.cpp nick.cpp main_test_nick.cpp
TEST_NICK_OBJECTS := $(TEST_NICK_SOURCES:.cpp=.o)

TEST_CHANNEL_SOURCES := Channel.cpp Server.cpp Client.cpp clientRegistration.cpp ServerChannel.cpp nick.cpp main_test_channel.cpp
TEST_CHANNEL_OBJECTS := $(TEST_CHANNEL_SOURCES:.cpp=.o)

TEST_SERVER_SOURCES := Channel.cpp Server.cpp Client.cpp clientRegistration.cpp ServerChannel.cpp nick.cpp main_test_server.cpp
TEST_SERVER_OBJECTS := $(TEST_SERVER_SOURCES:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CC) $(FLAGS) $(INCLUDES) $(OBJECTS) -o $(NAME) $(LIBS)

$(TEST): $(TEST_APP_OBJECTS) test_channels.o
	$(CC) $(FLAGS) $(INCLUDES) $(TEST_APP_OBJECTS) test_channels.o -o $(TEST) $(LIBS)

$(TEST_CLIENT): $(TEST_APP_OBJECTS) main_test_client.o
	$(CC) $(FLAGS) $(INCLUDES) $(TEST_APP_OBJECTS) main_test_client.o -o $(TEST_CLIENT) $(LIBS)

$(TEST_JOIN): $(TEST_APP_OBJECTS) main_test_join.o
	$(CC) $(FLAGS) $(INCLUDES) $(TEST_APP_OBJECTS) main_test_join.o -o $(TEST_JOIN) $(LIBS)

$(TEST_NICK): $(TEST_APP_OBJECTS) main_test_nick.o
	$(CC) $(FLAGS) $(INCLUDES) $(TEST_APP_OBJECTS) main_test_nick.o -o $(TEST_NICK) $(LIBS)

$(TEST_CHANNEL): $(TEST_APP_OBJECTS) main_test_channel.o
	$(CC) $(FLAGS) $(INCLUDES) $(TEST_APP_OBJECTS) main_test_channel.o -o $(TEST_CHANNEL) $(LIBS)

$(TEST_SERVER): $(TEST_APP_OBJECTS) main_test_server.o
	$(CC) $(FLAGS) $(INCLUDES) $(TEST_APP_OBJECTS) main_test_server.o -o $(TEST_SERVER) $(LIBS)

%.o: %.cpp $(HEADERS)
	$(CC) $(FLAGS) $(INCLUDES) -c $< -o $@

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
