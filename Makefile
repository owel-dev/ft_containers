CLANG = c++
CLANGFLAGS = -Wall -Wextra -Werror -std=c++98

ifdef USE_STL
	CLANGFLAGS += -D USE_STL=$(USE_STL)
endif

NAME = ft_containers
RM = rm -rf

OBJ_DIR = obj
SRC_DIR = src
INC_DIR = includes

FILES = main

ifeq ($(DEBUG),true)
	CLANG += -g
endif

SRCS = $(addsuffix .cpp, $(FILES))
OBJS = $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))

all: $(NAME)

$(OBJ_DIR) :
	@mkdir obj

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@$(CLANG) $(CLANGFLAGS) -I $(INC_DIR) -c $< -o $@	

$(NAME): $(OBJS)
	@$(CLANG) $(CLANGFLAGS) -o $(NAME) $(OBJS)

clean:
	@$(RM) $(OBJS)

fclean: clean
	@$(RM) $(NAME)
	@$(RM) $(TEST_BUILD_DIR)

re: clean $(NAME)

bench:
	@$(MAKE) USE_STL=1
	@echo "Testing with STL"
	@time ./$(NAME)
	@$(MAKE) clean
	@echo
	@$(MAKE) USE_STL=0
	@echo "Testing with my ft-containers STL"
	@time ./$(NAME)

TEST_DIR = tests
TEST_BUILD_DIR = build/tests
TEST_SRCS = $(wildcard $(TEST_DIR)/test_*.cpp)
TEST_BINS = $(patsubst $(TEST_DIR)/%.cpp,$(TEST_BUILD_DIR)/%,$(TEST_SRCS))
TEST_DEPS = $(wildcard $(TEST_DIR)/*.hpp) $(wildcard $(INC_DIR)/*.hpp)

test: $(TEST_BINS)
	@failed=0; first=1; \
	for bin in $(TEST_BINS); do \
	  if [ $$first -eq 1 ]; then first=0; else echo; fi; \
	  if [ -x $$bin ]; then ./$$bin || failed=1; else failed=1; fi; \
	done; \
	exit $$failed

$(TEST_BUILD_DIR)/%: $(TEST_DIR)/%.cpp $(TEST_DEPS) | $(TEST_BUILD_DIR)
	@$(RM) $@
	@$(CLANG) $(CLANGFLAGS) -I $(INC_DIR) -I $(TEST_DIR) $< -o $@ || { \
	  if [ -t 1 ] && [ -z "$$NO_COLOR" ]; then red='\033[1;31m'; reset='\033[0m'; \
	  else red=''; reset=''; fi; \
	  printf "  $${red}FAIL$${reset} %s did not compile\n" "$<"; }

$(TEST_BUILD_DIR):
	@mkdir -p $(TEST_BUILD_DIR)

.PHONY: all clean fclean re bench test
