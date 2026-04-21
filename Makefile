CC = gcc
CFLAGS = -Wall -Wextra -D_POSIX_C_SOURCE=200809L

all: city_manager

city_manager: city_manager.c
	$(CC) $(CFLAGS) -o city_manager city_manager.c

clean:
	rm -f city_manager
