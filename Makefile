CC = gcc
CFLAGS = -Wall -Wextra -g
EXECUTABLE = myLib
SRCS = main.c myDSlib.c
OBJS = main.o myDSlib.o

$(EXECUTABLE): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(EXECUTABLE) $(OBJS)