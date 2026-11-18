#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "stm32f407.h"
#include "i2c.h"

static bool i2cGetInfo(const I2C_Registers *i2c, uint32_t *clockMask, uint32_t *resetMask)
{
    if (i2c == NULL || clockMask == NULL || resetMask == NULL)
    {
        return false;
    }

    if (i2c == I2C1) // I2C1 base address
    {
        *clockMask = RCC_I2C1EN;
        *resetMask = RCC_I2C1RST;
    }
    else if (i2c == I2C2) // I2C2 base address
    {
        *clockMask = RCC_I2C2EN;
        *resetMask = RCC_I2C2RST;
    }
    else if (i2c == I2C3) // I2C3 base address
    {
        *clockMask = RCC_I2C3EN;
        *resetMask = RCC_I2C3RST;
    }
    else
    {
        return false;
    }
    return true;
}

static bool i2cIsClockEnabled(const I2C_Registers *i2c)
{

    uint32_t clockMask;
    uint32_t resetMask;

    if (!i2cGetInfo(i2c, &clockMask, &resetMask))
    {
        return false;
    }

    return (RCC->APB1ENR & clockMask) != 0U;
}

bool i2cInit(I2C_Registers *i2c, uint32_t peripheralClockHz, uint32_t busHz)
{
    uint32_t clockMask;
    uint32_t resetMask;

    if (!i2cGetInfo(i2c, &clockMask, &resetMask))
    {
        return false;
    }

    if (peripheralClockHz < 2000000U || peripheralClockHz > 42000000U)
    {
        return false;
    }

    if (peripheralClockHz % 1000000U != 0)
    {
        return false; // clock freq must be a whole number of Mhz
    }

    if (busHz == 0 || busHz > 100000U)
    {
        return false; // not standard mode
    }

    uint32_t frequencyMHz = peripheralClockHz / 1000000U;

    uint32_t denominator = 2U * busHz;

    uint32_t ccr = (peripheralClockHz + denominator - 1U) / denominator;

    if (ccr < 4U || ccr > 0xFFFU)
    {
        return false;
    }

    uint32_t trise = frequencyMHz + 1U; // standard mode

    RCC->APB1ENR |= clockMask;
    (void)RCC->APB1ENR; // read back to ensure clock is activated before continuing

    RCC->APB1RSTR |= resetMask;
    RCC->APB1RSTR &= ~resetMask;

    i2c->CR1 = 0U; // ensure i2c is disabled while configuring occurs

    i2c->CR2 = frequencyMHz; // write Pclk (input freq)

    i2c->OAR1 = (1U << 14U);
    i2c->OAR2 = 0U;

    i2c->CCR = ccr; // set divider

    i2c->TRISE = trise;

    i2c->CR1 = I2C_CR1_PE; // enable i2c

    return true;
}

static I2cStatus i2cWaitForFlag(const I2C_Registers *i2c, uint32_t flagMask, I2cGetTimeUs getTimeUs, uint32_t startUs, uint32_t timeoutUs)
{
    if (getTimeUs == NULL || flagMask == 0U)
    {
        return I2C_STATUS_INVALID_ARGUMENT;
    }

    if (!i2cIsClockEnabled(i2c))
    {
        return I2C_STATUS_NOT_READY;
    }

    if ((i2c->CR1 & I2C_CR1_PE) == 0U)
    {
        return I2C_STATUS_NOT_READY;
    }

    while (1)
    {
        uint32_t sr1 = i2c->SR1;

        // check hw flag
        if ((sr1 & I2C_SR1_ARLO) != 0U)
        {
            return I2C_STATUS_ARBITRATION_LOST;
        }

        if ((sr1 & I2C_SR1_BERR) != 0U)
        {
            return I2C_STATUS_BUS_ERROR;
        }

        if ((sr1 & I2C_SR1_OVR) != 0U)
        {
            return I2C_STATUS_OVERRUN;
        }

        if ((sr1 & I2C_SR1_AF) != 0U)
        {
            return I2C_STATUS_NACK;
        }

        // set flags
        if ((sr1 & flagMask) == flagMask)
        {
            return I2C_STATUS_OK;
        }

        if ((uint32_t)(getTimeUs() - startUs) >= timeoutUs)
        {
            return I2C_STATUS_TIMEOUT;
        }
    }
}

static I2cStatus i2cStartAddress(I2C_Registers *i2c, uint8_t address, bool read, I2cGetTimeUs getTimeUs, uint32_t startUs, uint32_t timeoutUs)
{

    if (address > 0x7FU || getTimeUs == NULL)
    {
        return I2C_STATUS_INVALID_ARGUMENT;
    }

    if (!i2cIsClockEnabled(i2c))
    {
        return I2C_STATUS_NOT_READY;
    }

    // check if peripheral enabled
    if ((i2c->CR1 & I2C_CR1_PE) == 0U)
    {
        return I2C_STATUS_NOT_READY;
    }

    // send start
    i2c->CR1 |= I2C_CR1_START;

    // wait for start sent flag
    I2cStatus status = i2cWaitForFlag(i2c, I2C_SR1_SB, getTimeUs, startUs, timeoutUs);
    if (status != I2C_STATUS_OK)
    {
        return status;
    }

    // send slave address
    i2c->DR = ((uint32_t)address << 1U) | (read ? 1U : 0U);

    // wait for address sent flag
    return i2cWaitForFlag(i2c, I2C_SR1_ADDR, getTimeUs, startUs, timeoutUs);
}

static void i2cClearAddressFlag(I2C_Registers *i2c)
{
    // read SR1 and SR2 to cear address flag
    (void)i2c->SR1;
    (void)i2c->SR2;
}

static void i2cClearErrorFlags(I2C_Registers *i2c)
{
    // Your implementation
    uint32_t mask = I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF | I2C_SR1_OVR;
    i2c->SR1 &= ~mask;
}

// Caller guarantees a valid peripheral with its clock enabled.
static void i2cCleanupTransfer(I2C_Registers *i2c)
{
    uint32_t cr1 = i2c->CR1;

    if ((cr1 & (I2C_CR1_START | I2C_CR1_STOP)) == 0U)
    {
        cr1 &= ~(I2C_CR1_ACK | I2C_CR1_POS);

        if ((i2c->SR2 & I2C_SR2_MSL) != 0U) // check to see if still controller
        {
            cr1 |= I2C_CR1_STOP;
        }

        i2c->CR1 = cr1;
    }

    i2cClearErrorFlags(i2c);
}

static I2cStatus i2cWaitBusIdle(const I2C_Registers *i2c, I2cGetTimeUs getTimeUs, uint32_t startUs, uint32_t timeoutUs)
{
    if (!i2cIsClockEnabled(i2c))
    {
        return I2C_STATUS_NOT_READY;
    }
    if (getTimeUs == NULL)
    {
        return I2C_STATUS_INVALID_ARGUMENT;
    }

    // check if peripheral is enabled
    if ((i2c->CR1 & I2C_CR1_PE) == 0U)
    {
        return I2C_STATUS_NOT_READY;
    }

    // loop until bus not busy;
    while ((i2c->SR2 & I2C_SR2_BUSY) != 0U)
    {
        if ((uint32_t)(getTimeUs() - startUs) >= timeoutUs)
        {
            return I2C_STATUS_TIMEOUT;
        }
    }

    return I2C_STATUS_OK;
}

static I2cStatus i2cWaitStopComplete(const I2C_Registers *i2c, I2cGetTimeUs getTimeUs, uint32_t startUs, uint32_t timeoutUs)
{

    if (getTimeUs == NULL)
    {
        return I2C_STATUS_INVALID_ARGUMENT;
    }

    if (!i2cIsClockEnabled(i2c))
    {
        return I2C_STATUS_NOT_READY;
    }

    // check if peripheral is enabled
    if ((i2c->CR1 & I2C_CR1_PE) == 0U)
    {
        return I2C_STATUS_NOT_READY;
    }

    // loop until stop condition is cleared;
    while ((i2c->CR1 & I2C_CR1_STOP) != 0U)
    {
        if ((uint32_t)(getTimeUs() - startUs) >= timeoutUs)
        {
            return I2C_STATUS_TIMEOUT;
        }
    }

    return I2C_STATUS_OK;
}

static I2cStatus i2cWriteBytes(I2C_Registers *i2c, const uint8_t *data, uint32_t length, I2cGetTimeUs getTimeUs, uint32_t startUs, uint32_t timeoutUs)
{
    if (data == NULL || length == 0 || getTimeUs == NULL)
    {
        return I2C_STATUS_INVALID_ARGUMENT;
    }

    if (!i2cIsClockEnabled(i2c))
    {
        return I2C_STATUS_NOT_READY;
    }

    // check if peripheral is enabled
    if ((i2c->CR1 & I2C_CR1_PE) == 0U)
    {
        return I2C_STATUS_NOT_READY;
    }

    for (uint32_t i = 0; i < length; i++)
    {
        I2cStatus status = i2cWaitForFlag(i2c, I2C_SR1_TXE, getTimeUs, startUs, timeoutUs);
        if (status != I2C_STATUS_OK)
        {
            return status;
        }
        i2c->DR = data[i];
    }

    // check for byte transfer finished flag
    return i2cWaitForFlag(i2c, I2C_SR1_BTF, getTimeUs, startUs, timeoutUs);
}

I2cStatus i2cMasterWrite(I2C_Registers *i2c, uint8_t address, const uint8_t *data, uint32_t length, I2cGetTimeUs getTimeUs, uint32_t timeoutUs)
{
    if (address > 0x7FU || getTimeUs == NULL || data == NULL || length == 0)
    {
        return I2C_STATUS_INVALID_ARGUMENT;
    }

    if (!i2cIsClockEnabled(i2c))
    {
        return I2C_STATUS_NOT_READY;
    }

    // check if peripheral is enabled
    if ((i2c->CR1 & I2C_CR1_PE) == 0U)
    {
        return I2C_STATUS_NOT_READY;
    }
    if ((i2c->CR1 & (I2C_CR1_START | I2C_CR1_STOP)) != 0U)
    {
        return I2C_STATUS_NOT_READY;
    }

    uint32_t startUs = getTimeUs();

    I2cStatus status = i2cWaitBusIdle(i2c, getTimeUs, startUs, timeoutUs);
    if (status != I2C_STATUS_OK)
    {
        return status;
    }
    i2cClearErrorFlags(i2c);

    status = i2cStartAddress(i2c, address, false, getTimeUs, startUs, timeoutUs);
    if (status != I2C_STATUS_OK)
    {
        i2cCleanupTransfer(i2c);
        return status;
    }

    i2cClearAddressFlag(i2c);

    status = i2cWriteBytes(i2c, data, length, getTimeUs, startUs, timeoutUs);
    if (status != I2C_STATUS_OK)
    {
        i2cCleanupTransfer(i2c);
        return status;
    }
    // request stop
    i2c->CR1 |= I2C_CR1_STOP;

    return i2cWaitStopComplete(i2c, getTimeUs, startUs, timeoutUs);
}

static I2cStatus i2cReadOneByte(I2C_Registers *i2c, uint8_t *data, I2cGetTimeUs getTimeUs, uint32_t startUs, uint32_t timeoutUs)
{
    if (data == NULL || getTimeUs == NULL)
    {
        return I2C_STATUS_INVALID_ARGUMENT;
    }
    if (!i2cIsClockEnabled(i2c))
    {
        return I2C_STATUS_NOT_READY;
    }

    // check if peripheral is enabled
    if ((i2c->CR1 & I2C_CR1_PE) == 0U)
    {
        return I2C_STATUS_NOT_READY;
    }

    // POS = 0: normal ACK position.
    // ACK = 0: send NACK after the single received byte.
    i2c->CR1 &= ~(I2C_CR1_POS | I2C_CR1_ACK);

    // clear addr to tell hw to move from address phase to recv phase
    i2cClearAddressFlag(i2c);

    // request a stop
    i2c->CR1 |= I2C_CR1_STOP;

    I2cStatus status = i2cWaitForFlag(i2c, I2C_SR1_RXNE, getTimeUs, startUs, timeoutUs);
    if (status != I2C_STATUS_OK)
    {
        return status;
    }

    *data = (uint8_t)i2c->DR; // reading DR clears RXNE flag

    return i2cWaitStopComplete(i2c, getTimeUs, startUs, timeoutUs);
}

static I2cStatus i2cReadTwoBytes(I2C_Registers *i2c, uint8_t *data, I2cGetTimeUs getTimeUs, uint32_t startUs, uint32_t timeoutUs)
{

    if (data == NULL || getTimeUs == NULL)
    {
        return I2C_STATUS_INVALID_ARGUMENT;
    }
    if (!i2cIsClockEnabled(i2c))
    {
        return I2C_STATUS_NOT_READY;
    }

    // check if peripheral is enabled
    if ((i2c->CR1 & I2C_CR1_PE) == 0U)
    {
        return I2C_STATUS_NOT_READY;
    }

    // POS = 1:  ACK applies after next byte.
    // ACK = 0: send NACK after the second received byte.
    // i.e send NACK after reading second byte
    i2c->CR1 |= I2C_CR1_POS;
    i2c->CR1 &= ~I2C_CR1_ACK;

    // clear addr to tell hw to move from address phase to recv phase
    i2cClearAddressFlag(i2c);

    // first byte in DR second byte in shift reg
    I2cStatus status = i2cWaitForFlag(i2c, I2C_SR1_BTF, getTimeUs, startUs, timeoutUs);
    if (status != I2C_STATUS_OK)
    {
        // restore pos
        i2c->CR1 &= ~(I2C_CR1_POS);
        return status;
    }

    // request a stop
    i2c->CR1 |= I2C_CR1_STOP;

    data[0] = (uint8_t)i2c->DR;
    data[1] = (uint8_t)i2c->DR;

    status = i2cWaitStopComplete(i2c, getTimeUs, startUs, timeoutUs);

    if (status == I2C_STATUS_OK)
    {
        i2c->CR1 &= ~I2C_CR1_POS;
    }

    return status;
}

static I2cStatus i2cReadManyBytes(I2C_Registers *i2c, uint8_t *data, uint32_t length, I2cGetTimeUs getTimeUs, uint32_t startUs, uint32_t timeoutUs)
{
    if (data == NULL || getTimeUs == NULL || length < 3U)
    {
        return I2C_STATUS_INVALID_ARGUMENT;
    }
    if (!i2cIsClockEnabled(i2c))
    {
        return I2C_STATUS_NOT_READY;
    }

    // check if peripheral is enabled
    if ((i2c->CR1 & I2C_CR1_PE) == 0U)
    {
        return I2C_STATUS_NOT_READY;
    }

    i2c->CR1 &= ~I2C_CR1_POS;
    i2c->CR1 |= I2C_CR1_ACK; // enable ack to recv more bytes
    i2cClearAddressFlag(i2c);

    uint32_t index = 0U;
    uint32_t remaining = length;

    while (remaining > 3U)
    {
        I2cStatus status = i2cWaitForFlag(i2c, I2C_SR1_RXNE, getTimeUs, startUs, timeoutUs);
        if (status != I2C_STATUS_OK)
        {
            return status;
        }

        data[index] = (uint8_t)i2c->DR;
        index++;
        remaining -= 1U;
    }

    // Handle final three bytes
    // 3rd last byte is in DR and 2nd last byte in shift reg
    I2cStatus status = i2cWaitForFlag(i2c, I2C_SR1_BTF, getTimeUs, startUs, timeoutUs);
    if (status != I2C_STATUS_OK)
    {
        return status;
    }

    // Configure NACK for the final byte received.
    i2c->CR1 &= ~I2C_CR1_ACK;

    // read the third-last byte
    // this releases the clock stretch and allows the final byte to arrive
    data[index++] = (uint8_t)i2c->DR;

    // wait until the final two bytes are buffered
    // bTF then holds SCL low again
    status = i2cWaitForFlag(i2c, I2C_SR1_BTF, getTimeUs, startUs, timeoutUs);
    if (status != I2C_STATUS_OK)
    {
        return status;
    }

    // request a stop
    i2c->CR1 |= I2C_CR1_STOP;

    data[index++] = (uint8_t)i2c->DR;
    data[index] = (uint8_t)i2c->DR;

    return i2cWaitStopComplete(i2c, getTimeUs, startUs, timeoutUs);
}
I2cStatus i2cMasterRead(I2C_Registers *i2c, uint8_t address, uint8_t *data, uint32_t length, I2cGetTimeUs getTimeUs, uint32_t timeoutUs)
{
    if (address > 0x7FU || getTimeUs == NULL ||
        data == NULL || length == 0U)
    {
        return I2C_STATUS_INVALID_ARGUMENT;
    }

    if (!i2cIsClockEnabled(i2c))
    {
        return I2C_STATUS_NOT_READY;
    }

    if ((i2c->CR1 & I2C_CR1_PE) == 0U)
    {
        return I2C_STATUS_NOT_READY;
    }
    if ((i2c->CR1 & (I2C_CR1_START | I2C_CR1_STOP)) != 0U)
    {
        return I2C_STATUS_NOT_READY;
    }

    uint32_t startUs = getTimeUs();

    I2cStatus status = i2cWaitBusIdle(i2c, getTimeUs, startUs, timeoutUs);

    if (status != I2C_STATUS_OK)
    {
        return status;
    }

    i2cClearErrorFlags(i2c);

    status = i2cStartAddress(i2c, address, true, getTimeUs, startUs, timeoutUs);

    if (status != I2C_STATUS_OK)
    {
        i2cCleanupTransfer(i2c);
        return status;
    }

    if (length == 1U)
    {
        status = i2cReadOneByte(i2c, data, getTimeUs, startUs, timeoutUs);
    }
    else if (length == 2U)
    {
        status = i2cReadTwoBytes(i2c, data, getTimeUs, startUs, timeoutUs);
    }
    else
    {
        status = i2cReadManyBytes(i2c, data, length, getTimeUs, startUs, timeoutUs);
    }

    if (status != I2C_STATUS_OK)
    {
        i2cCleanupTransfer(i2c);
    }

    return status;
}

I2cStatus i2cMasterWriteRead(I2C_Registers *i2c, uint8_t address, const uint8_t *writeData, uint32_t writeLength, uint8_t *readData, uint32_t readLength, I2cGetTimeUs getTimeUs, uint32_t timeoutUs)
{
    if (address > 0x7FU || getTimeUs == NULL ||
        writeData == NULL || readData == NULL || writeLength == 0U || readLength == 0U)
    {
        return I2C_STATUS_INVALID_ARGUMENT;
    }

    if (!i2cIsClockEnabled(i2c))
    {
        return I2C_STATUS_NOT_READY;
    }

    if ((i2c->CR1 & I2C_CR1_PE) == 0U)
    {
        return I2C_STATUS_NOT_READY;
    }
    if ((i2c->CR1 & (I2C_CR1_START | I2C_CR1_STOP)) != 0U)
    {
        return I2C_STATUS_NOT_READY;
    }

    uint32_t startUs = getTimeUs();

    I2cStatus status = i2cWaitBusIdle(i2c, getTimeUs, startUs, timeoutUs);
    if (status != I2C_STATUS_OK)
    {
        return status;
    }
    i2cClearErrorFlags(i2c);

    status = i2cStartAddress(i2c, address, false, getTimeUs, startUs, timeoutUs);
    if (status != I2C_STATUS_OK)
    {
        i2cCleanupTransfer(i2c);
        return status;
    }

    i2cClearAddressFlag(i2c);

    status = i2cWriteBytes(i2c, writeData, writeLength, getTimeUs, startUs, timeoutUs);
    if (status != I2C_STATUS_OK)
    {
        i2cCleanupTransfer(i2c);
        return status;
    }
    // repeated START
    status = i2cStartAddress(i2c, address, true, getTimeUs, startUs, timeoutUs);
    if (status != I2C_STATUS_OK)
    {
        i2cCleanupTransfer(i2c);
        return status;
    }

    if (readLength == 1U)
    {
        status = i2cReadOneByte(i2c, readData, getTimeUs, startUs, timeoutUs);
    }
    else if (readLength == 2U)
    {
        status = i2cReadTwoBytes(i2c, readData, getTimeUs, startUs, timeoutUs);
    }
    else
    {
        status = i2cReadManyBytes(i2c, readData, readLength, getTimeUs, startUs, timeoutUs);
    }

    if (status != I2C_STATUS_OK)
    {
        i2cCleanupTransfer(i2c);
        return status;
    }

    return I2C_STATUS_OK;
}
