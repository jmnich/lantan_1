/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SPI4_CS_Pin GPIO_PIN_4
#define SPI4_CS_GPIO_Port GPIOE
#define DAC_DIAG_Pin GPIO_PIN_0
#define DAC_DIAG_GPIO_Port GPIOC
#define LED1_Pin GPIO_PIN_0
#define LED1_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_1
#define LED2_GPIO_Port GPIOA
#define LED3_Pin GPIO_PIN_2
#define LED3_GPIO_Port GPIOA
#define DETECTOR_Pin GPIO_PIN_6
#define DETECTOR_GPIO_Port GPIOA
#define SRC_A_U_Pin GPIO_PIN_7
#define SRC_A_U_GPIO_Port GPIOA
#define SRC_B_U_Pin GPIO_PIN_4
#define SRC_B_U_GPIO_Port GPIOC
#define SRC_C_U_Pin GPIO_PIN_5
#define SRC_C_U_GPIO_Port GPIOC
#define SRC_D_U_Pin GPIO_PIN_0
#define SRC_D_U_GPIO_Port GPIOB
#define DAC_DIAG_A0_Pin GPIO_PIN_7
#define DAC_DIAG_A0_GPIO_Port GPIOE
#define DAC_DIAG_A1_Pin GPIO_PIN_8
#define DAC_DIAG_A1_GPIO_Port GPIOE
#define FAN1_Pin GPIO_PIN_9
#define FAN1_GPIO_Port GPIOE
#define FAN2_Pin GPIO_PIN_10
#define FAN2_GPIO_Port GPIOE
#define FAN3_Pin GPIO_PIN_11
#define FAN3_GPIO_Port GPIOE
#define FAN4_Pin GPIO_PIN_12
#define FAN4_GPIO_Port GPIOE
#define FAN5_Pin GPIO_PIN_13
#define FAN5_GPIO_Port GPIOE
#define ENA_9V_Pin GPIO_PIN_14
#define ENA_9V_GPIO_Port GPIOE
#define TIA_SEL_B_Pin GPIO_PIN_12
#define TIA_SEL_B_GPIO_Port GPIOB
#define TIA_SEL_A_Pin GPIO_PIN_13
#define TIA_SEL_A_GPIO_Port GPIOB
#define AMP_SEL_B_Pin GPIO_PIN_14
#define AMP_SEL_B_GPIO_Port GPIOB
#define AMP_SEL_A_Pin GPIO_PIN_15
#define AMP_SEL_A_GPIO_Port GPIOB
#define SRC_A_RELEASE_Pin GPIO_PIN_8
#define SRC_A_RELEASE_GPIO_Port GPIOD
#define SRC_B_RELEASE_Pin GPIO_PIN_9
#define SRC_B_RELEASE_GPIO_Port GPIOD
#define SRC_C_RELEASE_Pin GPIO_PIN_10
#define SRC_C_RELEASE_GPIO_Port GPIOD
#define SRC_D_RELEASE_Pin GPIO_PIN_11
#define SRC_D_RELEASE_GPIO_Port GPIOD
#define SRC_A_VOLT_SEL_Pin GPIO_PIN_12
#define SRC_A_VOLT_SEL_GPIO_Port GPIOD
#define SRC_B_VOLT_SEL_Pin GPIO_PIN_13
#define SRC_B_VOLT_SEL_GPIO_Port GPIOD
#define SRC_C_VOLT_SEL_Pin GPIO_PIN_14
#define SRC_C_VOLT_SEL_GPIO_Port GPIOD
#define SRC_D_VOLT_SEL_Pin GPIO_PIN_15
#define SRC_D_VOLT_SEL_GPIO_Port GPIOD
#define LED0_DBG_Pin GPIO_PIN_0
#define LED0_DBG_GPIO_Port GPIOD
#define LED1_DBG_Pin GPIO_PIN_1
#define LED1_DBG_GPIO_Port GPIOD
#define LED2_DBG_Pin GPIO_PIN_2
#define LED2_DBG_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
