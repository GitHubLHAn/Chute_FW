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
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define THAPCOI_Pin GPIO_PIN_0
#define THAPCOI_GPIO_Port GPIOA
#define THAPLED3_Pin GPIO_PIN_1
#define THAPLED3_GPIO_Port GPIOA
#define THAPLED2_Pin GPIO_PIN_2
#define THAPLED2_GPIO_Port GPIOA
#define THAPLED1_Pin GPIO_PIN_3
#define THAPLED1_GPIO_Port GPIOA
#define ADDR0_Pin GPIO_PIN_4
#define ADDR0_GPIO_Port GPIOA
#define ADDR1_Pin GPIO_PIN_5
#define ADDR1_GPIO_Port GPIOA
#define ADDR2_Pin GPIO_PIN_6
#define ADDR2_GPIO_Port GPIOA
#define ADDR3_Pin GPIO_PIN_7
#define ADDR3_GPIO_Port GPIOA
#define ADDR4_Pin GPIO_PIN_0
#define ADDR4_GPIO_Port GPIOB
#define ADDR5_Pin GPIO_PIN_1
#define ADDR5_GPIO_Port GPIOB
#define ADDR6_Pin GPIO_PIN_2
#define ADDR6_GPIO_Port GPIOB
#define ADDR7_Pin GPIO_PIN_10
#define ADDR7_GPIO_Port GPIOB
#define ADDR8_Pin GPIO_PIN_11
#define ADDR8_GPIO_Port GPIOB
#define LCD_CS_Pin GPIO_PIN_12
#define LCD_CS_GPIO_Port GPIOB
#define LCD_SCK_Pin GPIO_PIN_13
#define LCD_SCK_GPIO_Port GPIOB
#define ADDR9_Pin GPIO_PIN_14
#define ADDR9_GPIO_Port GPIOB
#define LCD_MOSI_Pin GPIO_PIN_15
#define LCD_MOSI_GPIO_Port GPIOB
#define LCD_RST_Pin GPIO_PIN_8
#define LCD_RST_GPIO_Port GPIOA
#define RS485_DE_Pin GPIO_PIN_11
#define RS485_DE_GPIO_Port GPIOA
#define BUTTON_Pin GPIO_PIN_3
#define BUTTON_GPIO_Port GPIOB
#define SENSOR_Pin GPIO_PIN_4
#define SENSOR_GPIO_Port GPIOB
#define LED_BAO_Pin GPIO_PIN_5
#define LED_BAO_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
