# Running a WAMR-based C Program on Zephyr

## Introduction to Zephyr

[Zephyr](https://zephyrproject.org/) is a lightweight, real-time operating system (RTOS) designed for embedded systems. It supports multiple architectures and provides a rich set of APIs for networking, device management, and multithreading. Unlike traditional Linux-based systems, Zephyr does not have a full POSIX environment, requiring adaptations when porting applications from Linux to Zephyr.

### Differences Between Standard Linux APIs and Zephyr APIs

When porting a C program from Linux to Zephyr, we need to replace standard POSIX APIs with their Zephyr equivalents. Below are key differences:

| Feature         | Linux (POSIX) API | Zephyr API |
|---------------|----------------|------------|
| Threads       | `pthread_create()` | `k_thread_create()` |
| Memory Allocation | `malloc()/free()` | `k_malloc()/k_free()` |
| Logging       | `printf()` | `LOG_INF()` |
| Mutexes       | `pthread_mutex_t` | `struct k_mutex` |
| Networking (MQTT) | `paho-mqtt` (POSIX sockets) | `net/mqtt.h` (Zephyr native) |
| Delays        | `sleep()` | `k_sleep()` |

Since Zephyr does not support traditional `main()` entry points like Linux, we need to define our own initialization function using `SYS_INIT()` or create a Zephyr thread for execution.

## Setting Up Zephyr for Development on Linux

To run our program on Zephyr, we first need to install Zephyr dependencies and set up an emulation environment for testing.

### Prerequisites

1. Install Zephyr dependencies:
   ```sh
   sudo apt update && sudo apt install --no-install-recommends cmake ninja-build gperf ccache dfu-util python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file make gcc git curl
   ```

2. Install Zephyr SDK:
   ```sh
   wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.3/zephyr-sdk-0.16.3_linux-x86_64.tar.xz
   tar xvf zephyr-sdk-0.16.3_linux-x86_64.tar.xz -C $HOME
   export ZEPHYR_SDK_INSTALL_DIR=$HOME/zephyr-sdk-0.16.3
   $ZEPHYR_SDK_INSTALL_DIR/setup.sh
   ```

3. Initialize Zephyr environment:
   ```sh
   west init -m https://github.com/zephyrproject-rtos/zephyr zephyrproject
   cd zephyrproject
   west update
   west zephyr-export
   source zephyr-env.sh
   ```

4. Install Python dependencies:
   ```sh
   pip3 install -r zephyr/scripts/requirements.txt
   ```

### Running Zephyr on Linux Using QEMU

Zephyr provides an emulator (QEMU) for running and testing Zephyr applications without physical hardware. To simulate Zephyr:

```sh
west build -p auto -b qemu_x86 samples/hello_world -- -DCONF_FILE=prj.conf
west run
```

This should print `Hello World!` from Zephyr in the terminal, confirming that the environment is set up correctly.

## Building WAMR for Zephyr

Since Zephyr is an embedded RTOS, we need to cross-compile the WebAssembly Micro Runtime (WAMR) for Zephyr. Follow these steps:

1. Clone WAMR:
   ```sh
   git clone https://github.com/bytecodealliance/wasm-micro-runtime.git
   cd wasm-micro-runtime
   ```

2. Build WAMR for Zephyr:
   ```sh
   cd product-mini/platforms/zephyr
   west build -p auto -b qemu_x86
   west flash
   ```

This compiles WAMR for Zephyr and prepares it to run WebAssembly workloads.

## Building and Running Our C Program on Zephyr

To compile and run our WAMR-based C program on Zephyr:

1. Copy your modified Zephyr-compatible C code to a new application directory inside `zephyrproject`.
2. Create a `CMakeLists.txt` file:
   ```cmake
   cmake_minimum_required(VERSION 3.13.1)
   find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
   project(wamr_zephyr)
   FILE(GLOB app_sources src/*.c)
   target_sources(app PRIVATE ${app_sources})
   ```
3. Create a `prj.conf` file:
   ```ini
   CONFIG_STDOUT_CONSOLE=y
   CONFIG_NETWORKING=y
   CONFIG_NET_IPV4=y
   CONFIG_MQTT_LIB=y
   ```
4. Build and run:
   ```sh
   west build -p auto -b qemu_x86
   west run
   ```

### Flashing to a Physical Device

If you want to flash the application to an actual embedded board (e.g., STM32, NRF52, ESP32), replace `qemu_x86` with your board name:
```sh
west build -p auto -b <your-board>
west flash
```

## Conclusion

This README provides a complete guide to porting a Linux-based C program to Zephyr, including:
- Differences in APIs between Linux and Zephyr.
- Setting up Zephyr development tools and dependencies.
- Running Zephyr applications using QEMU for Linux-based emulation.
- Building WAMR for Zephyr.
- Compiling and running our MQTT-enabled WASM execution program on Zephyr.

With this setup, you can now develop and test your WAMR-based application in an embedded environment powered by Zephyr! 🚀


