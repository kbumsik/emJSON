BIN=simple_example full_example

BIN_OBJS=$(BIN:=.o)

# Define cimpilers
CC=gcc
CXX=g++
CCFLAGS=-g -std=c99 -Wall -Wextra -Werror

# Define path
SRC_DIR:=../emJSON
INC_DIR:=../emJSON

SRC:=$(wildcard $(SRC_DIR)/*.c)
OBJ:=$(SRC:.c=.o)
INC:=$(wildcard $(INC_DIR)/*.h)

all: $(BIN)

$(SRC_DIR)/%.o: %.c $(INC)
	$(CC) $< $(CCFLAGS) -I $(INC_DIR) -c -o $@
	
%.o: %.c $(INC)
	$(CC) $< $(CCFLAGS) -I $(INC_DIR) -c -o $@

$(BIN): $(BIN_OBJS) $(OBJ)
	$(CC) $(CCFLAGS) $@.o $(OBJ) -I $(INC_DIR) -o $@

test:
	./simple_example


.PHONY: clean
clean:
	rm -f *.o $(BIN) $(BIN_OBJS) $(OBJ)
