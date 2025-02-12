


#ifndef CONFIG_H
#define CONFIG_H

#include <string>

const std::string MQTT_BROKER_ADDRESS = "tcp://34.47.250.228:1883";
const std::string MQTT_CLIENT_ID = "wasm_executor";
const std::string TOPIC_STORE = "wasm/store";
const std::string TOPIC_START = "wasm/start";
const std::string STORAGE_PATH = "./storage/";

#endif

