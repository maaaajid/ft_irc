NAME = ircserv

CXX = c++
CPPFLAGS = -Wall -Wextra -Werror -std=c++98 

SRCS = src/channel.cpp \
		src/client.cpp \
		src/command.cpp \
		src/logger.cpp \
		src/replies.cpp \
		src/server.cpp \
		src/utils.cpp \
		src/bot.cpp \
		main.cpp \

OBJS = $(SRCS:.cpp=.o)

all : $(NAME)

$(NAME) : $(OBJS)
	$(CXX) $(CPPFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

clean :
	rm -f $(OBJS)

fclean : clean
	rm -f $(NAME) 

re : fclean all