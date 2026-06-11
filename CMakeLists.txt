cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(hw6_mouse C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(hw6_mouse
    main.c
    usb_descriptors.c
    mpu6050.c
)

target_include_directories(hw6_mouse PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(hw6_mouse
    pico_stdlib
    hardware_i2c
    hardware_gpio
    tinyusb_device
    tinyusb_board
    m
)

pico_enable_stdio_uart(hw6_mouse 0)
pico_enable_stdio_usb(hw6_mouse 0)

