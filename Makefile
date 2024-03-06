BIN_DIR := bin
SRC_DIR := src
COMPILER := clang
LIB_DIR := lib

build: 
	$(COMPILER) -c $(SRC_DIR)/main.c -I$(LIB_DIR)/ -o $(BIN_DIR)/objs/main.o
	ar rcs $(BIN_DIR)/libserver.a $(BIN_DIR)/objs/main.o
	cp $(LIB_DIR)/server.h $(BIN_DIR)/

run-example: $(SRC_DIR)/example.out
	./example/example.out

$(SRC_DIR)/example.out:
	cp $(BIN_DIR)/libserver.a $(LIB_DIR)/server.h example/
	$(COMPILER) example/libserver.a example/example.c -Iexample/ -o example/example.out

