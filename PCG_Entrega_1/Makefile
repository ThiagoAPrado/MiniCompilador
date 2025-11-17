# Compilador e flags
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2 -Iinclude

# Diretórios do projeto
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Nome do executável
TARGET = $(BIN_DIR)/MiniCompilador

# Arquivos fonte
SRC = $(wildcard $(SRC_DIR)/*.cpp)

# Arquivos objeto gerados dentro de build/
OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC))

# Regra padrão
all: $(TARGET)

# Link final
$(TARGET): $(OBJ) | $(BIN_DIR)
	$(CXX) $(OBJ) -o $(TARGET)

# Compilar cada .cpp para .o dentro da pasta build/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Criar diretórios automáticos
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Executar
run: $(TARGET)
	./$(TARGET)

# Limpar tudo
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all clean run
