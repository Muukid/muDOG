CC = clang

CFLAGS = -std=c99 -O3

CFLAGS += -Wall -Wextra -pedantic -Werror
CFLAGS += -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function
CFLAGS += -Wno-deprecated-declarations -Wno-newline-eof

CFLAGS += -Iinclude

.PHONY: clean run compile all dirs

SRC  = $(wildcard src/**/*.c) $(wildcard src/*.c) $(wildcard src/**/**/*.c) $(wildcard src/**/**/**/*.c)
OBJ  = $(SRC:.c=.o)
BIN = bin

APPNAME = muDOG

all: dirs compile

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

ifeq ($(OS),Windows_NT)
dirs:
	if not exist "bin" mkdir $(BIN)
else
dirs:
	mkdir -p $(BIN)
endif

compile: $(OBJ)
	$(CC) -o $(BIN)/$(APPNAME) $^ $(LDFLAGS)

ifeq ($(OS),Windows_NT)
# Oh, Bill.... you are so beautiful sometimes...
clean:
	del /F /Q $(BIN)
	del /F /Q /S *.o
else
clean:
	rm -rf $(BIN) $(OBJ)
endif
