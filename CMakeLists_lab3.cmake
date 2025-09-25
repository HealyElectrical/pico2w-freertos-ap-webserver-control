# ELEX 7820 Lab 3 CMakeLists_lab3.cmake

target_link_libraries(lab3
    pico_cyw43_arch_lwip_sys_freertos
    FreeRTOS-Kernel-Heap4 )

pico_configure_ip4_address(lab3 PRIVATE
    CYW43_DEFAULT_IP_AP_ADDRESS 192.168.4.1 )
    
set(FREERTOS_KERNEL_PATH "C:/Users/Ilove/OneDrive - BCIT/Documents/BCIT/term 7/RTES_7820/Lab/lab2c/FreeRTOS-Kernel")
include(FreeRTOS_Kernel_import.cmake)