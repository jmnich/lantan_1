/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins
     PH0-OSC_IN (PH0)   ------> RCC_OSC_IN
     PH1-OSC_OUT (PH1)   ------> RCC_OSC_OUT
     PA13 (JTMS/SWDIO)   ------> DEBUG_JTMS-SWDIO
     PA14 (JTCK/SWCLK)   ------> DEBUG_JTCK-SWCLK
     PB3 (JTDO/TRACESWO)   ------> DEBUG_JTDO-SWO
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, SPI4_CS_Pin|DAC_DIAG_A0_Pin|DAC_DIAG_A1_Pin|FAN1_Pin
                          |FAN2_Pin|FAN3_Pin|FAN4_Pin|FAN5_Pin
                          |ENA_9V_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED1_Pin|LED2_Pin|LED3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, SRC_A_RELEASE_Pin|SRC_B_RELEASE_Pin|SRC_C_RELEASE_Pin|SRC_D_RELEASE_Pin
                          |SRC_A_VOLT_SEL_Pin|SRC_B_VOLT_SEL_Pin|SRC_C_VOLT_SEL_Pin|SRC_D_VOLT_SEL_Pin
                          |LED0_DBG_Pin|LED1_DBG_Pin|LED2_DBG_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : SPI4_CS_Pin DAC_DIAG_A0_Pin DAC_DIAG_A1_Pin FAN1_Pin
                           FAN2_Pin FAN3_Pin FAN4_Pin FAN5_Pin
                           ENA_9V_Pin */
  GPIO_InitStruct.Pin = SPI4_CS_Pin|DAC_DIAG_A0_Pin|DAC_DIAG_A1_Pin|FAN1_Pin
                          |FAN2_Pin|FAN3_Pin|FAN4_Pin|FAN5_Pin
                          |ENA_9V_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : LED1_Pin LED2_Pin LED3_Pin */
  GPIO_InitStruct.Pin = LED1_Pin|LED2_Pin|LED3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SRC_A_RELEASE_Pin SRC_B_RELEASE_Pin SRC_C_RELEASE_Pin SRC_D_RELEASE_Pin
                           SRC_A_VOLT_SEL_Pin SRC_B_VOLT_SEL_Pin SRC_C_VOLT_SEL_Pin SRC_D_VOLT_SEL_Pin
                           LED0_DBG_Pin LED1_DBG_Pin LED2_DBG_Pin */
  GPIO_InitStruct.Pin = SRC_A_RELEASE_Pin|SRC_B_RELEASE_Pin|SRC_C_RELEASE_Pin|SRC_D_RELEASE_Pin
                          |SRC_A_VOLT_SEL_Pin|SRC_B_VOLT_SEL_Pin|SRC_C_VOLT_SEL_Pin|SRC_D_VOLT_SEL_Pin
                          |LED0_DBG_Pin|LED1_DBG_Pin|LED2_DBG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*AnalogSwitch Config */
  HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA0, SYSCFG_SWITCH_PA0_CLOSE);

  /*AnalogSwitch Config */
  HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA1, SYSCFG_SWITCH_PA1_CLOSE);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
