CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g --coverage -I.
LDFLAGS = -lm -ldl --coverage

PLUGIN_CFLAGS = -Wall -Wextra -std=c11 -g -fPIC -I.
PLUGIN_LDFLAGS = -shared -lm

EXEC = calculator
SRCS = main.c calc.c loader.c
OBJS = $(SRCS:.c=.o)

OPS_DIR = ops
PLUGIN_DIR = plugins
PLUGIN_SRCS = $(wildcard $(OPS_DIR)/*.c)
PLUGINS = $(patsubst $(OPS_DIR)/%.c,$(PLUGIN_DIR)/lib%.so,$(PLUGIN_SRCS))

.PHONY: all plugins clean coverage

all: $(EXEC) plugins

plugins: $(PLUGINS)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC) $(LDFLAGS)

%.o: %.c calc.h loader.h plugin_api.h
	$(CC) $(CFLAGS) -c $< -o $@

$(PLUGIN_DIR)/lib%.so: $(OPS_DIR)/%.c plugin_api.h
	mkdir -p $(@D)
	$(CC) $(PLUGIN_CFLAGS) $< -o $@ $(PLUGIN_LDFLAGS)

clean:
	rm -f $(OBJS) $(EXEC) *.gcno *.gcda *.gcov $(OPS_DIR)/*.gcno $(OPS_DIR)/*.gcda
	rm -rf $(PLUGIN_DIR) coverage_report coverage.info

coverage:
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory coverage_report