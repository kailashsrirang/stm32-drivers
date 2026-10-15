CC := arm-none-eabi-gcc
SIZE := arm-none-eabi-size

TARGET := build/minimal.elf

SOURCES := examples/minimal/main.c \
          platform/stm32f407/startup/startup.c \
          platform/stm32f407/src/gpio.c

OBJECTS := $(patsubst %.c,build/%.o,$(SOURCES))
DEPS := $(OBJECTS:.o=.d)

LINKER_SCRIPT := platform/stm32f407/linker/stm32f407.ld

CPU_FLAGS := -mcpu=cortex-m4 -mthumb -mfloat-abi=soft

CFLAGS := $(CPU_FLAGS) \
          -std=gnu11 \
          -O0 -g3 \
          -Wall -Wextra \
          -ffreestanding \
          -fno-builtin \
          -ffunction-sections -fdata-sections \
          -Iplatform/stm32f407/include \
          -MMD \
          -MP

LDFLAGS := -nostdlib \
           -T$(LINKER_SCRIPT) \
           -Wl,--gc-sections \
           -Wl,-Map=build/minimal.map

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS) $(LINKER_SCRIPT) Makefile
	mkdir -p build
	$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) -o $@
	$(SIZE) $@

build/%.o: %.c Makefile
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf build