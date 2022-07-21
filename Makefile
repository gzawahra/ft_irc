NAME				=	ircserv

DIRECTORY_CHECK		=	src
DIRECTORY_SOURCE	=	src
DIRECTORY_OBJECT	=	objects

FILES_CHECK			=	Utils/Utils.hpp \
						Server/Config.hpp \
						Server/Display.hpp \
						Server/Channel.hpp \
						Server/Server.hpp \
						User/Command/Command.hpp \
						User/Client.hpp

FILES_SOURCE		=	Utils/Utils.cpp \
						Server/Config.cpp \
						Server/Display.cpp \
						Server/Channel.cpp \
						Server/Server.cpp \
						User/Command/Connection.cpp \
						User/Command/Channel.cpp \
						User/Command/Sending.cpp \
						User/Command/Server.cpp \
						User/Command/Service.cpp \
						User/Command/User.cpp \
						User/Command/Misc.cpp \
						User/Command/Command.cpp \
						User/Command/replies.cpp \
						User/Client.cpp \
						main.cpp

COMPILE				=	clang++
COMPILATION_FLAG	=	-Wall -Wextra -Werror -std=c++98 -pedantic-errors -g

ARGUMENTS			=	6667 password

all: $(NAME)

$(DIRECTORY_OBJECT)/%.o: $(DIRECTORY_SOURCE)/%.cpp $(FILES_CHECK:%=$(DIRECTORY_CHECK)/%)
	@printf "\e[33m"
	@printf "Compile\t$< -> $@\n"
	mkdir -p $(dir $@)
	$(COMPILE) $(COMPILATION_FLAG) -c $< -o $@

$(NAME): $(FILES_SOURCE:%.cpp=$(DIRECTORY_OBJECT)/%.o)
	@printf "\e[32m"
	@printf "Build\t$@\n"
	$(COMPILE) -o $@ $(FILES_SOURCE:%.cpp=$(DIRECTORY_OBJECT)/%.o)

run: all
	@printf "\e[0m"
	./$(NAME) $(ARGUMENTS)

clean:
	@printf "\e[31m"
	@printf "Remove\t$(DIRECTORY_OBJECT)\n"
	rm -rf $(DIRECTORY_OBJECT)

fclean: clean
	@printf "Remove\t$(NAME)\n"
	rm -f $(NAME)

re: fclean all

debugflags:
	$(eval COMPILATION_FLAG=-D DEBUG)

debug: debugflags run

leaks: debugflags all
	@printf "\e[0m"
	valgrind --leak-check=full ./$(NAME) $(ARGUMENTS)

.PHONY: all run clean fclean re debugflags debug leaks
.SILENT:
