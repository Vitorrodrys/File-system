# Diretórios de origem e de objetos
SRCD = src
OBJD = obj

# Nome do binário de saída
BIN = filesystem.out

# Compilador e flags
CXX = g++
CXXFLAGS = -Wall -g -std=c++17  # Ajuste as flags conforme necessário

# Encontrar todos os arquivos .cpp recursivamente em SRCD
SRCS = $(shell find $(SRCD) -name '*.cpp')

# Gerar a lista de arquivos objeto (.o) correspondente
OBJS = $(SRCS:$(SRCD)/%.cpp=$(OBJD)/%.o)

# Regra default: compilar e linkar tudo
all: $(BIN)

# Regra para linkar o binário final
$(BIN): $(OBJS)
	$(CXX) $(OBJS) -o $(BIN)

# Regra para compilar os arquivos .cpp para .o
$(OBJD)/%.o: $(SRCD)/%.cpp
	@mkdir -p $(dir $@)  # Garante que o diretório obj/subdiretorios exista
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regra para limpar os arquivos gerados
clean:
	rm -rf $(OBJD) $(BIN)

# Regra para limpar apenas os arquivos objeto
clean-obj:
	rm -rf $(OBJD)/*.o

# Regra para recompilar tudo
rebuild: clean all
