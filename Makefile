NAME				=	ircserv

DIRECTORY_CHECK		=	src
DIRECTORY_SOURCE	=	src
DIRECTORY_OBJECT	=	objects

FILES_CHECK			=	Utils/Utils.hpp \
						Server/Config/Config.hpp \
						Server/Display/Display.hpp \
						Server/Channel/Channel.hpp \
						Server/Server.hpp \
						User/Command/Command.hpp \
						User/User.hpp
FILES_SOURCE		=	Utils/Utils.cpp \
						Server/Config/Config.cpp \
						Server/Display/Display.cpp \
						Server/Channel/Channel.cpp \
						Server/Server.cpp \
						User/Command/Connection/PASS.cpp \
						User/Command/Connection/NICK.cpp \
						User/Command/Connection/USER.cpp \
						User/Command/Connection/OPER.cpp \
						User/Command/Connection/MODE.cpp \
						User/Command/Connection/SERVICE.cpp \
						User/Command/Connection/QUIT.cpp \
						User/Command/Connection/SQUIT.cpp \
						User/Command/Channel.cpp \
						User/Command/Sending.cpp \
						User/Command/Server.cpp \
						User/Command/Service.cpp \
						User/Command/User.cpp \
						User/Command/Misc.cpp \
						User/Command/Optional/AWAY.cpp \
						User/Command/Optional/REHASH.cpp \
						User/Command/Optional/DIE.cpp \
						User/Command/Optional/RESTART.cpp \
						User/Command/Optional/SUMMON.cpp \
						User/Command/Optional/USERS.cpp \
						User/Command/Optional/WALLOPS.cpp \
						User/Command/Optional/USERHOST.cpp \
						User/Command/Optional/ISON.cpp \
						User/Command/Command.cpp \
						User/Command/replies.cpp \
						User/User.cpp \
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
