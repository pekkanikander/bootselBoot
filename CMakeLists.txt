cmake_minimum_required(VERSION 3.14)
project(bootselBoot C)
set(CMAKE_C_STANDARD 11)

# Include custom CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Find and configure libusb, from CMake modules
include(libusb-library)

# Define executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_executable(bootselBoot bootselBoot.c)
target_link_libraries(bootselBoot PRIVATE LIBUSB::LIBUSB)
