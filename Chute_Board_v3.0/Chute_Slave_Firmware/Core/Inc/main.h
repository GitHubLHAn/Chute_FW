/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#define BIT_7_Pin GPIO_PIN_3
#define BIT_7_GPIO_Port GPIOA
#define BIT_6_Pin GPIO_PIN_4
#define BIT_6_GPIO_Port GPIOA
#define BIT_5_Pin GPIO_PIN_5
#define BIT_5_GPIO_Port GPIOA
#define BIT_4_Pin GPIO_PIN_6
#define BIT_4_GPIO_Port GPIOA
#define BIT_3_Pin GPIO_PIN_7
#define BIT_3_GPIO_Port GPIOA
#define BIT_2_Pin GPIO_PIN_0
#define BIT_2_GPIO_Port GPIOB
#define BIT_1_Pin GPIO_PIN_1
#define BIT_1_GPIO_Port GPIOB
#define BIT_0_Pin GPIO_PIN_2
#define BIT_0_GPIO_Port GPIOB
#define USART3_TX_DB_Pin GPIO_PIN_10
#define USART3_TX_DB_GPIO_Port GPIOB
#define USART3_RX_DB_Pin GPIO_PIN_11
#define USART3_RX_DB_GPIO_Port GPIOB
#define LCD_CS_Pin GPIO_PIN_12
#define LCD_CS_GPIO_Port GPIOB
#define LCD_CLK_Pin GPIO_PIN_13
#define LCD_CLK_GPIO_Port GPIOB
#define LCD_RST_Pin GPIO_PIN_14
#define LCD_RST_GPIO_Port GPIOB
#define LCD_MOSI_Pin GPIO_PIN_15
#define LCD_MOSI_GPIO_Port GPIOB
#define RS485_DE_Pin GPIO_PIN_8
#define RS485_DE_GPIO_Port GPIOA
#define USART1_TX_RS485_Pin GPIO_PIN_9
#define USART1_TX_RS485_GPIO_Port GPIOA
#define USART1_RX_RS485_Pin GPIO_PIN_10
#define USART1_RX_RS485_GPIO_Port GPIOA
#define LED_RS485_TRX_Pin GPIO_PIN_11
#define LED_RS485_TRX_GPIO_Port GPIOA
#define LED_DEBUG_Pin GPIO_PIN_12
#define LED_DEBUG_GPIO_Port GPIOA
#define SENSOR_DET_Pin GPIO_PIN_15
#define SENSOR_DET_GPIO_Port GPIOA
#define BUTTON_DET_Pin GPIO_PIN_3
#define BUTTON_DET_GPIO_Port GPIOB
#define LED_BAO_CTL_Pin GPIO_PIN_4
#define LED_BAO_CTL_GPIO_Port GPIOB
#define SENSOR_BACKUP_DET_Pin GPIO_PIN_9
#define SENSOR_BACKUP_DET_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
