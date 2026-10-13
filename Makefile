CC := arm-none-eabi-gcc
SIZE := arm-none-eabi-size

TARGET := build/minimal.elf

SOURCES := examples/minimal/main.c \
           platform/stm32f407/startup/startup.c

LINKER_SCRIPT := platform/stm32f407/linker/stm32f407.ld

CPU_FLAGS := -mcpu=cortex-m4 -mthumb -mfloat-abi=soft

CFLAGS := $(CPU_FLAGS) \
          -std=gnu11 \
          -O0 -g3 \
          -Wall -Wextra \
          -ffreestanding \
          -fno-builtin \
          -ffunction-sections -fdata-sections \
          -Iplatform/stm32f407/include  # include path for STM32F407 headers

LDFLAGS := -nostdlib \
           -T$(LINKER_SCRIPT) \
           -Wl,--gc-sections \
           -Wl,-Map=build/minimal.map # report shows address and size of each section in the output file

.PHONY: all clean #ignore file system and always run these cmds

all: $(TARGET)

$(TARGET): $(SOURCES) $(LINKER_SCRIPT) Makefile
	mkdir -p build
	$(CC) $(CFLAGS) $(SOURCES) $(LDFLAGS) -o $@
	$(SIZE) $@

clean:
	rm -rf build