CFLAGS = -std=gnu11 -Wall -Werror -Wextra

.PHONY: all clean factory

all: factory

clean:
	$(RM) factory

factory:
	$(CC) $(CFLAGS) factory.c -o factory -pthread