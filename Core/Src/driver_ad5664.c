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
    
    // Perform software reset
    AD5664_SoftwareReset();
}

/**
 * @brief Write value to DAC register (no update) for channel A
 * @param usValue 16-bit value to write to DAC register
 */
void AD5664_WriteDAC(uint16_t usValue) {
    uint8_t aucTxData[3];
    
    // Command: Write to DAC register (C3:C0 = 0x01)
    // Address: 0x00 (for DAC A)
    aucTxData[0] = (AD5664_CMD_WRITE_DAC << 4) | (AD5664_DEFAULT_ADDR << 2);
    aucTxData[1] = (uint8_t)(usValue >> 8);
    aucTxData[2] = (uint8_t)(usValue & 0xFF);
    
    AD5664_SPI_Transmit(aucTxData, 3);
}

/**
 * @brief Write value to DAC register and update immediately for channel A
 * @param usValue 16-bit value to write and update
 */
void AD5664_WriteAndUpdateDAC(uint16_t usValue) {
    uint8_t aucTxData[3];
    
    // Command: Write and update DAC register (C3:C0 = 0x00)
    // Address: 0x00 (for DAC A)
    aucTxData[0] = (AD5664_CMD_WRITE_UPDATE_DAC << 4) | (AD5664_DEFAULT_ADDR << 2);
    aucTxData[1] = (uint8_t)(usValue >> 8);
    aucTxData[2] = (uint8_t)(usValue & 0xFF);
    
    AD5664_SPI_Transmit(aucTxData, 3);
}

/**
 * @brief Update DAC output with current register value for channel A
 */
void AD5664_UpdateDAC(void) {
    uint8_t aucTxData[3];
    
    // Command: Update DAC (C3:C0 = 0x02)
    // Address: 0x00 (for DAC A)
    // Data: 0 (no data needed for update command)
    aucTxData[0] = (AD5664_CMD_UPDATE_DAC << 4) | (AD5664_DEFAULT_ADDR << 2);
    aucTxData[1] = 0x00;
    aucTxData[2] = 0x00;
    
    AD5664_SPI_Transmit(aucTxData, 3);
}

/**
 * @brief Set power mode for DAC
 * @param ePowerMode Power mode to set
 */
void AD5664_SetPowerMode(AD5664_PowerMode_t ePowerMode) {
    uint8_t aucTxData[3];
    
    // Command: Power command (C3:C0 = 0x06)
    // Address: 0x00 (for DAC A)
    // Data: Power mode (2-bit) in bits D1:D0
    // 24-bit word format:
    // Byte 0: [C3 C2 C1 C0 | A1 A0 D15 D14]
    // Byte 1: [D13 D12 D11 D10 | D9 D8 D7 D6]
    // Byte 2: [D5 D4 D3 D2 | D1 D0 X X]
    // For power command, only D1:D0 are used (power mode bits)
    aucTxData[0] = (AD5664_CMD_POWER_DOWN << 4) | (AD5664_DEFAULT_ADDR << 2);
    aucTxData[1] = 0x00;
    aucTxData[2] = (uint8_t)(ePowerMode & 0x03);
    
    AD5664_SPI_Transmit(aucTxData, 3);
}

/**
 * @brief Perform software reset
 *        Resets all registers to default values and updates all DAC outputs
 */
void AD5664_SoftwareReset(void) {
    uint8_t aucTxData[3];
    
    // Command: Software reset (C3:C0 = 0x07)
    // Address: 0x07 (broadcast to all DACs)
    aucTxData[0] = (AD5664_CMD_SW_RESET << 4) | (AD5664_ADDR_ALL_DACS << 2);
    aucTxData[1] = 0x00;
    aucTxData[2] = 0x00;
    
    AD5664_SPI_Transmit(aucTxData, 3);
    
    // Wait for reset to complete (minimum 24 SCLK cycles)
    // The AD5664 requires at least 24 SCLK cycles after reset
    HAL_Delay(1);
}

/**
 * @brief Write to configuration register
 * @param usConfig 16-bit configuration value
 */
void AD5664_WriteConfig(uint16_t usConfig) {
    uint8_t aucTxData[3];
    
    // Command: Write to configuration register (C3:C0 = 0x08)
    aucTxData[0] = (AD5664_CMD_WRITE_CONFIG << 4);
    aucTxData[1] = (uint8_t)(usConfig >> 8);
    aucTxData[2] = (uint8_t)(usConfig & 0xFF);
    
    AD5664_SPI_Transmit(aucTxData, 3);
}

/**
 * @brief Convert voltage in mV to DAC code
 * @param ulVoltage_mV Voltage in millivolts
 * @return 16-bit DAC code
 */
static uint16_t AD5664_VoltageToCode(uint32_t ulVoltage_mV) {
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
    uint8_t ucAddr = (eChannel == AD5664_CHANNEL_ALL) ? AD5664_ADDR_ALL_DACS : eChannel;
    
    // 24-bit word format: [C3:C0][A1:A0][D15:D0]
    // Byte 0: Command (4 bits) | Address (2 bits) | D15:D14 (2 bits)
    // Byte 1: D13:D8 (8 bits)
    // Byte 2: D7:D0 (8 bits)
    aucTxData[0] = (AD5664_CMD_WRITE_UPDATE_DAC << 4) | (ucAddr << 2);
    aucTxData[1] = (uint8_t)(usCode >> 8);
    aucTxData[2] = (uint8_t)(usCode & 0xFF);
    
    AD5664_SPI_Transmit(aucTxData, 3);
}

/**
 * @brief Set voltages for all 4 DAC channels simultaneously
 * @param ulVoltageA_mV Voltage for channel A in millivolts
 * @param ulVoltageB_mV Voltage for channel B in millivolts
 * @param ulVoltageC_mV Voltage for channel C in millivolts
 * @param ulVoltageD_mV Voltage for channel D in millivolts
 */
void AD5664_SetAllVoltages(uint32_t ulVoltageA_mV, uint32_t ulVoltageB_mV, 
                          uint32_t ulVoltageC_mV, uint32_t ulVoltageD_mV) {
    // Set each channel individually
    AD5664_SetVoltage(AD5664_CHANNEL_A, ulVoltageA_mV);
    AD5664_SetVoltage(AD5664_CHANNEL_B, ulVoltageB_mV);
    AD5664_SetVoltage(AD5664_CHANNEL_C, ulVoltageC_mV);
    AD5664_SetVoltage(AD5664_CHANNEL_D, ulVoltageD_mV);
}
