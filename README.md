# WASM Execution Manager

## Overview
This project is a lightweight WASM execution manager designed to receive, load, and execute WebAssembly (WASM) workloads dynamically. The execution is managed using the WebAssembly Micro Runtime (WAMR) and controlled through messaging. The system supports starting and stopping WASM workloads efficiently in an embedded or cloud environment.

## Features
- Receives and stores WASM workloads dynamically.
- Loads and executes WASM workloads using WAMR.
- Supports runtime execution control.
- Threaded execution model for non-blocking operations.

## Prerequisites
Before setting up and running this project, ensure you have the following dependencies installed on your system:

- **GCC** (for compiling the C source code)
- **CMake** (for building WAMR)
- **Make** (for build automation)
- **WebAssembly Micro Runtime (WAMR)** (for executing WASM workloads)
- **Paho MQTT C Library** (for message handling)

### Install Dependencies on Linux
```sh
sudo apt update && sudo apt install -y gcc cmake make libpaho-mqtt3c-dev
```

## Setting Up the Project

### Clone the Repository
```sh
git clone <your-repository-url>
cd <your-repository-name>
```

### Build and Install WAMR
```sh
mkdir -p wamr_runtime && cd wamr_runtime
git clone --recursive https://github.com/bytecodealliance/wasm-micro-runtime.git
cd wasm-micro-runtime/product-mini/platforms/linux
mkdir build && cd build
cmake .. && make -j$(nproc)
cd ../../../../..
```

### Build the WASM Execution Manager
```sh
make
```

This will compile the project and generate the executable in the `build/` directory.

## Running the Project
To start the execution manager, run:
```sh
./build/main
```

The program will wait for incoming WASM workloads and execute them dynamically.

## Project Structure
```
.
├── src/            # Source code files
├── include/        # Header files
├── wamr_runtime/   # WebAssembly Micro Runtime
├── build/          # Compiled binaries and object files
├── Makefile        # Build system
└── README.md       # Project documentation
```

## Execution Flow
1. The system initializes WAMR.
2. It listens for incoming WASM workloads.
3. Upon receiving a workload, it:
   - Loads the WASM module.
   - Instantiates the module.
   - Creates an execution environment.
   - Executes the specified function.
4. Workloads can be stopped on request.

## Troubleshooting
- **Build Errors?** Ensure all dependencies are installed and WAMR is properly built.
- **Execution Fails?** Check that your WASM module is compiled correctly for WAMR.
- **Missing Libraries?** Ensure `libpaho-mqtt3c` and WAMR's shared libraries are linked correctly.

## Future Enhancements
- Improved workload management and logging.
- Support for multiple concurrent executions.
- Additional runtime controls.

---
Feel free to contribute or raise issues if you encounter problems!


