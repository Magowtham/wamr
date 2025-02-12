#include "mqtt_client.h"


#include "config.h"
#include <mqtt/async_client.h>
#include <iostream>

mqtt::async_client client(MQTT_BROKER_ADDRESS, MQTT_CLIENT_ID);
std::function<void(const std::string&)> message_callback;

MQTTClient::MQTTClient() {
    mqtt::connect_options connOpts;
    connOpts.set_keep_alive_interval(20);
    connOpts.set_clean_session(true);
    
    try {
        client.connect(connOpts)->wait();
        std::cout << "Connected to MQTT Broker" << std::endl;
    } catch (const mqtt::exception& e) {
        std::cerr << "MQTT Connection Failed: " << e.what() << std::endl;
    }
}

void MQTTClient::subscribe(const std::string& topic, std::function<void(const std::string&)> callback) {
    message_callback = callback;
    client.subscribe(topic, 1);
}

void MQTTClient::publish(const std::string& topic, const std::string& message) {
    client.publish(topic, message.c_str(), message.size(), 1, false);
}

void MQTTClient::loop() {
    while (true) {
        auto msg = client.consume_message();
        if (msg && message_callback) {
            message_callback(msg->to_string());
        }
    }
}

