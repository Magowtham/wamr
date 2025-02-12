




#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <string>
#include <functional>

class MQTTClient {
public:
    MQTTClient();
    void subscribe(const std::string& topic, std::function<void(const std::string&)> callback);
    void publish(const std::string& topic, const std::string& message);
    void loop();
};

#endif

