

// Device2/src/wasm_runner.cpp
#include "wamr_wrapper.h"

bool wamr_initialize_runtime() {
    return wasm_runtime_init();
}



wasm_module_inst_t wamr_load_module(std::vector<uint8_t> &wasm_bytes, wasm_module_t &module) {
    
    uint8_t *buffer = wasm_bytes.data();
    uint32_t wasm_size = wasm_bytes.size();
    char error_buf[128];
    module = wasm_runtime_load(buffer, wasm_size, error_buf, sizeof(error_buf));
    if (!module) {
        printf("Failed to load WASM module: %s\n", error_buf);
        free(buffer);
        return nullptr;
    }

    wasm_module_inst_t module_inst = wasm_runtime_instantiate(module, 8092, 8092, error_buf, sizeof(error_buf));
    if (!module_inst) {
        printf("Failed to instantiate WASM module: %s\n", error_buf);
        wasm_runtime_unload(module);
        free(buffer);
        return nullptr;
    }

    return module_inst;
}

bool wamr_execute_function(wasm_module_inst_t module_inst, const char *func_name, int arg) {
    wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, func_name);
    if (!func) {
        printf("Function %s not found!\n", func_name);
        return false;
    }

    wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 8092);
    if (!exec_env) {
        printf("Failed to create execution environment\n");
        return false;
    }

    uint32_t argv[1] = {arg};
    if (!wasm_runtime_call_wasm(exec_env, func, 1, argv)) {
        printf("Error: %s\n", wasm_runtime_get_exception(module_inst));
        wasm_runtime_destroy_exec_env(exec_env);
        return false;
    }

    printf("Function %s returned: %d\n", func_name, argv[0]);
    wasm_runtime_destroy_exec_env(exec_env);
    return true;
}

/*

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <wasm_file>\n", argv[0]);
        return -1;
    }

    if (!wamr_initialize_runtime()) {
        printf("Failed to initialize WAMR\n");
        return -1;
    }

    wasm_module_t module;
    wasm_module_inst_t module_inst = wamr_load_module(argv[1], module);
    if (!module_inst) {
        printf("Failed to load WASM module\n");
        return -1;
    }

    wamr_execute_function(module_inst, "fib", 8);

    wasm_runtime_deinstantiate(module_inst);
    wasm_runtime_unload(module);
    wasm_runtime_destroy();
    return 0;
}
*/
