cmake_minimum_required(VERSION 3.12)

include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(pio-display)

pico_sdk_init()

add_subdirectory(./src)
