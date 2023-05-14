cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(BMP280)
# initialize the Raspberry Pi Pico SDK
pico_sdk_init()
#set(CMAKE_C_STANDARD 11)
#set(CMAKE_CXX_STANDARD 17)

# rest of your project
add_executable(main
    src/main.cpp
    src/BMP280.h
    src/BMP280.cpp
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(main pico_stdlib hardware_spi)

# enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(main)
