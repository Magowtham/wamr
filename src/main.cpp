




#include "mqtt_client.h"
#include "storage_manager.h"
#include "wamr_wrapper.h"


MQTTClient mqttClient;

void onWasmStore(const std::string& msg) {
    store_wasm_bytes("wasm1", std::vector<uint8_t>(msg.begin(), msg.end()));
}

void onWasmStart(const std::string& msg) {
    
    if (!wamr_initialize_runtime()) {
        printf("Failed to initialize WAMR\n");
        return;
    }

    std::vector<uint8_t> wasm_bytes = load_wasm_bytes("wasm1");
        
    wasm_module_t module;
    wasm_module_inst_t module_inst = wamr_load_module(wasm_bytes, module);
    if (!module_inst) {
        printf("Failed to load WASM module\n");
        return;
    }

    wamr_execute_function(module_inst, "hello", 8);

    wasm_runtime_deinstantiate(module_inst);
    wasm_runtime_unload(module);
    wasm_runtime_destroy();

    //mqttClient.publish("wasm/result", std::to_string(exit_code));
}



int main() {

    mqttClient.subscribe("wasm/store", onWasmStore);
    mqttClient.subscribe("wasm/start", onWasmStart);
    mqttClient.loop();

   

    printf("hello world");
    return 0;
}

