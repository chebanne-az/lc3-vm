CC = gcc
CFLAGS = -Wall -g -std=c99
SRC = main.c lc3.c memory.c trap.c
OBJ = $(SRC:.c=.o)
TARGET = lc3

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
