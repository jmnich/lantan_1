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
    AD5664_CMD_WRITE_UPDATE_CHANNEL = 0x03,  // Write to and update channel
    AD5664_CMD_LOAD_LDAC_REGISTER = 0x06,  // Configure LDAC register
} AD5664_Command_t;

// AD5664 power mode (for power command)
typedef enum {
    AD5664_POWER_NORMAL = 0x00,
    AD5664_POWER_1K = 0x01,
    AD5664_POWER_100K = 0x02,
    AD5664_POWER_HIZ = 0x03,
} AD5664_PowerMode_t;

// AD5664 reference voltage in mV (external reference)
#define AD5664_REF_VOLTAGE_mV        2500UL

// AD5664 DAC resolution
#define AD5664_MAX_CODE              65535UL

// Channel enumeration
typedef enum {
    AD5664_CHANNEL_A = 0b000,
    AD5664_CHANNEL_B = 0b001,
    AD5664_CHANNEL_C = 0b010,
    AD5664_CHANNEL_D = 0b011,
    AD5664_CHANNEL_ALL = 0b111,
} AD5664_Channel_t;

// Function prototypes
void AD5664_Init(void);
void AD5664_SetVoltage(AD5664_Channel_t eChannel, uint32_t ulVoltage_mV);
void AD5664_SetAllVoltages(uint32_t ulVoltage_mV);
uint16_t AD5664_VoltageToCode(uint32_t ulVoltage_mV);

#endif // DRIVER_AD5664_H_
