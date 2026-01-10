# Compiler and flags
CC      := gcc
CFLAGS  := -Wall -Wextra -pedantic -std=c11
LDFLAGS :=

# Source files
COMMON_SRC := lib/server.c lib/p2p_thread.c lib/messaging.c lib/user.c
COMMON_OBJ := $(COMMON_SRC:.c=.o)

# Headers (for dependency tracking)
HEADERS := lib/server.h lib/p2p_thread.h lib/messaging.h lib/user.h lib/pch.h

# Executables
TARGETS := utente lavagna

# Default target
all: $(TARGETS)

# Utente (client)
utente: utente.o $(COMMON_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

# Lavagna (server)
lavagna: lavagna.o $(COMMON_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

# Generic rule for object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(TARGETS) *.o

# Run the server
run-server: lavagna
	./lavagna

# Run the client
run-client: utente
	./utente

.PHONY: all clean distclean rebuild run-server run-client