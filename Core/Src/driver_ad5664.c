#include "driver_ad5664.h"

// CS pin control macros
#define AD5664_CS_LOW()    HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_RESET)
#define AD5664_CS_HIGH()   HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET)

// SPI handle
#define AD5664_SPI_HANDLE  hspi4

// Default DAC address (for single DAC device, address is 0)
#define AD5664_DEFAULT_ADDR  0x00

/**
 * @brief Send data to AD5664 via SPI
 * @param pTxData Pointer to transmit data buffer
 * @param usSize Size of data to transmit in bytes
 */
static void AD5664_SPI_Transmit(uint8_t *pTxData, uint16_t usSize) {
    AD5664_CS_LOW();
    HAL_SPI_Transmit(&AD5664_SPI_HANDLE, pTxData, usSize, HAL_MAX_DELAY);
    AD5664_CS_HIGH();
}

/**
 * @brief Initialize AD5664 DAC
 *        Sets up the CS pin and performs a software reset
 */
void AD5664_Init(void) {
    // Ensure CS is high initially
    AD5664_CS_HIGH();
    for(int i = 0; i < 20; i++) __NOP();
    AD5664_SetAllVoltages(0);

    // uint8_t aucTxData[3];
    // aucTxData[0] = (AD5664_CMD_LOAD_LDAC_REGISTER << 3);
    // aucTxData[1] = (uint8_t)(0x0);
    // aucTxData[2] = (uint8_t)(0b00001111);
    
    // AD5664_SPI_Transmit(aucTxData, 3);
}

/**
 * @brief Convert voltage in mV to DAC code
 * @param ulVoltage_mV Voltage in millivolts
 * @return 16-bit DAC code
 */
uint16_t AD5664_VoltageToCode(uint32_t ulVoltage_mV) {
    // Clamp voltage to reference range
    if (ulVoltage_mV > AD5664_REF_VOLTAGE_mV) {
        ulVoltage_mV = AD5664_REF_VOLTAGE_mV;
    }
    
    // Calculate DAC code: (Vout / Vref) * MAX_CODE
    // Using integer arithmetic to avoid floating point
    uint32_t ulCode = (ulVoltage_mV * AD5664_MAX_CODE) / AD5664_REF_VOLTAGE_mV;
    
    return (uint16_t)ulCode;
}

/**
 * @brief Set voltage for a specific DAC channel
 * @param eChannel Channel to set (A, B, C, D)
 * @param ulVoltage_mV Voltage in millivolts (0 to 2500)
 */
void AD5664_SetVoltage(AD5664_Channel_t eChannel, uint32_t ulVoltage_mV) {
    uint16_t usCode = AD5664_VoltageToCode(ulVoltage_mV);
    uint8_t aucTxData[3];
    uint8_t ucAddr = (uint8_t)eChannel; // = (eChannel == AD5664_CHANNEL_ALL) ? AD5664_ADDR_ALL_DACS : eChannel;  
    
        aucTxData[0] = (0b010 << 3) | (ucAddr << 0);
        aucTxData[1] = (uint8_t)(usCode >> 8);
        aucTxData[2] = (uint8_t)(usCode & 0xFF);
    
    AD5664_SPI_Transmit(aucTxData, 3);
}

/**
 * @brief Set voltages for all 4 DAC channels simultaneously
 * @param ulVoltage_mV Voltage in millivolts
 */
void AD5664_SetAllVoltages(uint32_t ulVoltage_mV) {
    // Set each channel individually
    AD5664_SetVoltage(AD5664_CHANNEL_ALL, ulVoltage_mV);
}
