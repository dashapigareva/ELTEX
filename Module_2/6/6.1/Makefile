
CC = gcc
CFLAGS = -Wall -Wextra -g


TARGET = phonebook

LIB_NAME = libcontact.a

all: $(TARGET)

$(TARGET): main.o $(LIB_NAME)
	$(CC) $(CFLAGS) -o $(TARGET) main.o -L. -lcontact

# Флаги rcs: r (replace - заменить файлы), c (create - создать архив), s (создать индекс)
$(LIB_NAME): contact.o
	ar rcs $(LIB_NAME) contact.o

main.o: main.c contact.h
	$(CC) $(CFLAGS) -c main.c


contact.o: contact.c contact.h
	$(CC) $(CFLAGS) -c contact.c


clean:
	rm -f *.o *.a $(TARGET)