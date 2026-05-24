#ifndef DRIVER_AD5664_H_
#define DRIVER_AD5664_H_

#include "stm32h7xx_hal.h"
#include "spi.h"

// AD5664 register addresses
#define AD5664_REG_DAC                0x00
#define AD5664_REG_INPUT_SHIFT        0x01
#define AD5664_REG_DAC_UPDATE         0x02
#define AD5664_REG_CONFIG            0x03
#define AD5664_REG_POWER_CMD         0x04
#define AD5664_REG_CONTROL           0x05
#define AD5664_REG_SW_RESET          0x06

// AD5664 commands (24-bit format)
// Command format: [C3:C0][A1:A0][D15:D0]
// Where C3:C0 is the command, A1:A0 is the address (for multi-DAC devices)

typedef enum {
    AD5664_CMD_WRITE_UPDATE_DAC = 0x00,  // Write and update DAC register
    AD5664_CMD_WRITE_DAC = 0x01,          // Write to DAC register (no update)
    AD5664_CMD_UPDATE_DAC = 0x02,         // Update DAC register
    AD5664_CMD_WRITE_UPDATE_ALL = 0x03,  // Write and update all DACs
    AD5664_CMD_WRITE_ALL = 0x04,          // Write all DAC registers (no update)
    AD5664_CMD_UPDATE_ALL = 0x05,         // Update all DACs
    AD5664_CMD_POWER_DOWN = 0x06,        // Power down
    AD5664_CMD_SW_RESET = 0x07,          // Software reset
    AD5664_CMD_WRITE_CONFIG = 0x08,      // Write to configuration register
    AD5664_CMD_READ_CONFIG = 0x09,       // Read from configuration register
} AD5664_Command_t;

// AD5664 reference select (for control register)
typedef enum {
    AD5664_REF_INTERNAL = 0x00,
    AD5664_REF_EXTERNAL = 0x01,
} AD5664_Reference_t;

// AD5664 power mode (for power command)
typedef enum {
    AD5664_POWER_NORMAL = 0x00,
    AD5664_POWER_1K = 0x01,
    AD5664_POWER_100K = 0x02,
    AD5664_POWER_HIZ = 0x03,
} AD5664_PowerMode_t;

// AD5664 DAC address (for multi-DAC devices)
#define AD5664_ADDR_DAC_A             0x00
#define AD5664_ADDR_DAC_B             0x01
#define AD5664_ADDR_DAC_C             0x02
#define AD5664_ADDR_DAC_D             0x03
#define AD5664_ADDR_ALL_DACS          0x07

// AD5664 reference voltage in mV (external reference)
#define AD5664_REF_VOLTAGE_mV        2500UL

// AD5664 DAC resolution
#define AD5664_MAX_CODE              65535UL

// Channel enumeration
typedef enum {
    AD5664_CHANNEL_A = 0,
    AD5664_CHANNEL_B,
    AD5664_CHANNEL_C,
    AD5664_CHANNEL_D,
    AD5664_CHANNEL_ALL
} AD5664_Channel_t;

// Function prototypes
void AD5664_Init(void);
void AD5664_WriteDAC(uint16_t usValue);
void AD5664_WriteAndUpdateDAC(uint16_t usValue);
void AD5664_UpdateDAC(void);
void AD5664_SetPowerMode(AD5664_PowerMode_t ePowerMode);
void AD5664_SoftwareReset(void);
void AD5664_WriteConfig(uint16_t usConfig);
void AD5664_SetVoltage(AD5664_Channel_t eChannel, uint32_t ulVoltage_mV);
void AD5664_SetAllVoltages(uint32_t ulVoltageA_mV, uint32_t ulVoltageB_mV, 
                          uint32_t ulVoltageC_mV, uint32_t ulVoltageD_mV);

#endif // DRIVER_AD5664_H_
