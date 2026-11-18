#ifndef I2C_H
#define I2C_H

#include <stdbool.h>
#include <stdint.h>
#include "stm32f407.h"

#define I2C_SR1_SB (1U << 0U)
#define I2C_SR1_ADDR (1U << 1U)
#define I2C_SR1_BTF (1U << 2U)
#define I2C_SR1_RXNE (1U << 6U)
#define I2C_SR1_TXE (1U << 7U)
#define I2C_SR1_BERR (1U << 8U)
#define I2C_SR1_ARLO (1U << 9U)
#define I2C_SR1_AF (1U << 10U)
#define I2C_SR1_OVR (1U << 11U)

#define I2C_SR2_BUSY (1U << 1U)
#define I2C_SR2_MSL (1U << 0U)

#define I2C_CR1_START (1U << 8U)
#define I2C_CR1_STOP (1U << 9U)
#define I2C_CR1_PE (1U << 0U)
#define I2C_CR1_ACK (1U << 10U)
#define I2C_CR1_POS (1U << 11U)

typedef enum
{
    I2C_STATUS_OK = 0,
    I2C_STATUS_INVALID_ARGUMENT,
    I2C_STATUS_NOT_READY,
    I2C_STATUS_TIMEOUT,
    I2C_STATUS_NACK,
    I2C_STATUS_BUS_ERROR,
    I2C_STATUS_ARBITRATION_LOST,
    I2C_STATUS_OVERRUN
} I2cStatus;

typedef uint32_t (*I2cGetTimeUs)(void);
bool i2cInit(I2C_Registers *i2c, uint32_t peripheralClockHz, uint32_t busHz);
I2cStatus i2cMasterWrite(I2C_Registers *i2c, uint8_t address, const uint8_t *data, uint32_t length, I2cGetTimeUs getTimeUs, uint32_t timeoutUs);
I2cStatus i2cMasterRead(I2C_Registers *i2c, uint8_t address, uint8_t *data, uint32_t length, I2cGetTimeUs getTimeUs, uint32_t timeoutUs);
I2cStatus i2cMasterWriteRead(I2C_Registers *i2c, uint8_t address, const uint8_t *writeData, uint32_t writeLength, uint8_t *readData, uint32_t readLength, I2cGetTimeUs getTimeUs, uint32_t timeoutUs);

#endif
