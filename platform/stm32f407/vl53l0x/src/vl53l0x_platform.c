#include "vl53l0x_platform.h"

VL53L0X_Error VL53L0X_ReadMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count)
{
    if (Dev == NULL)
    {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }

    if (Dev->i2c == NULL || Dev->getTimeUs == NULL || pdata == NULL || count == 0U)
    {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }

    // convert to 7 bits
    uint8_t address = (uint8_t)(Dev->I2cDevAddr >> 1U);

    I2cStatus status = i2cMasterWriteRead(
        Dev->i2c,
        address,
        &index,
        1U,
        pdata,
        count,
        Dev->getTimeUs,
        Dev->timeoutUs);

    if (status == I2C_STATUS_OK)
    {
        return VL53L0X_ERROR_NONE;
    }

    if (status == I2C_STATUS_INVALID_ARGUMENT)
    {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }

    return VL53L0X_ERROR_CONTROL_INTERFACE;
}

VL53L0X_Error VL53L0X_RdByte(VL53L0X_DEV Dev, uint8_t index, uint8_t *data)
{
    return VL53L0X_ReadMulti(Dev, index, data, 1U);
}

VL53L0X_Error VL53L0X_RdWord(
    VL53L0X_DEV Dev,
    uint8_t index,
    uint16_t *data)
{
    if (data == NULL)
    {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }

    uint8_t buffer[2];

    VL53L0X_Error status =
        VL53L0X_ReadMulti(Dev, index, buffer, 2U);

    if (status != VL53L0X_ERROR_NONE)
    {
        return status;
    }

    *data = (uint16_t)(((uint16_t)buffer[0] << 8U) |
                       (uint16_t)buffer[1]);

    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_RdDWord(
    VL53L0X_DEV Dev,
    uint8_t index,
    uint32_t *data)
{
    if (data == NULL)
    {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }

    uint8_t buffer[4];

    VL53L0X_Error status =
        VL53L0X_ReadMulti(Dev, index, buffer, 4U);

    if (status != VL53L0X_ERROR_NONE)
    {
        return status;
    }

    *data = ((uint32_t)buffer[0] << 24U) |
            ((uint32_t)buffer[1] << 16U) |
            ((uint32_t)buffer[2] << 8U) |
            (uint32_t)buffer[3];

    return VL53L0X_ERROR_NONE;
}

/* Our adapter's maximum payload, excluding the register address. */
#define VL53L0X_MAX_WRITE_BYTES 255U

VL53L0X_Error VL53L0X_WriteMulti(
    VL53L0X_DEV Dev,
    uint8_t index,
    uint8_t *pdata,
    uint32_t count)
{
    if (Dev == NULL)
    {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }

    if (Dev->i2c == NULL ||
        Dev->getTimeUs == NULL ||
        pdata == NULL ||
        count == 0U ||
        count > VL53L0X_MAX_WRITE_BYTES)
    {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }

    uint8_t buffer[VL53L0X_MAX_WRITE_BYTES + 1U];

    /* First byte selects the starting register. */
    buffer[0] = index;

    /* Remaining bytes contain the data to write. */
    for (uint32_t i = 0U; i < count; i++)
    {
        buffer[i + 1U] = pdata[i];
    }

    uint8_t address = (uint8_t)(Dev->I2cDevAddr >> 1U);

    I2cStatus status = i2cMasterWrite(
        Dev->i2c,
        address,
        buffer,
        count + 1U,
        Dev->getTimeUs,
        Dev->timeoutUs);

    if (status == I2C_STATUS_OK)
    {
        return VL53L0X_ERROR_NONE;
    }

    if (status == I2C_STATUS_INVALID_ARGUMENT)
    {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }

    return VL53L0X_ERROR_CONTROL_INTERFACE;
}

VL53L0X_Error VL53L0X_WrByte(
    VL53L0X_DEV Dev,
    uint8_t index,
    uint8_t data)
{
    return VL53L0X_WriteMulti(Dev, index, &data, 1U);
}

VL53L0X_Error VL53L0X_WrWord(
    VL53L0X_DEV Dev,
    uint8_t index,
    uint16_t data)
{
    uint8_t buffer[2];

    buffer[0] = (uint8_t)(data >> 8U);
    buffer[1] = (uint8_t)(data & 0xFFU);

    return VL53L0X_WriteMulti(Dev, index, buffer, 2U);
}

VL53L0X_Error VL53L0X_WrDWord(
    VL53L0X_DEV Dev,
    uint8_t index,
    uint32_t data)
{
    uint8_t buffer[4];

    buffer[0] = (uint8_t)(data >> 24U);
    buffer[1] = (uint8_t)(data >> 16U);
    buffer[2] = (uint8_t)(data >> 8U);
    buffer[3] = (uint8_t)data;

    return VL53L0X_WriteMulti(Dev, index, buffer, 4U);
}

VL53L0X_Error VL53L0X_UpdateByte(
    VL53L0X_DEV Dev,
    uint8_t index,
    uint8_t AndData,
    uint8_t OrData)
{
    uint8_t value;

    VL53L0X_Error status =
        VL53L0X_RdByte(Dev, index, &value);

    if (status != VL53L0X_ERROR_NONE)
    {
        return status;
    }

    value = (uint8_t)((value & AndData) | OrData);

    return VL53L0X_WrByte(Dev, index, value);
}

VL53L0X_Error VL53L0X_PollingDelay(VL53L0X_DEV Dev)
{
    if (Dev == NULL)
    {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }

    if (Dev->getTimeUs == NULL)
    {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }

    uint32_t startUs = Dev->getTimeUs();

    while ((uint32_t)(Dev->getTimeUs() - startUs) < 1000U)
    {
    }

    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_LockSequenceAccess(VL53L0X_DEV Dev)
{
    if (Dev == NULL)
    {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }

    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_UnlockSequenceAccess(VL53L0X_DEV Dev)
{
    if (Dev == NULL)
    {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }

    return VL53L0X_ERROR_NONE;
}