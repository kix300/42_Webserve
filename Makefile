NAME = webserve
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g3

SRC_DIR = srcs/
OBJ_DIR = obj/
INCLUDE_DIR = include/

SRC = main.cpp \
	  server/handle_connection.cpp server/init.cpp server/utils.cpp server/handle_inout.cpp \
	  response/response.cpp \
	  parsing/parse_server.cpp parsing/parsing_class.cpp parsing/handle_directive.cpp parsing/handle_file.cpp parsing/parsing_utils.cpp

OBJS = $(SRC:%.cpp=$(OBJ_DIR)%.o)


all: create_dir $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "Compilation successful -> [$(NAME)]"

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

create_dir:
	@mkdir -p $(OBJ_DIR)

clean:
	@rm -rf $(OBJ_DIR)
	@echo "Object files removed"

fclean: clean
	@rm -f $(NAME)
	@echo "Executable removed"

re: fclean all

.PHONY: all clean fclean re create_dir
