# Default to gcc if CC isn't set (local builds)
CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -g -Wall -Werror
LDFLAGS ?= -pthread

TARGET = aesdsocket

all: $(TARGET)

default: $(TARGET)

$(TARGET): aesdsocket.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

aesdsocket.o: aesdsocket.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) *.o
