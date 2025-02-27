# MQTT WASM Publisher

## Overview
This project is a simple **MQTT Publisher** written in **C** that reads a **WebAssembly AOT compiled binary file** and publishes it to an **MQTT Broker** over a specified topic. It uses the **Eclipse Paho MQTT C Client** for MQTT communication.

## Features
- Reads a WASM **AOT compiled** file from disk.
- Publishes the file as a payload to an **MQTT broker**.
- Supports customizable MQTT topics like `wasm/start` and `wasm/kill`.
- Uses **Eclipse Paho MQTT C library** for message transmission.

---

## Prerequisites
Before building and running this project, ensure you have the necessary tools and dependencies installed.

### 1. Install Make (if not installed)
Make is required to compile the project.

```sh
sudo apt update
sudo apt install make -y
```

### 2. Install GCC (if not installed)
GCC is required to compile the C source code.

```sh
sudo apt install gcc -y
```

### 3. Install Eclipse Paho MQTT C Client Library
This project requires the **Paho MQTT C client** library.

```sh
sudo apt install libpaho-mqtt3c-dev -y
```

Alternatively, you can install it from source:

```sh
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c
make
sudo make install
```

---

## Build Instructions
Compile the `pub.c` source file and generate the binary inside the `build/` directory using Make.

```sh
make
```

---

## Usage
Once compiled, the binary can be run as follows:

```sh
./build/pub <wasm-aot-file-path> <topic>
```

### Example Usage
```sh
./build/pub test1 wasm/start
```

- **`test1`** → The path to the AOT compiled WASM file.
- **`wasm/start`** → The MQTT topic to publish the file to.

### Available Topics
- **`wasm/start`** → Starts execution of the WASM module.
- **`wasm/kill`** → Stops execution of the WASM module.

---

## How It Works
1. The program reads the WASM file into memory.
2. It establishes an MQTT connection with the broker at `tcp://34.47.250.228:1883`.
3. The WASM file is sent as a **single MQTT message** to the given topic.
4. Once published, the client disconnects and frees allocated memory.

---

## Error Handling
If any error occurs during execution, the program prints an appropriate error message. 

Possible errors include:
- **File not found** → Ensure the WASM file path is correct.
- **Memory allocation failure** → Check system memory availability.
- **MQTT Connection failure** → Verify the broker address and network connection.

---

## Cleanup
To remove the compiled binary:
```sh
make clean
```

---

## License
This project is open-source and free to use under the **MIT License**.


