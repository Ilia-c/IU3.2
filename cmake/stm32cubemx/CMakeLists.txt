cmake_minimum_required(VERSION 3.22)

project(stm32cubemx)
add_library(stm32cubemx INTERFACE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

target_compile_definitions(stm32cubemx INTERFACE 
	USE_HAL_DRIVER 
	STM32L476xx
    $<$<CONFIG:Debug>:DEBUG>
)

target_include_directories(stm32cubemx INTERFACE
    ../../Core/Inc
    ../../FATFS/Target
    ../../FATFS/App
    ../../USB_DEVICE/App
    ../../USB_DEVICE/Target
    ../../Drivers/STM32L4xx_HAL_Driver/Inc
    ../../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy
    ../../Middlewares/Third_Party/FreeRTOS/Source/include
    ../../Middlewares/Third_Party/FatFs/src
    ../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
    ../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
    ../../Middlewares/Third_Party/FatFs/src
    ../../Middlewares/ST/STM32_USB_Device_Library/Core/Inc
    ../../Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Inc
    ../../Drivers/CMSIS/Device/ST/STM32L4xx/Include
    ../../Drivers/CMSIS/Include
    ../../Library/Settings
    ../../Library/Display_i2c/
    ../../Library/Display_i2c/hardware
    ../../Library/Keyboard/
    ../../Library/Internal_per
    ../../Library/w25q128
    ../../Library/ADC
    ../../Library/SD_card
)

target_sources(stm32cubemx INTERFACE
    ../../Core/Src/main.c
    ../../Core/Src/freertos.c
    ../../Core/Src/stm32l4xx_it.c
    ../../Core/Src/stm32l4xx_hal_msp.c
    ../../Core/Src/stm32l4xx_hal_timebase_tim.c
    ../../FATFS/Target/bsp_driver_sd.c
    ../../FATFS/Target/sd_diskio.c
    ../../FATFS/Target/fatfs_platform.c
    ../../FATFS/App/fatfs.c
    ../../USB_DEVICE/App/usb_device.c
    ../../USB_DEVICE/App/usbd_desc.c
    ../../USB_DEVICE/App/usbd_storage_if.c
    ../../USB_DEVICE/Target/usbd_conf.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pcd.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pcd_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_usb.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ramfunc.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_gpio.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cortex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_exti.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_adc.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_adc_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rtc.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rtc_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_sdmmc.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_sd.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_sd_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart_ex.c
    ../../Core/Src/system_stm32l4xx.c
    ../../Middlewares/Third_Party/FatFs/src/diskio.c
    ../../Middlewares/Third_Party/FatFs/src/ff.c
    ../../Middlewares/Third_Party/FatFs/src/ff_gen_drv.c
    ../../Middlewares/Third_Party/FatFs/src/option/syscall.c
    ../../Middlewares/Third_Party/FreeRTOS/Source/croutine.c
    ../../Middlewares/Third_Party/FreeRTOS/Source/event_groups.c
    ../../Middlewares/Third_Party/FreeRTOS/Source/list.c
    ../../Middlewares/Third_Party/FreeRTOS/Source/queue.c
    ../../Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c
    ../../Middlewares/Third_Party/FreeRTOS/Source/tasks.c
    ../../Middlewares/Third_Party/FreeRTOS/Source/timers.c
    ../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c
    ../../Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
    ../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c
    ../../Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c
    ../../Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c
    ../../Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c
    ../../Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc.c
    ../../Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc_bot.c
    ../../Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc_data.c
    ../../Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc_scsi.c
    ../../Core/Src/sysmem.c
    ../../Core/Src/syscalls.c
    ../../startup_stm32l476xx.s
    ../../Library/Display_i2c/Display.c
    ../../Library/Display_i2c/hardware/OLED_Fonst.c
    ../../Library/Display_i2c/hardware/OLED_Icons.c
    ../../Library/Display_i2c/hardware/OLED.c
    ../../Library/Keyboard/Keyboard.c
    ../../Library/Settings/Settings.c
    ../../Library/Internal_per/RTC_data.c
    ../../Library/w25q128/w25q128.c
    ../../Library/ADC/MS5193T.c
    ../../Library/SD_card/SD.c
)

target_link_directories(stm32cubemx INTERFACE
)

target_link_libraries(stm32cubemx INTERFACE
)

# Validate that STM32CubeMX code is compatible with C standard
if(CMAKE_C_STANDARD LESS 11)
    message(ERROR "Generated code requires C11 or higher")
endif()


