
# Nome dell'eseguibile
TARGET = game-of-life

# File sorgente
SRC = main.c

# Rilevamento OS
UNAME_S := $(shell uname -s)

# Opzioni e librerie dipendenti dalla piattaforma
ifeq ($(OS),Windows_NT)
    CC = gcc
    CFLAGS = -Wall
    LDFLAGS = 
    EXECUTABLE = $(TARGET).exe
else ifeq ($(UNAME_S),Linux)
    CC = gcc
    CFLAGS = -Wall
    LDFLAGS = 
    EXECUTABLE = $(TARGET)
else ifeq ($(UNAME_S),Darwin)  # macOS
    CC = gcc
    CFLAGS = -Wall
    LDFLAGS = 
    EXECUTABLE = $(TARGET)
endif

.PHONY: all clean run

all: $(EXECUTABLE)

$(EXECUTABLE): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(EXECUTABLE) $(LDFLAGS)

run: $(EXECUTABLE)
	./$(EXECUTABLE)

clean:
	rm -f $(TARGET) $(TARGET).exe
