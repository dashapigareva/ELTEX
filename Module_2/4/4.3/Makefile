CC = gcc
CFLAGS = -Wall -Wextra -g

TARGET = phonebook

OBJS = main.o contact.o

all: $(TARGET)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)
main.o: main.c contact.h
	$(CC) $(CFLAGS) -c main.c
contact.o: contact.c contact.h
	$(CC) $(CFLAGS) -c contact.c
clean:
	rm -f *.o $(TARGET)