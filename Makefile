CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -Iinclude
SRC = src/main.c src/options.c src/entry.c src/sort.c src/format.c src/list.c src/utils.c src/compat.c
OBJ = $(SRC:.c=.o)
TARGET = ls

# Detect Windows environment
ifeq ($(OS),Windows_NT)
    BIN_EXT = .exe
    CLEAN_CMD = cmd /C "del /Q /F $(subst /,\,$(OBJ)) $(TARGET) $(TARGET)$(BIN_EXT) 2>nul"
else
    BIN_EXT =
    CLEAN_CMD = rm -f $(OBJ) $(TARGET)
endif

BIN = $(TARGET)$(BIN_EXT)

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-$(CLEAN_CMD)
