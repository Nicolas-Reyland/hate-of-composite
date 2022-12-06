# Makefile for httpd project

# Here is a list of all the compilation flags (-D...) that can be passed to CMD_CFLAGS :
# - BETTER_CHOOSE_RSA_E : choose the rsa exponent 'e' manually. it's generally bad idea ...
# - CANDIDATES_COUNT : enable logging of candidate counts (can lighlty slow down the program)
# - FORTUNA_NO_AUTO_RESEED : disable Fortuna CSPRNG self-reseeding. use this if your system is corrupt in some way
# - MILLER_RABIN_MAX_NUM_TESTS=N : max number of tests to perform with miller rabin, until we decide that the candidate is indeed, prime
#                                  by default, this value is set to 40 (see reason in docs)


# -*- Setup Compilation Variables -*-
CC ?= gcc

# Warnings
CFLAGS += -Wall -Wextra -Wvla -Werror
# Standard
CFLAGS += -std=c99
# Include paths
CFLAGS += -Isrc

# custom
CFLAGS += $(CMD_CFLAGS)

TEST_LDLIBS = -lcriterion
LDLIBS = -lssl -lcrypto

ifdef DEBUG
# debugging
CFLAGS += -g
# fsanitize
CFLAGS += -fsanitize=address
LD_LIBS += -fsanitize=address
endif


# -*- Setup Files Variables -*-
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

MAIN_C = src/main.c
SRCS = $(filter-out $(MAIN_C), $(call rwildcard, src, *.c))
TEST_SRCS = $(wildcard tests/test_*.c)
OBJS = $(SRCS:.c=.o)
TEST_OBJS = $(TEST_SRCS:.c=.o)

EXE = my_prime
TEST_EXE = my_prime-test


# -*- Rules -*-
all: $(EXE)

$(EXE): $(OBJS) $(MAIN_C)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

check: $(TEST_EXE) $(EXE)
	@echo No tests written yet

$(TEST_EXE): $(OBJS) $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS) $(TEST_LDLIBS)

clean:
	$(RM) $(EXE) $(OBJS)
	$(RM) $(TEST_EXE) $(TEST_OBJS)


# -*- Misc -*-
.PHONY: all check clean

