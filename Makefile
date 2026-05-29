# Configuración del Compilador
CC = gcc
CFLAGS = -Wall -Wextra -pthread -Iinclude

# Directorios de trabajo
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

# Archivos fuente y objetos correspondientes
SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/queue.c $(SRC_DIR)/cpu.c $(SRC_DIR)/metrics.c
OBJECTS = $(OBJ_DIR)/main.o $(OBJ_DIR)/queue.o $(OBJ_DIR)/cpu.o $(OBJ_DIR)/metrics.o
TARGET = minikernel

# Regla principal: Compila todo el ejecutable
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

# Regla genérica para compilar archivos .c a .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Limpieza del proyecto
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean
