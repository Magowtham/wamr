#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTClient.h>

#define MQTT_ADDRESS   "tcp://34.47.250.228:1883"  // MQTT Broker
#define MQTT_CLIENTID  "c_publisher"              // Unique Client ID
#define MQTT_QOS       1
#define MQTT_TIMEOUT   10000L                      // Timeout in milliseconds

// Function to read the entire file into memory
int read_wasm_file(const char *file_path, unsigned char **buffer, size_t *size) {
    FILE *file = fopen(file_path, "rb");  // Open file in binary mode
    if (!file) {
        perror("Failed to open file");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    *size = ftell(file);  // Get file size
    rewind(file);

    *buffer = (unsigned char *)malloc(*size);
    if (!*buffer) {
        perror("Memory allocation failed");
        fclose(file);
        return -1;
    }

    if (fread(*buffer, 1, *size, file) != *size) {
        perror("Failed to read file");
        free(*buffer);
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}

int main(int argc,char **args) {
     
    if(argc < 3 ) {
	    printf("Usage ./main <wasm-aot-file-path> <topic>\n");
	    printf("Topics: wasm/start, wasm/kill");
	    return 0;
    }

    const char *file_path = args[1];  // Path to the AOT wasm file
    char *topic = args[2];
    unsigned char *wasm_buffer = NULL;
    size_t wasm_size = 0;

    if (read_wasm_file(file_path, &wasm_buffer, &wasm_size) != 0) {
        fprintf(stderr, "Error reading AOT file.\n");
        return EXIT_FAILURE;
    }

    // Initialize MQTT client
    MQTTClient client;
    MQTTClient_create(&client, MQTT_ADDRESS, MQTT_CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    
    // Set MQTT options with increased packet size
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    
    if (MQTTClient_connect(client, &conn_opts) != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "Failed to connect to MQTT broker.\n");
        free(wasm_buffer);
        return EXIT_FAILURE;
    }

    printf("Connected to MQTT broker. Publishing %zu bytes...\n", wasm_size);

    // Prepare and send the full file as one MQTT message
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = wasm_buffer;
    pubmsg.payloadlen = (int)wasm_size;
    pubmsg.qos = MQTT_QOS;
    pubmsg.retained = 0;

    MQTTClient_deliveryToken token;
    if (MQTTClient_publishMessage(client,topic, &pubmsg, &token) != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "Failed to publish message.\n");
        free(wasm_buffer);
        MQTTClient_disconnect(client, 1000);
        MQTTClient_destroy(&client);
        return EXIT_FAILURE;
    }

    MQTTClient_waitForCompletion(client, token, MQTT_TIMEOUT);
    printf("File published successfully!\n");

    // Cleanup
    free(wasm_buffer);
    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);

    return EXIT_SUCCESS;
}

