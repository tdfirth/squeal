# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Wno-gnu-case-range -Isrc -g -std=c11 -pedantic -O0
LDFLAGS = 

# Directories
SRC = src
TEST = test
LIB = lib
OBJ = build/obj
BIN = build/bin

# Source files
MAIN = $(SRC)/main.c
SRC_C = $(filter-out $(MAIN), $(wildcard $(SRC)/*.c))
TEST_C = $(wildcard $(TEST)/*.c)
MUNIT_C = $(wildcard $(LIB)/munit/*.c)

# Object files
MAIN_OBJ = $(OBJ)/main.o
SRC_OBJ = $(SRC_C:$(SRC)/%.c=$(OBJ)/%.o)
TEST_OBJ = $(TEST_C:$(TEST)/%.c=$(OBJ)/test/%.o)
MUNIT_OBJ = $(MUNIT_C:$(LIB)/%.c=$(OBJ)/%.o)

# Targets
NAME = squeal
TEST_NAME = test-$(NAME)

# main
TARGET = $(BIN)/$(NAME)

$(NAME): dirs $(TARGET)

args ?= 

run: $(NAME)
	./$(TARGET) $(args)

$(TARGET): $(SRC_OBJ) $(MAIN_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

# test
TEST_TARGET = $(BIN)/$(TEST_NAME)

test: $(TEST_NAME)
	./$(TEST_TARGET) $(args)

$(TEST_NAME): dirs $(TEST_TARGET)

# Test target links and include munit.
$(MUNIT_OBJ): LDFLAGS += -Llib/munit
$(TEST_TARGET): CFLAGS += -Ilib/munit
$(TEST_TARGET): $(SRC_OBJ) $(TEST_OBJ) $(MUNIT_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(MAIN_OBJ): $(MAIN)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/test/%.o: $(TEST)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/%.o: $(LIB)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

dirs:
	@mkdir -p build/{bin,obj}
	@mkdir -p build/obj/{test,munit}

clean:
	@rm -rf build

debug-vars:
	@echo "SRC_C = $(SRC_C)"
	@echo "SRC_OBJ = $(SRC_OBJ)"
	@echo "TEST_C = $(TEST_C)"
	@echo "TEST_OBJ = $(TEST_OBJ)"
	@echo "MUNIT_C = $(MUNIT_C)"
	@echo "MUNIT_OBJ = $(MUNIT_OBJ)"

.PHONY: all squeal test-squeal clean print-vars dirs
