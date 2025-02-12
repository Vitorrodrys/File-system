# Implementação de Sistema de Arquivos com FUSE em C++

Este projeto consiste na implementação de um sistema de arquivos em **user space**, utilizando a API do **FUSE** em **C++**.

## Compilação

Para compilar o projeto, abra o terminal na pasta do projeto e execute:

```sh
make all
```

O processo de compilação gerará um arquivo de saída chamado **filesystem.out**.

## Execução

Para executar o sistema de arquivos, utilize o seguinte comando:

```sh
DISK_FILE=/path/to/vdisk DISK_SIZE=4294967296 ./filesystem.out /mnt/mount_path -f -s -d
```

### Parâmetros:
- **DISK_FILE**: Variável de ambiente que especifica o arquivo do disco virtual a ser utilizado pelo sistema de arquivos.
- **DISK_SIZE**: Variável de ambiente que define o tamanho do disco virtual.

Certifique-se de substituir os caminhos conforme a sua configuração local.

