CC      = gcc
CFLAGS  = -Wall -Wextra -std=gnu11 -g -MMD -MP
LDFLAGS =
TARGET  = chmod_mask

SRCS = main.c menu.c permissions.c file_info.c
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^


%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

-include $(DEPS)