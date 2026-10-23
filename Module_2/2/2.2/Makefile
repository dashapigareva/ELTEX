CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g --coverage
LDFLAGS = -lm --coverage

SRCS = main.c calc.c
OBJS = $(SRCS:.c=.o)
EXEC = calculator

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC) *.gcno *.gcda *.gcov
	rm -rf coverage_report


coverage:
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory coverage_report