# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: axburin- <axburin-@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/06/25 10:15:35 by axburin-          #+#    #+#              #
#    Updated: 2025/07/08 10:47:24 by axburin-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME        = ircserv
INCLUDE     = include

CC          = c++
CFLAGS      = -Wall -Wextra -Werror -std=c++98 -Iincludes

RM          = rm -f
MKDIR       = mkdir

# Colors
DEF_COLOR = \033[0;39m
GRAY = \033[0;90m
RED = \033[0;91m
GREEN = \033[0;92m
YELLOW = \033[0;93m
BLUE = \033[0;94m
MAGENTA = \033[0;95m
CYAN = \033[0;96m
WHITE = \033[0;97m

SRC = ./src/main.cpp	\
	./src/utils/utils.cpp	\
	./src/utils/error.cpp	\
	./src/utils/irc_utils.cpp	\
	./src/class/Client.cpp	\
	./src/class/Server.cpp	\
	./src/class/Channel.cpp \
	./src/parsing/IRCMessage.cpp \
    ./src/parsing/CommandParser.cpp \

OBJ_DIR       = ./obj/
OBJ           = $(patsubst %.cpp, $(OBJ_DIR)%.o, $(SRC))

all: $(NAME)

$(NAME): $(OBJ)
	@$(CC) $(CFLAGS) $(OBJ) -o $(NAME)
	@printf "$(GREEN)%s compiled!\n$(DEF_COLOR)" $(NAME)

$(OBJ_DIR)%.o: %.cpp
	@mkdir -p $(dir $@)
	@printf "$(CYAN)Compiling: $(notdir $<)$(DEF_COLOR)\n"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@if [ -d $(OBJ_DIR) ]; then \
		$(RM) -r $(OBJ_DIR); \
		printf "$(YELLOW)Objects removed.\n$(DEF_COLOR)"; \
	fi

fclean: clean
	@if [ -e $(NAME) ]; then \
		printf "$(YELLOW)Removing \"%s\"...\n$(DEF_COLOR)" $(NAME); \
		$(RM) $(NAME); \
	fi

re: fclean all

.PHONY: all clean fclean re