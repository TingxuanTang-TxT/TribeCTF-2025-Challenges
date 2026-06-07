#Makefile For Challenge 1
CC = gcc
CFLAGS = -O3
TARGET = ./bin/ContactForm
LIBS = -lcrypto -Llib

SRC := $(wildcard src/*.c)
OBJ := $(SRC:src/%.c=obj/%.o)

all: $(OBJ)
	@mkdir -p bin
	$(CC) -o $(TARGET) $(OBJ) $(LIBS) -s

obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) -c $< -o $@ -Iinc $(CFLAGS) $(LIBS) -s

clean:
	rm -f $(TARGET) $(OBJ)
