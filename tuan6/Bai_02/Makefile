TARGET = firmware
DEBUG = 1
OPT = -Og

BUILD_DIR = Build
BIN_DIR = Bin

C_SOURCES = \
Src/main.c \
Src/stm32f1xx_it.c \
Src/stm32f1xx_hal_msp.c \
Src/system_stm32f1xx.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pwr.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim_ex.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_uart.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c \
Middlewares/Third_Party/FreeRTOS/croutine.c \
Middlewares/Third_Party/FreeRTOS/event_groups.c \
Middlewares/Third_Party/FreeRTOS/list.c \
Middlewares/Third_Party/FreeRTOS/queue.c \
Middlewares/Third_Party/FreeRTOS/stream_buffer.c \
Middlewares/Third_Party/FreeRTOS/tasks.c \
Middlewares/Third_Party/FreeRTOS/timers.c \
Middlewares/Third_Party/FreeRTOS/portable/MemMang/heap_4.c \
Middlewares/Third_Party/FreeRTOS/portable/GCC/ARM_CM3/port.c

ASM_SOURCES = Src/startup_stm32f103xb.s

PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
BIN = $(CP) -O binary -S

CPU = -mcpu=cortex-m3
MCU = $(CPU) -mthumb

C_DEFS = -DSTM32F103xB -DUSE_HAL_DRIVER

C_INCLUDES = \
-IInc \
-IDrivers/STM32F1xx_HAL_Driver/Inc \
-IDrivers/CMSIS/Device/ST/STM32F1xx/Include \
-IDrivers/CMSIS/Include \
-IMiddlewares/Third_Party/FreeRTOS/include \
-IMiddlewares/Third_Party/FreeRTOS/portable/GCC/ARM_CM3

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections
ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

LDSCRIPT = Linker/STM32F103C8.ld
LDFLAGS = $(MCU) -specs=nano.specs --specs=nosys.specs -T$(LDSCRIPT) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

all: $(BIN_DIR)/$(TARGET).bin

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BIN_DIR)/%.bin: $(BUILD_DIR)/$(TARGET).elf | $(BIN_DIR)
	$(BIN) $< $@

$(BUILD_DIR):
	mkdir -p $@

$(BIN_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

flash: all
	st-flash --reset write $(BIN_DIR)/$(TARGET).bin 0x08000000

.PHONY: all clean flash
