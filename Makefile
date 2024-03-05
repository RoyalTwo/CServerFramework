BIN_DIR := bin
SRC_DIR := src
COMPILER := clang
LIB_DIR := lib

build: 

run-example: $(SRC_DIR)/example.out
	./$(BIN_DIR)/example.out

$(SRC_DIR)/example.out:
	$(COMPILER) example/example.c $(SRC_DIR)/main.c -I$(LIB_DIR)/ -o $(BIN_DIR)/example.out
