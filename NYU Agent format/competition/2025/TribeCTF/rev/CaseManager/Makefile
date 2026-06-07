#Makefile For Challenge 2
CC = gcc
CFLAGS = -O1
TARGET = ./bin/CaseManager
LIBS = -lcrypto -L./lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SRC := $(wildcard src/*.c)
OBJ := $(SRC:src/%.c=obj/%.o)
DEBUGOBJ := $(SRC:src/%.c=obj2/%.o)

all: $(OBJ)
	@mkdir -p bin
	$(CC) -o $(TARGET) $(OBJ) $(LIBS) -s

obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) -c $< -o $@ -Iinc $(CFLAGS) $(LIBS) -s

debug: $(DEBUGOBJ)
	@mkdir -p bin
	$(CC) -o $(TARGET)_DEBUG $(DEBUGOBJ) -Wall $(LIBS) 

obj2/%.o: src/%.c
	@mkdir -p obj2
	$(CC) -c $< -o $@ -Iinc -O0 -Wall $(LIBS) -g

clean:
	rm -f $(TARGET) $(OBJ)
	rm -f $(TARGET)_DEBUG $(DEBUGOBJ)
