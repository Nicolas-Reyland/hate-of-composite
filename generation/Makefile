# Makefile for httpd project


# -*- Setup Compilation Variables -*-
CC ?= gcc

CFLAGS = -Wall -Wextra -Wvla
# not sure ?
CFLAGS += -g
# mandatory (self)
CFLAGS += -Isrc -I.
# custom
CFLAGS += $(CMD_CFLAGS)

TEST_LDLIBS = -lcriterion
LDLIBS = -lssl -lcrypto


# -*- Setup Files Variables -*-
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

MAIN_C = main.c
SRCS = $(filter-out $(MAIN_C), $(wildcard *.c))
TEST_SRCS = $(wildcard tests/test_*.c)
OBJS = $(SRCS:.c=.o)
TEST_OBJS = $(TEST_SRCS:.c=.o)

EXE = my_prime
TEST_EXE = my_prime-test


# -*- Rules -*-
all: $(EXE)

check: $(TEST_EXE) $(EXE)
	@echo No tests written yet

$(EXE): $(OBJS) $(MAIN_C)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(TEST_EXE): $(OBJS) $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS) $(TEST_LDLIBS)

clean:
	$(RM) $(EXE) $(OBJS)
	$(RM) $(TEST_EXE) $(TEST_OBJS)


# -*- Misc -*-
.PHONY: all check clean
