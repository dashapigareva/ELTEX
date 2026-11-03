
CC = gcc
CFLAGS = -Wall -Wextra -g

TARGET = phonebook
LIB_NAME = libcontact.so

all: $(TARGET)

$(TARGET): main.o $(LIB_NAME)
	$(CC) $(CFLAGS) -o $(TARGET) main.o -L. -lcontact -Wl,-rpath,.


$(LIB_NAME): contact.o
	$(CC) -shared -o $(LIB_NAME) contact.o


main.o: main.c contact.h
	$(CC) $(CFLAGS) -c main.c

contact.o: contact.c contact.h
	$(CC) $(CFLAGS) -fPIC -c contact.c


clean:
	rm -f *.o *.a *.so $(TARGET)