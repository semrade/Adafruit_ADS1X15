# Nom du compilateur et options
CC = g++
CFLAGS = -g -Wall -Wextra -O0

# Liste des fichiers source C++
SRC_CPP = Adafruit_ADS1X15.cpp main.cpp

# Liste des fichiers source C
SRC_C = driver.c

# Générer une liste des fichiers objets pour les fichiers source C++
OBJ_CPP = $(SRC_CPP:.cpp=.o)

# Générer une liste des fichiers objets pour les fichiers source C
OBJ_C = $(SRC_C:.c=.o)

# Nom de l'exécutable final
TARGET = main

# Règle par défaut
all: $(TARGET)

# Règle pour créer l'exécutable final en liant tous les fichiers objets
$(TARGET): $(OBJ_CPP) $(OBJ_C)
	$(CC) $(CFLAGS) -o $@ $^

# Règle générique pour compiler chaque fichier source C++
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Règle générique pour compiler chaque fichier source C
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Règle pour nettoyer les fichiers objets et les exécutables générés
clean:
	rm -f $(OBJ_CPP) $(OBJ_C) $(TARGET)

# Déclarations des phony targets pour éviter les conflits avec les fichiers portant les mêmes noms
.PHONY: all clean
