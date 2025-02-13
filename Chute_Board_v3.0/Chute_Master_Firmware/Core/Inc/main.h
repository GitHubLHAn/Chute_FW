/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

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
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define UART2_TX_SLAVEA_Pin GPIO_PIN_2
#define UART2_TX_SLAVEA_GPIO_Port GPIOA
#define UART2_RX_SLAVEB_Pin GPIO_PIN_3
#define UART2_RX_SLAVEB_GPIO_Port GPIOA
#define BIT6_Pin GPIO_PIN_4
#define BIT6_GPIO_Port GPIOA
#define BIT5_Pin GPIO_PIN_5
#define BIT5_GPIO_Port GPIOA
#define BIT4_Pin GPIO_PIN_6
#define BIT4_GPIO_Port GPIOA
#define BIT3_Pin GPIO_PIN_7
#define BIT3_GPIO_Port GPIOA
#define BIT2_Pin GPIO_PIN_0
#define BIT2_GPIO_Port GPIOB
#define BIT1_Pin GPIO_PIN_1
#define BIT1_GPIO_Port GPIOB
#define BIT0_Pin GPIO_PIN_2
#define BIT0_GPIO_Port GPIOB
#define UART3_TX_SLAVEB_Pin GPIO_PIN_10
#define UART3_TX_SLAVEB_GPIO_Port GPIOB
#define UART3_RX_SLAVEB_Pin GPIO_PIN_11
#define UART3_RX_SLAVEB_GPIO_Port GPIOB
#define LED_TRX_SLAVEB_Pin GPIO_PIN_12
#define LED_TRX_SLAVEB_GPIO_Port GPIOB
#define LED_TRX_SLAVEA_Pin GPIO_PIN_13
#define LED_TRX_SLAVEA_GPIO_Port GPIOB
#define RS485_SLAVEB_DE_Pin GPIO_PIN_14
#define RS485_SLAVEB_DE_GPIO_Port GPIOB
#define RS485_SLAVEA_DE_Pin GPIO_PIN_15
#define RS485_SLAVEA_DE_GPIO_Port GPIOB
#define RS485_APP_DE_Pin GPIO_PIN_8
#define RS485_APP_DE_GPIO_Port GPIOA
#define UART1_TX_APP_Pin GPIO_PIN_9
#define UART1_TX_APP_GPIO_Port GPIOA
#define UART1_RX_APP_Pin GPIO_PIN_10
#define UART1_RX_APP_GPIO_Port GPIOA
#define LED_TRX_APP_Pin GPIO_PIN_11
#define LED_TRX_APP_GPIO_Port GPIOA
#define LED_DEBUG_Pin GPIO_PIN_12
#define LED_DEBUG_GPIO_Port GPIOA
#define SPEAKER_Pin GPIO_PIN_5
#define SPEAKER_GPIO_Port GPIOB
#define LAMP_GREEN_Pin GPIO_PIN_6
#define LAMP_GREEN_GPIO_Port GPIOB
#define LAMP_YELLOW_Pin GPIO_PIN_7
#define LAMP_YELLOW_GPIO_Port GPIOB
#define LAMP_RED_Pin GPIO_PIN_8
#define LAMP_RED_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
