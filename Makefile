CPP = c++
CPPFLAGS = -Wall -Wextra -Wshadow -Werror -std=c++98 -g
DEPFLAGS = -MMD -MP

NAME = webserv

SRCS = srcs/main.cpp srcs/HTTPError.cpp srcs/parsing/HTTPRequestParser.cpp \
		srcs/parsing/parsing_utils.cpp srcs/parsing/ConfigFileParsing.cpp \
		srcs/utils.cpp srcs/CGI/php.cpp \
		srcs/Socket.cpp srcs/Client.cpp srcs/Connection.cpp \
		test/print_parsed_conf.cpp test/test_http_parsing.cpp \
		test/print_parsed_request.cpp \
		srcs/HTTPResponse.cpp srcs/ResponseBuilder.cpp srcs/CGIHandler.cpp \
		
OBJS_DIR = ./.obj
OBJS = $(SRCS:%.cpp=$(OBJS_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

all: $(NAME) compile_msg

$(NAME): $(OBJS)
	$(CPP) $(CPPFLAGS) $(OBJS) -o $(NAME)

$(OBJS_DIR)/%.o: %.cpp
	mkdir -p $(@D)
	$(CPP) $(CPPFLAGS) $(DEPFLAGS) -c $< -o $@

-include $(DEPS)

clean: del_msg
	rm -rf $(OBJS_DIR)

fclean: clean all_del_msg
	rm -f $(NAME)

re: fclean all

GREEN = \033[0;32m
YELLOW = \033[0;33m
RESET = \033[0m

compile_msg:
	echo "$(GREEN)Successfully Compiled $(YELLOW)$(NAME) \
	$(GREEN)executable.$(RESET)"

del_msg:
	echo "$(GREEN)Successfully Removed $(YELLOW)$(OBJS_DIR) \
	$(GREEN)directory.$(RESET)"

all_del_msg:
	echo "$(GREEN)Successfully Removed $(YELLOW)$(NAME) \
	$(GREEN)executable.$(RESET)"

.PHONY: all clean fclean re compile_msg del_msg all_del_msg
$(verbose).SILENT:
