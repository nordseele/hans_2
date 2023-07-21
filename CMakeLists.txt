cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w)
include(pico_sdk_import.cmake)

set(target_proj hans_2)
project(${target_proj} C CXX ASM)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wunused-parameter -ffunction-sections -fdata-sections -Wl,--gc-sections, -Wno-unused-variable")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(${target_proj}
    main.c
    dhcpserver/dhcpserver.c
    dnsserver/dnsserver.c
    midi_func.c
    helpers.c
)

target_include_directories(${target_proj} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        ${CMAKE_CURRENT_LIST_DIR}/dhcpserver
        ${CMAKE_CURRENT_LIST_DIR}/dnsserver
)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/pio_midi_uart_lib)

target_link_options(${target_proj} PRIVATE -Xlinker --print-memory-usage)
target_compile_options(${target_proj} PRIVATE -Wall -Wextra -DCFG_TUSB_DEBUG=0)
target_link_libraries(${target_proj} pico_stdlib pio_midi_uart_lib hardware_i2c pico_cyw43_arch_lwip_threadsafe_background)
pico_add_extra_outputs(${target_proj})


pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

