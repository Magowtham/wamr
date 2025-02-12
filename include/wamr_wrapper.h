
// Device2/include/wamr_wrapper.h
#ifndef WAMR_WRAPPER_H
#define WAMR_WRAPPER_H

#include <wasm_export.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdint>
#include<vector>

// Function to initialize WAMR runtime
bool wamr_initialize_runtime();

// Function to load and instantiate a WASM module
wasm_module_inst_t wamr_load_module(std::vector<uint8_t> &wasm_bytes, wasm_module_t &module);

// Function to execute a WASM function
bool wamr_execute_function(wasm_module_inst_t module_inst, const char *func_name, int arg);

#endif // WAMR_WRAPPER_H

