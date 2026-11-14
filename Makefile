CC      = gcc
CFLAGS  = -std=gnu11 -Wall -Wextra -Werror -Wno-unused-parameter -O2 -g

TARGET = banker
OBJS   = banker.o parse.o customer.o commands.o safety.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

banker.o: banker.c parse.h customer.h commands.h safety.h
parse.o: parse.c parse.h
customer.o: customer.c customer.h parse.h
commands.o: commands.c commands.h parse.h
safety.o: safety.c safety.h customer.h

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean
