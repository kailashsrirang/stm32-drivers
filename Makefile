CC := arm-none-eabi-gcc
SIZE := arm-none-eabi-size
HOST_CC ?= cc
.DEFAULT_GOAL := all

TARGET := build/minimal.elf
UNITY_DIR := tests/unity/src
SERVO_TEST := build/tests/test_servo

SOURCES := examples/minimal/main.c \
          platform/stm32f407/startup/startup.c \
          platform/stm32f407/src/gpio.c \
          platform/stm32f407/src/timer.c \
          platform/stm32f407/src/servo.c  \
          platform/stm32f407/src/i2c.c


SOURCES +=  third_party/vl53l0x/core/src/vl53l0x_api.c \
            third_party/vl53l0x/core/src/vl53l0x_api_calibration.c \
            third_party/vl53l0x/core/src/vl53l0x_api_core.c \
            third_party/vl53l0x/core/src/vl53l0x_api_ranging.c \
            third_party/vl53l0x/core/src/vl53l0x_api_strings.c \
            platform/stm32f407/vl53l0x/src/vl53l0x_platform.c

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
          -Iplatform/stm32f407/vl53l0x/include \
          -MMD \
          -MP

CFLAGS += -Ithird_party/vl53l0x/core/inc \
          -Iplatform/stm32f407/vl53l0x/inc

LDFLAGS := -nostdlib \
           -T$(LINKER_SCRIPT) \
           -Wl,--gc-sections \
           -Wl,-Map=build/minimal.map

.PHONY: all clean test-unit

.PHONY: flash

flash: $(TARGET)
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $(TARGET) verify reset exit"


all: $(TARGET)

test-unit: $(SERVO_TEST)
	./$(SERVO_TEST)

$(SERVO_TEST): tests/test_servo.c tests/fakes/timer_fake.c tests/fakes/timer_fake.h \
               platform/stm32f407/src/servo.c platform/stm32f407/include/servo.h \
               platform/stm32f407/include/timer.h platform/stm32f407/include/stm32f407.h \
               $(UNITY_DIR)/unity.c $(UNITY_DIR)/unity.h $(UNITY_DIR)/unity_internals.h Makefile
	mkdir -p $(dir $@)
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror \
		-I$(UNITY_DIR) -Itests/fakes -Iplatform/stm32f407/include \
		tests/test_servo.c tests/fakes/timer_fake.c \
		platform/stm32f407/src/servo.c $(UNITY_DIR)/unity.c -o $@

$(TARGET): $(OBJECTS) $(LINKER_SCRIPT) Makefile
	mkdir -p build
	$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) -Wl,--start-group -lc -lgcc -Wl,--end-group -o $@
	$(SIZE) $@

build/%.o: %.c Makefile
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf build