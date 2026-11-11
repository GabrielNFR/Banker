CC      = gcc
CFLAGS  = -std=gnu11 -Wall -Wextra -Werror -Wno-unused-parameter -O2 -g -pthread
LDFLAGS = -pthread

TARGET = banker
OBJS   = banker.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

banker.o: banker.c banker.h
	$(CC) $(CFLAGS) -c banker.c -o $@

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean