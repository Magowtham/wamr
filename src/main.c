#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include "MQTTClient.h"
#include <string.h>
#include<unistd.h>
#include "wasm_export.h"

#define ADDRESS     "tcp://34.47.250.228:1883"  // Broker address
#define CLIENTID    "/device2/execution/handler"
#define QOS         1
#define TIMEOUT     10000L  // Timeout in milliseconds

// Topics
#define TOPIC_START "wasm/start"
#define TOPIC_KILL  "wasm/kill"

#define BUFFER_SIZE  233608

#define STACK_SIZE 8092
#define HEAP_SIZE 8092

typedef struct {
        uint8_t *wasm_workload;
        uint32_t wasm_workload_size;
        char *func_name;
} WasmWorkloadCtx;

typedef struct {
        wasm_module_t mod;
        wasm_module_inst_t mod_inst;
        wasm_exec_env_t exec_env;
} WasmRuntimeCtx;

WasmRuntimeCtx *runtime_ctx=NULL;

int read_wasm_file(const char *file_path, unsigned char **buffer, size_t *size) {
    FILE *file = fopen(file_path, "rb"); // Open file in binary mode
    if (!file) {
        perror("Failed to open file");
        return -1;
    }

    fseek(file, 0, SEEK_END);  // Move to end of file
    *size = ftell(file);       // Get file size
    rewind(file);              // Move back to beginning

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

void* wasm_execute(void *params) {

    WasmWorkloadCtx *ctx = (WasmWorkloadCtx*)params;

    wasm_module_t module;
    wasm_module_inst_t module_inst;
    wasm_function_inst_t func;
    wasm_exec_env_t exec_env;
    char error_buf[128];

    /* Initialize WAMR */
    if (!wasm_runtime_init()) {
        printf("Failed to initialize WAMR\n");
        return NULL;
    }

    /*
    const char *file_path = "./test1";
    unsigned char *wasm_bytes;
    size_t wasm_size;

    int status =  read_wasm_file(file_path, &wasm_bytes, &wasm_size);

    if(status != 0) {
 	printf("failed to read file");
	return NULL;
    }

    */


    
    uint8_t *buffer = ctx->wasm_workload;
    uint32_t size = ctx->wasm_workload_size;
    char *func_name = ctx->func_name;

    printf("wasm bytes recieved -> %u\n",size);
/*
    uint8_t *buffer = (uint8_t* )wasm_bytes;
    uint32_t size = wasm_size;
    char *func_name = "_start";
*/
    module = wasm_runtime_load(buffer, size, error_buf, sizeof(error_buf));

    if (!module) {
        printf("Failed to load AOT module: %s\n", error_buf);
        return NULL;
    }

    /* Instantiate the module */
    module_inst = wasm_runtime_instantiate(module, HEAP_SIZE, 0, error_buf, sizeof(error_buf));
    if (!module_inst) {
        printf("Failed to instantiate module: %s\n", error_buf);
        wasm_runtime_unload(module);
        return NULL;
    }

    /* Create an execution environment */
    exec_env = wasm_runtime_create_exec_env(module_inst, HEAP_SIZE);
    if (!exec_env) {
        printf("Failed to create execution environment\n");
        wasm_runtime_deinstantiate(module_inst);
        wasm_runtime_unload(module);
        return NULL;
    }

    //updating global variable
    runtime_ctx = (WasmRuntimeCtx*)malloc(sizeof(WasmRuntimeCtx));
    
    runtime_ctx->mod=module;
    runtime_ctx->mod_inst=module_inst;
    runtime_ctx->exec_env=exec_env;

    free(ctx);

    /* Lookup the function */
    func = wasm_runtime_lookup_function(module_inst, func_name);
    if (!func) {
        printf("Failed to find function %s\n",func_name);
    } else {
        uint32_t args[1] = {0};  // No arguments for 'main'
        if (wasm_runtime_call_wasm(exec_env, func, 0, args)) {
            printf("Execution success\n");
        } else {
            printf("Execution failed\n");
        }
    }


    return NULL;
}


void wasm_kill(WasmRuntimeCtx *ctx) {
    wasm_runtime_destroy_exec_env(ctx->exec_env);
    wasm_runtime_deinstantiate(ctx->mod_inst);
    wasm_runtime_unload(ctx->mod);
    wasm_runtime_destroy();
    free(ctx);
}



int message_handler(void *context, char *topic_name, int topic_len, MQTTClient_message *message) {

    if(strcmp(topic_name,TOPIC_START) == 0) {	   

	pthread_t wasm_thread;

   	WasmWorkloadCtx *ctx = (WasmWorkloadCtx *)malloc(sizeof(WasmWorkloadCtx));

	ctx->wasm_workload = (uint8_t *)malloc(message->payloadlen);
        if (!ctx->wasm_workload) {
            printf("Failed to allocate memory for WASM bytes\n");
            free(ctx);
            return 1;
        }

        memcpy(ctx->wasm_workload, message->payload, message->payloadlen);

	ctx->wasm_workload_size=message->payloadlen;
    	ctx->func_name="_start";

	if (pthread_create(&wasm_thread, NULL,wasm_execute,(void *)ctx) != 0) {
        	printf("Failed to create WASM thread\n");
        	return 1;
    	}

    } else if(strcmp(topic_name,TOPIC_KILL) == 0) {
	if(!runtime_ctx) {
 		printf("WASM workload is not running\n");
		return 1;
	}
	wasm_kill(runtime_ctx);
	runtime_ctx=NULL;
    } else {
	printf("unknown topic: %s\n",topic_name);
    }

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topic_name);

    return 1;  
}





int main() {

	MQTTClient client;

	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

	//mqtt client
    	MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	
	//set connection options
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;


	// Connect to the broker
    	if (MQTTClient_connect(client, &conn_opts) != MQTTCLIENT_SUCCESS) {
        	printf("Failed to connect to broker\n");
        	return -1;
    	} else {
        	printf("Connected to broker successfully\n");
    	}


	// Set callback for message arrival
    	MQTTClient_setCallbacks(client, NULL, NULL, message_handler, NULL);


    	// Subscribe to topics


    	if (MQTTClient_subscribe(client, TOPIC_START, QOS) != MQTTCLIENT_SUCCESS) {
        	printf("Failed to subscribe to %s\n", TOPIC_START);
        	return -1;
   	}
        
	printf("Subscribed to topic: %s\n", TOPIC_START);

    	if (MQTTClient_subscribe(client, TOPIC_KILL, QOS) != MQTTCLIENT_SUCCESS) {
        	printf("Failed to subscribe to %s\n", TOPIC_KILL);
        	return -1;
   	}
    	
	printf("Subscribed to topic: %s\n", TOPIC_KILL);



	printf("waiting for the incoming messages...\n");

	    // Variables to receive message details
    	char* topic_name = NULL;
    	int topic_len;
    	MQTTClient_message* message = NULL;

	while(1) {
		int rc = MQTTClient_receive(client, &topic_name, &topic_len, &message, TIMEOUT);
		
		if (rc == MQTTCLIENT_SUCCESS && message != NULL) {
            		message_handler(NULL, topic_name, topic_len, message);  // Invoke the callback function
        	} else if (rc != MQTTCLIENT_SUCCESS) {
            		printf("Error receiving message...\n");
        	}

		sleep(1);

	}


	// Disconnect from the broker
    	MQTTClient_disconnect(client, 10000);
    	MQTTClient_destroy(&client);

    	return 0;


}


