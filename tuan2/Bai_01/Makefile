TARGET = firmware
BUILD_DIR = Build
BIN_DIR = Bin

CC = arm-none-eabi-gcc
CP = arm-none-eabi-objcopy
SZ = arm-none-eabi-size

CPU = -mcpu=cortex-m3
MCU = $(CPU) -mthumb

C_SOURCES = \
Src/main.c \
Src/stm32f1xx_hal_msp.c \
Src/stm32f1xx_it.c \
Src/system_stm32f1xx.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_uart.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c

ASM_SOURCES = \
Src/startup_stm32f103xb.s

C_DEFS = -DSTM32F103xB

C_INCLUDES = \
-IInc \
-IDrivers/CMSIS/Device/ST/STM32F1xx/Include \
-IDrivers/CMSIS/Include \
-IDrivers/STM32F1xx_HAL_Driver/Inc

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) -O0 -Wall -fdata-sections -ffunction-sections

LDSCRIPT = Linker/STM32F103C8.ld
LDFLAGS = $(MCU) -specs=nano.specs --specs=nosys.specs -T$(LDSCRIPT) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--gc-sections

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

all: $(BIN_DIR)/$(TARGET).elf $(BIN_DIR)/$(TARGET).bin

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	$(CC) -x assembler-with-cpp -c $(CFLAGS) $< -o $@

$(BIN_DIR)/$(TARGET).elf: $(OBJECTS) | $(BIN_DIR)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BIN_DIR)/%.bin: $(BIN_DIR)/%.elf
	$(CP) -O binary -S $< $@

$(BUILD_DIR):
	mkdir -p $@

$(BIN_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

flash: all
	st-flash write $(BIN_DIR)/$(TARGET).bin 0x8000000

.PHONY: all clean flash
