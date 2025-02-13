/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

	#include <string.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <stdbool.h>

	#include "st7920.h"
	#include "rs485.h"
	#include "user.h"
	#include "flash_f103c8t6.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */


	uint8_t ID_Slave_arr[56];
	uint8_t ID_ptr, ID_Start, ID_End;
	uint16_t Count_RX_Slave[56];
	uint16_t Count_TX_Slave[56];
	uint16_t qty = 0;
	
	uint16_t cnt_AutoSend = 0;
	uint8_t vChute_isFull = 0;
	uint8_t chute_full_arr[56];
	uint8_t vSum_chute_full = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
		HAL_GPIO_WritePin(SLEEP_MODE_CTL_GPIO_Port,SLEEP_MODE_CTL_Pin,GPIO_PIN_RESET);
		
		OFF_LED_BAO();
		OFF_SPEAKER();
		ON_LED_RED();
		ON_LED_GREEN();
		ON_LED_YELLOW();
		
		for(uint8_t i=1; i<56; i++){
			ID_Slave_arr[i] = i;
			chute_full_arr[i] = 0;
			Count_RX_Slave[i] = 0;
			Count_TX_Slave[i] = 0;
		}
		ID_Start = 1;
		ID_End = 16; 
		ID_ptr = ID_Start;
				
		
		RS485_Init(&vRS485, &huart3);
			
		/*INit LCD Display*/
		HAL_TIM_Base_Start_IT(&htim1);
		ST7920_Init();
		
		ST7920_GraphicMode(1);
		//ST7920_DrawBitmap(viettel_post);
		ST7920_DrawBitmap(Yody);
		
		HAL_Delay(3000);
		
		ST7920_GraphicMode(0);
		ST7920_Clear();
		
		sprintf((char*)ROW0, "YODY XIN CHAO");
		sprintf((char*)ROW1, "MACH MANG");
		sprintf((char*)ROW2, "ID: %d", vRS485.ID_Board);
		//sprintf((char*)ROW3, "YODY");
		
		ST7920_SendString(3, 0, (char*)ROW3);
		ST7920_SendString(2, 0, (char*)ROW2);
		ST7920_SendString(1, 0, (char*)ROW1);
		ST7920_SendString(0, 0, (char*)ROW0);
		
		//HAL_Delay(1000);
		
		//Flash_Erase(ADDRES_DATA_STORAGE);
		Get_Address_Chute();
		
		HAL_TIM_Base_Start_IT(&htim2);

		HAL_GPIO_WritePin(LED_DEBUG_GPIO_Port, LED_DEBUG_Pin, GPIO_PIN_RESET);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	/*----------------------------------------------------*/
		//Check_Connection_App();
		
	/*----------------------------------------------------*/
		Process_Chute();
	
	/*----------------------------------------------------*/
		//RS485_Transmit(&vRS485);
		
	/*----------------------------------------------------*/
		Display_LCD(CYCLE_LCD);
		
	/*----------------------------------------------------*/
		BlinkLed_Debug();
		
	/*---------------------------------------------------------------------------------*/
	/*---------------------------------------------------------------------------------*/
	/*---------------------------------------------------------------------------------*/
	/*---------------------------------------------------------------------------------*/
		if(cnt_AutoSend >= 100)
		{
			HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET);
			
			
			
			vRS485.txBuff[0] = 0xAB;		// header
			vRS485.txBuff[1] = 0x00;		// ID high
			vRS485.txBuff[2] =  ID_Slave_arr[ID_ptr];		// ID low
			vRS485.txBuff[3] = (qty >> 8) & 0xFF;		// qty high
			vRS485.txBuff[4] = qty & 0xFF;		// qty low
			vRS485.txBuff[5] = 0x00;		// chute status
			if(vChute_isFull == 1){
				vRS485.txBuff[7] = 0x01;		// thap coi
				vRS485.txBuff[6] = 0x01;		// thap led red
			}
			else{
				vRS485.txBuff[6] = 0x03;		// thap led green
				vRS485.txBuff[7] = 0x00;		// thap coi
			}
			vRS485.txBuff[8] = 0xFE;		// du phong
			vRS485.txBuff[9] = Cal_CheckSum(vRS485.txBuff, RS485_NUM_BYTE);		// check sum
		
			HAL_UART_Transmit(&huart3, vRS485.txBuff, RS485_NUM_BYTE, HAL_MAX_DELAY);
			Count_TX_Slave[vRS485.txBuff[2]]++;
			
			HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);
			//memset(vRS485.txBuff, 0xE0, RS485_NUM_BYTE);
			vRS485.cnt_mess_sent++;	
		
			if(++ID_ptr > ID_End){
				ID_ptr = ID_Start;
				qty++;
			}
		
			cnt_AutoSend = 0;
		}
		vSum_chute_full = 0;
		for(uint8_t j=1; j <56; j++)
		{
		
			vSum_chute_full += chute_full_arr[j];		
		}
		if(vSum_chute_full != 0){
			vChute_isFull = 1;
			ON_LED_BAO();
		}
		else{
			vChute_isFull = 0;
			OFF_LED_BAO();
		}
		
		
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 71;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 71;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 57600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_DEBUG_GPIO_Port, LED_DEBUG_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, RS485_DE_Pin|LCD_CS_Pin|LCD_CLK_Pin|LCD_MOSI_Pin
                          |LED_BAO_Pin|THAPCOI_Pin|THAPLED_GREEN_Pin|THAPLED_YELLOW_Pin
                          |THAPLED_REG_Pin|SLEEP_MODE_CTL_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_DEBUG_Pin */
  GPIO_InitStruct.Pin = LED_DEBUG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_DEBUG_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ADDR0_Pin ADDR1_Pin ADDR2_Pin ADDR3_Pin
                           ADDR4_Pin ADDR5_Pin ADDR6_Pin ADDR7_Pin
                           ADDR10_Pin ADDR11_Pin */
  GPIO_InitStruct.Pin = ADDR0_Pin|ADDR1_Pin|ADDR2_Pin|ADDR3_Pin
                          |ADDR4_Pin|ADDR5_Pin|ADDR6_Pin|ADDR7_Pin
                          |ADDR10_Pin|ADDR11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : ADDR8_Pin ADDR9_Pin LCD_RST_Pin BUTTON_Pin */
  GPIO_InitStruct.Pin = ADDR8_Pin|ADDR9_Pin|LCD_RST_Pin|BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : RS485_DE_Pin */
  GPIO_InitStruct.Pin = RS485_DE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(RS485_DE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_CS_Pin LCD_CLK_Pin LCD_MOSI_Pin LED_BAO_Pin
                           THAPCOI_Pin THAPLED_GREEN_Pin THAPLED_YELLOW_Pin THAPLED_REG_Pin
                           SLEEP_MODE_CTL_Pin */
  GPIO_InitStruct.Pin = LCD_CS_Pin|LCD_CLK_Pin|LCD_MOSI_Pin|LED_BAO_Pin
                          |THAPCOI_Pin|THAPLED_GREEN_Pin|THAPLED_YELLOW_Pin|THAPLED_REG_Pin
                          |SLEEP_MODE_CTL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : SLEEP_MODE_DET_Pin SENSOR_Pin */
  GPIO_InitStruct.Pin = SLEEP_MODE_DET_Pin|SENSOR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
