


#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <cstdint>

bool store_wasm_bytes(const std::string& id, const std::vector<uint8_t>& wasm_bytes);
std::vector<uint8_t> load_wasm_bytes(const std::string& id);
void clear_stored_data(const std::string& id);

#endif

