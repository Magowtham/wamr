cmake_minimum_required(VERSION 3.10)
project(MyWasmProject)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define source and include directories
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(WAMR_DIR ${CMAKE_SOURCE_DIR}/wamr/wamr_source)
set(WAMR_LIB ${WAMR_DIR}/build/libiwasm.so)
set(MQTT_C_DIR ${CMAKE_SOURCE_DIR}/mqtt/mqtt_c)
set(MQTT_CPP_DIR ${CMAKE_SOURCE_DIR}/mqtt/mqtt_cpp)

# Add include directories
include_directories(${INCLUDE_DIR})
include_directories(${WAMR_DIR}/core/iwasm/include)
include_directories(${WAMR_DIR}/core/shared/utils)
include_directories(${WAMR_DIR}/core/iwasm/libraries/libc/include)
include_directories(${MQTT_C_DIR}/include)
include_directories(${MQTT_CPP_DIR}/include)

# Find all .cpp files in src/
file(GLOB SOURCES ${SRC_DIR}/*.cpp)

# Create the executable
add_executable(main ${SOURCES})

# Link WAMR runtime
target_link_libraries(main ${WAMR_LIB}  m)
target_link_libraries(main ${MQTT_C_DIR}/lib/libpaho-mqtt3c.so ${MQTT_C_DIR}/lib/libpaho-mqtt3a.so ${MQTT_CPP_DIR}/lib/libpaho-mqttpp3.so pthread m)
