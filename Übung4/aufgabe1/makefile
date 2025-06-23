# Name der ausführbaren Datei
TARGET = start

# Name der Quelldatei
SRC = /Users/mathusan/Desktop/Übung4/aufgabe1/start.c

# Compiler und Flags
CC = gcc
CFLAGS = -Wall -Wextra -O2

# Standardziel: Programm bauen
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Ziel zum Löschen der erzeugten Dateien
clean:
	rm -f $(TARGET)

# Optional: Ziel für 'make run', um das Programm mit Beispielparametern zu starten
run: $(TARGET)
	./$(TARGET) ls -l /tmp

.PHONY: all clean run

