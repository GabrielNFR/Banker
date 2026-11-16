CC      = gcc
CFLAGS  = -std=gnu11 -Wall -Wextra -Werror -Wno-unused-parameter -O2 -g

TARGET  = banker
SRC     = banker.c
OBJ     = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC) -o $@

clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: all clean
