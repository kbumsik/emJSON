BIN= simple_example

# Define cimpilers
CC=gcc
CXX=g++
CCFLAGS= -g -std=c99 -Wall -Wextra -Werror

# Define path
SRC_DIR= ../emJSON/
INC_DIR= ../emJSON/

SRC= $(wildcard $(SRC_DIR)*.c)
OBJ= $(wildcard $(SRC_DIR)*.o)
INC= $(wildcard $(INC_DIR)*.h)

all: $(BIN)

debug: $(BIN)

%.o: %.c $(INC)
	$(CC) $< $(CCFLAGS) -I $(INC_DIR) -c -o $@

$(BIN): simple_example.o ../emJSON/emJSON.o ../emJSON/json_parser.o ../emJSON/json.o
	$(CC) $(CCFLAGS) simple_example.o ../emJSON/json.o ../emJSON/emJSON.o ../emJSON/json_parser.o -I $(INC_DIR) -o simple_example

test:
	./simple_example


.PHONY: clean
clean:
	rm -f *.o simple_example ../emJSON/*.o