CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I.
TARGET = project1
SRCS = main.c A.c B.c C.c D.c utils.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
