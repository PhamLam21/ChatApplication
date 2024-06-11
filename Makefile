.PHONY: all clean

CC := g++ -pthread
CFLAG := -c -fPIC -o

BIN_DIR := ./bin
INC_DIR := ./inc
SRC_DIR := ./src
OBJ_DIR := ./obj

all:
	$(CC) $(CFLAG) $(OBJ_DIR)/main.o ./main.cpp -I $(INC_DIR)
	$(CC) $(CFLAG) $(OBJ_DIR)/AppChat.o $(SRC_DIR)/AppChat.cpp -I $(INC_DIR)
	$(CC) $(CFLAG) $(OBJ_DIR)/ConnectionManager.o $(SRC_DIR)/ConnectionManager.cpp -I $(INC_DIR)
	$(CC) $(OBJ_DIR)/main.o $(OBJ_DIR)/AppChat.o $(OBJ_DIR)/ConnectionManager.o -o $(BIN_DIR)/ChatApp
clean: 
	rm -rf $(OBJ_DIR)/*
	rm -rf $(BIN_DIR)/*