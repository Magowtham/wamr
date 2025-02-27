#include <zephyr.h>
#include <sys/printk.h>
#include <net/mqtt.h>
#include <net/socket.h>
#include <zephyr/types.h>
#include <logging/log.h>
#include "wasm_export.h"

LOG_MODULE_REGISTER(wasm_mqtt, LOG_LEVEL_INF);

#define MQTT_BROKER_ADDR "34.47.250.228"
#define MQTT_BROKER_PORT 1883

#define TOPIC_START "wasm/start"
#define TOPIC_KILL "wasm/kill"

#define STACK_SIZE 8092
#define HEAP_SIZE 8092

static struct mqtt_client client;
static struct sockaddr_storage broker;
static struct k_thread wasm_thread;
static K_THREAD_STACK_DEFINE(wasm_stack, 8192);
static K_MUTEX_DEFINE(runtime_lock);

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

static WasmRuntimeCtx *runtime_ctx = NULL;

void wasm_execute(void *ctx, void *unused1, void *unused2) {
    WasmWorkloadCtx *workload = (WasmWorkloadCtx *)ctx;
    char error_buf[128];

    if (!wasm_runtime_init()) {
        LOG_ERR("Failed to initialize WAMR");
        return;
    }

    wasm_module_t module = wasm_runtime_load(workload->wasm_workload, workload->wasm_workload_size, error_buf, sizeof(error_buf));
    if (!module) {
        LOG_ERR("Failed to load AOT module: %s", error_buf);
        return;
    }

    wasm_module_inst_t module_inst = wasm_runtime_instantiate(module, HEAP_SIZE, 0, error_buf, sizeof(error_buf));
    if (!module_inst) {
        LOG_ERR("Failed to instantiate module: %s", error_buf);
        wasm_runtime_unload(module);
        return;
    }

    wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, HEAP_SIZE);
    if (!exec_env) {
        LOG_ERR("Failed to create execution environment");
        wasm_runtime_deinstantiate(module_inst);
        wasm_runtime_unload(module);
        return;
    }

    k_mutex_lock(&runtime_lock, K_FOREVER);
    runtime_ctx = k_malloc(sizeof(WasmRuntimeCtx));
    runtime_ctx->mod = module;
    runtime_ctx->mod_inst = module_inst;
    runtime_ctx->exec_env = exec_env;
    k_mutex_unlock(&runtime_lock);

    wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, workload->func_name);
    if (!func) {
        LOG_ERR("Failed to find function %s", workload->func_name);
    } else {
        uint32_t args[1] = {0};
        if (wasm_runtime_call_wasm(exec_env, func, 0, args)) {
            LOG_INF("Execution success");
        } else {
            LOG_ERR("Execution failed");
        }
    }
}

void wasm_kill(void) {
    if (!runtime_ctx) {
        LOG_INF("No WASM workload running");
        return;
    }

    k_mutex_lock(&runtime_lock, K_FOREVER);
    wasm_runtime_destroy_exec_env(runtime_ctx->exec_env);
    wasm_runtime_deinstantiate(runtime_ctx->mod_inst);
    wasm_runtime_unload(runtime_ctx->mod);
    wasm_runtime_destroy();
    k_free(runtime_ctx);
    runtime_ctx = NULL;
    k_mutex_unlock(&runtime_lock);

    LOG_INF("WASM workload terminated");
}

static void mqtt_event_handler(struct mqtt_client *client, const struct mqtt_evt *evt) {
    switch (evt->type) {
    case MQTT_EVT_PUBLISH: {
        struct mqtt_publish_param *p = &evt->param.publish;
        uint8_t payload[p->message.payload.len];
        mqtt_read_publish_payload(client, payload, p->message.payload.len);

        if (strncmp(p->message.topic.topic.utf8, TOPIC_START, p->message.topic.topic.size) == 0) {
            WasmWorkloadCtx *ctx = k_malloc(sizeof(WasmWorkloadCtx));
            ctx->wasm_workload = k_malloc(p->message.payload.len);
            memcpy(ctx->wasm_workload, payload, p->message.payload.len);
            ctx->wasm_workload_size = p->message.payload.len;
            ctx->func_name = "_start";

            k_thread_create(&wasm_thread, wasm_stack, K_THREAD_STACK_SIZEOF(wasm_stack), wasm_execute, ctx, NULL, NULL, 7, 0, K_NO_WAIT);
        } else if (strncmp(p->message.topic.topic.utf8, TOPIC_KILL, p->message.topic.topic.size) == 0) {
            wasm_kill();
        }
        break;
    }
    default:
        break;
    }
}

void main(void) {
    struct sockaddr_in *broker_addr = (struct sockaddr_in *)&broker;
    broker_addr->sin_family = AF_INET;
    broker_addr->sin_port = htons(MQTT_BROKER_PORT);
    net_addr_pton(AF_INET, MQTT_BROKER_ADDR, &broker_addr->sin_addr);

    mqtt_client_init(&client);
    client.broker = &broker;
    client.evt_cb = mqtt_event_handler;
    client.client_id.utf8 = "device2-execution-handler";
    client.client_id.size = strlen(client.client_id.utf8);
    client.transport.type = MQTT_TRANSPORT_NON_SECURE;

    if (mqtt_connect(&client) != 0) {
        LOG_ERR("Failed to connect to broker");
        return;
    }
    LOG_INF("Connected to broker");

    struct mqtt_topic topics[] = {
        {.topic = {.utf8 = TOPIC_START, .size = strlen(TOPIC_START)}},
        {.topic = {.utf8 = TOPIC_KILL, .size = strlen(TOPIC_KILL)}},
    };
    struct mqtt_subscription_list subs = {.list = topics, .list_count = ARRAY_SIZE(topics), .qos = MQTT_QOS_1};
    mqtt_subscribe(&client, &subs);

    while (1) {
        mqtt_input(&client);
        k_sleep(K_SECONDS(1));
    }
}

