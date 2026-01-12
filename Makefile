# Compiler and flags
CC      := gcc
CFLAGS  := -Wall -Wextra -pedantic -std=c11
LDFLAGS :=

# Source files
COMMON_SRC := src/common/server.c src/common/protocol.c src/common/client.c
COMMON_OBJ := $(COMMON_SRC:.c=.o)

UTENTE_SRC := src/utente/p2p_thread.c src/utente/worker_thread.c src/utente/utente_state.c src/utente/utente_utils.c
UTENTE_OBJ := $(UTENTE_SRC:.c=.o) 

LAVAGNA_SRC := src/lavagna/database.c src/lavagna/lavagna_utils.c
LAVAGNA_OBJ := $(LAVAGNA_SRC:.c=.o)

# Headers (for dependency tracking)
HEADERS := include/server.h include/protocol.h include/client.h include/common.h include/thread.h include/utente_state.h include/database.h include/utente_utils.h include/lavagna_utils.h

# Executables
TARGETS := utente lavagna

# Default target
all: $(TARGETS)

# Utente (client)
utente: utente.o $(COMMON_OBJ) $(UTENTE_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

# Lavagna (server)
lavagna: lavagna.o $(COMMON_OBJ) $(LAVAGNA_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

# Generic rule for object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(TARGETS) *.o
	rm -f src/**/*.o

# Run the server
run-server: lavagna
	./lavagna

# Run the client
run-client: utente
	./utente

.PHONY: all clean distclean rebuild run-server run-client