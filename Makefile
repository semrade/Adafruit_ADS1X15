# Nom du compilateur et options
CC = g++
CFLAGS = -g -Wall -Wextra -O0

# Liste des fichiers source
SRC = Adafruit_ADS1X15.cpp main.cpp

# Générer une liste des fichiers objets en remplaçant .cpp par .o dans les noms des fichiers
OBJ = $(SRC:.cpp=.o)

# Nom de l'exécutable final
TARGET = main

# Règle par défaut
all: $(TARGET)

# Règle pour créer l'exécutable final en liant les fichiers objets
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Règle générique pour compiler chaque fichier source en fichier objet
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Règle pour nettoyer les fichiers objets et les exécutables générés
clean:
	rm -f $(OBJ) $(TARGET)

# Déclarations des phony targets pour éviter les conflits avec les fichiers portant les mêmes noms
.PHONY: all clean
