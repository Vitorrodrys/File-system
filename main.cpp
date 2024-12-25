#include <iostream>
#include <unordered_map>
#include <vector>
#include <cstring>  // Para copiar os dados

std::vector<char> serialize_to_bytes(const std::unordered_map<Key, Value>& map) {
    std::vector<char> bytes;

    for (const auto& pair : map) {
        // Serializando a chave
        size_t key_size = sizeof(std::string);
        bytes.insert(bytes.end(), reinterpret_cast<const char*>(&key_size), reinterpret_cast<const char*>(&key_size + sizeof(std::string)));
        bytes.insert(bytes.end(), reinterpret_cast<const char*>(&pair.first), reinterpret_cast<const char*>(&pair.first + sizeof(std::string)));

        // Serializando o valor
        size_t value_size = sizeof(Value);
        bytes.insert(bytes.end(), reinterpret_cast<const char*>(&value_size), reinterpret_cast<const char*>(&value_size + sizeof(Value)));
        bytes.insert(bytes.end(), reinterpret_cast<const char*>(&pair.second), reinterpret_cast<const char*>(&pair.second + sizeof(Value)));
    }

    return bytes;
}

int main() {
    std::unordered_map<std::string, int> hash_map;
    hash_map["/home"] = 1;
    hash_map["/home/vitor"] = 2;
    hash_map["/home/seila"] = 3;

    std::vector<char> serialized_bytes = serialize_to_bytes(hash_map);

    std::cout << "Tabela hash serializada para bytes: ";
    for (char byte : serialized_bytes) {
        std::cout << std::hex << (int)(unsigned char)byte << " ";
    }
    std::cout << std::endl;

    return 0;
}
