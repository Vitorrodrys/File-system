FUSE_ROOT_DIR = /usr/include/fuse3

FUSE3_CFLAGS = $(shell pkg-config --cflags fuse3)
FUSE3_LIBS = $(shell pkg-config --libs fuse3)

# Diretórios de origem e de objetos
SRCD = src
OBJD = obj

# Nome do binário de saída
BIN = filesystem.out

# Compilador e flags
CXX = g++
CXXFLAGS = -Wall -g -std=c++23 -D_FILE_OFFSET_BITS=64 $(FUSE3_CFLAGS)
LDFLAGS = $(FUSE3_LIBS)


# Encontrar todos os arquivos .cpp recursivamente em SRCD
SRCS = $(shell find $(SRCD) -name '*.cpp')

# Gerar a lista de arquivos objeto (.o) correspondente
OBJS = $(SRCS:$(SRCD)/%.cpp=$(OBJD)/%.o)

# Regra default: compilar e linkar tudo
all: $(BIN)

# Regra para linkar o binário final
$(BIN): $(OBJS)
	$(CXX) $(OBJS) -o $(BIN) $(LDFLAGS)

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