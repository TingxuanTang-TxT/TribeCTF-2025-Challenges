CC=arm-none-eabi-gcc
LD=arm-none-eabi-ld
OBJCOPY=arm-none-eabi-objcopy

CFLAGS=-mcpu=cortex-r5 -O2 -nostdlib -ffreestanding -Wall -s
INCLUDE := -Iincludes
LDFLAGS=-T linker.ld

SRC=src
OBJ=obj
INCDIR=includes

# Support .c and .S files in src/
CSRC := $(shell find $(SRC) -name "*.c")
SASMSRC := $(shell find $(SRC) -name "*.S")
OBJS := $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(CSRC)) \
        $(patsubst $(SRC)/%.S,$(OBJ)/%.o,$(SASMSRC))

OBJDIRS := $(patsubst $(SRC)/%,$(OBJ)/%,$(sort $(dir $(CSRC) $(SASMSRC))))

BINDIR=bin

.PHONY: all
all: $(BINDIR)/flashrom1.bin

$(BINDIR)/flashrom1.elf: $(OBJS) | $(BINDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@

$(BINDIR)/flashrom1.bin: $(BINDIR)/flashrom1.elf
	$(OBJCOPY) -O binary $< $@

$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(OBJ)/%.o: $(SRC)/%.S | $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(OBJ): 
	mkdir -p $(OBJDIRS)

$(BINDIR):
	mkdir -p $@

.PHONY: clean
clean:
	$(RM) -rf $(OBJ)/*.o $(BINDIR)/*
