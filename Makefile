INC_DIR = ./include
BIN_DIR = ./bin
SRC_DIR = ./src
OBJ_DIR = ./obj
SRC2_DIR = ./test

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(notdir $(SRC)))

SRC2 = $(wildcard $(SRC2_DIR)/*.cpp)
OBJ2 = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(notdir $(SRC2)))

TARGET = main
BIN_TARGET = $(BIN_DIR)/$(TARGET)

CC = g++
CFLAGS = -g -Wall -I$(INC_DIR) -llog4cpp -lpthread -std=c++11 -ljson

$(BIN_TARGET): $(OBJ) $(OBJ2)
	$(CC) $^ -o $@ $(CFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) -c $< -o $@ $(CFLAGS)

$(OBJ_DIR)/%.o: $(SRC2_DIR)/%.cpp
	$(CC) -c $< -o $@ $(CFLAGS)

.PHONY: clean
clean:
	-rm -f $(OBJ_DIR)/*.o 
