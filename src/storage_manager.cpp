


#include "storage_manager.h"
#include "config.h"
#include <fstream>

bool store_wasm_bytes(const std::string& id, const std::vector<uint8_t>& wasm_bytes) {
    std::ofstream file(STORAGE_PATH + id + ".wasm", std::ios::binary);
    if (!file) return false;
    file.write(reinterpret_cast<const char*>(wasm_bytes.data()), wasm_bytes.size());
    return true;
}

std::vector<uint8_t> load_wasm_bytes(const std::string& id) {
    std::ifstream file(STORAGE_PATH + id + ".wasm", std::ios::binary);
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

void clear_stored_data(const std::string& id) {
    std::remove((STORAGE_PATH + id + ".wasm").c_str());
}

