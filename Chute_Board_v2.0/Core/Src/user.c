/*
 * user.h
 * Created on: 18-July-2024
 * Author: Le Huu An
 */
 
 
#include "user.h"

#include "st7920.h"
#include "rs485.h"
#include "uart.h"
#include "flash_f103c8t6.h"

#include <string.h>
#include <stdio.h>

/*
NOTE: 


*/
/**********************************************************************************************************************************/
/******EXTERN*******/
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim1;

/**********************************************************************************************************************************/
/*******DECLARE VARIABLE********/
	uint8_t chute_isFull = 0;
	uint8_t chute_status =  CHUTE_READY;
	
	uint8_t ROW0[LCD_SIZE_ROW] = {0};
	uint8_t ROW1[LCD_SIZE_ROW] = {0};
	uint8_t ROW2[LCD_SIZE_ROW] = {0};
	uint8_t ROW3[LCD_SIZE_ROW] = {0};
	
	uint16_t count_packet = 0;
	
	volatile uint16_t cnt_flag_led_DB = 0;
	volatile bool flag_led_DB = 0;
	
	volatile bool flag_sensor_1ms = false;
	volatile bool flag_button_1ms = false;
	volatile uint16_t cnt_flag_led_chute = 0;
	
	uint8_t status_sensor = 0;
	uint16_t cnt_sensor_on = 0;

	volatile uint16_t cnt_flag_display_lcd = 0;
	bool flag_button_lcd = false;
	bool flag_newPacket_lcd = false;
//	bool flag_config_OK_lcd = false;
//	bool flag_config_ERROR_lcd = false;
	
	uint8_t status_thap_coi = 0x00;	
	uint8_t status_thap_led = 0x00;
	
	uint8_t getButton = 0;
	uint16_t cnt_button_on = 0;
	uint8_t button_Action = 0x00;

	bool flag_connect_app = false;
	volatile uint16_t cnt_ping = 0; //TIME_CHECK_CONNECTION*1000 + 1;
	
	uint8_t Chute_Addr_Arr[16] = {'D','C',':',' ',0,0,0,0,0,0,0,0,0,0,0,0};
	uint8_t Chute_Infor_update_arr[12] = {0,0,0,0,0,0,0,0,0,0,0,0};

	volatile uint16_t cnt_flag_thap_red = 0;
	volatile uint16_t cnt_flag_thap_yellow = 0;
	volatile uint16_t cnt_flag_thap_green = 0;
	volatile uint16_t cnt_flag_thap_coi = 0;
	
	
/**********************************************************************************************************************************/
/*******FUNCTION********/

/*================================================================================*/
/*																GET TIME 																				*/
/*================================================================================*/
	volatile uint32_t tim_tick_1ms = 0;
	
	uint32_t getMicroSecond(void)
 	{
		return tim_tick_1ms*1000 + __HAL_TIM_GetCounter(&htim2);
	}

/*================================================================================*/
/*														CALCULATING CHECKSUM																*/
/*================================================================================*/
		
	uint8_t Cal_CheckSum(uint8_t *Buff, uint8_t length)
	{
		uint8_t result = 0x00;
		
		for(uint8_t i=0; i<length-1; i++)
			result += Buff[i];
		
		return result;
	}
	
/*================================================================================*/
/*															FLASH THE LED DEBUG 															*/
/*================================================================================*/
	volatile uint16_t cnt_flag_flashLed = 0;
	volatile bool flag_flashLed = 0;
	
	void Flash_Led(void)
	{
		if(flag_flashLed)
		{
			HAL_GPIO_WritePin(LED_DEBUG_GPIO_Port, LED_DEBUG_Pin, GPIO_PIN_SET);
			if(++cnt_flag_flashLed >= 10)
			{
				HAL_GPIO_WritePin(LED_DEBUG_GPIO_Port, LED_DEBUG_Pin, GPIO_PIN_RESET);
				cnt_flag_flashLed = 0;
				flag_flashLed = 0;
			}
		}
	}

/*================================================================================*/
/*																BLINK THE LED DEBUG															*/
/*================================================================================*/
	void BlinkLed_Debug(void)
	{
		if(cnt_flag_led_DB >= 300){
			HAL_GPIO_TogglePin(LED_DEBUG_GPIO_Port, LED_DEBUG_Pin);
			cnt_flag_led_DB = 0;	
		}
	}
	
	void Toggle_Led_RED(uint16_t time){
		if(cnt_flag_thap_red >= time){
			TOGGLE_LED_RED();
			cnt_flag_thap_red = 0;
		}
	}
	
	void Toggle_Led_YELLOW(uint16_t time){
		if(cnt_flag_thap_yellow >= time){
			TOGGLE_LED_YELLOW();
			cnt_flag_thap_yellow = 0;
		}
	}
	
	void Toggle_Led_Green(uint16_t time){
		if(cnt_flag_thap_green >= time){
			TOGGLE_LED_GREEN();
			cnt_flag_thap_green = 0;
		}
	}
	
	void Toggle_Speaker(uint16_t time){
		if(cnt_flag_thap_coi >= time){
			TOGGLE_SPEAKER();
			cnt_flag_thap_coi = 0;
		}
	}

/*================================================================================*/
/*																GET SENSOR																			*/
/*================================================================================*/	
	uint8_t Get_Sensor(void)
	{	
		/*Notice: status_sensor = 1 when readpin = 1*/
		status_sensor = HAL_GPIO_ReadPin(SENSOR_GPIO_Port, SENSOR_Pin);
		if(flag_sensor_1ms)
		{
			if(status_sensor == 1)
			{
				if(cnt_sensor_on == 3000)		// 3 seconds
				{
					chute_isFull = 1;
				}
				else
				{
					cnt_sensor_on++;
					if(cnt_sensor_on == 5)	
						flag_newPacket_lcd = true;
				}
			}
			else
			{
				cnt_sensor_on = 0;
				chute_isFull = 0;
			}
			
			flag_sensor_1ms = false;			
		}		
		return chute_isFull;
	}

/*================================================================================*/
/*																DISPLAY LCD																			*/
/*================================================================================*/
	void Display_LCD(uint16_t cycle)
	{

		if(cnt_flag_display_lcd >= cycle)
		{
			memset(ROW0, 0x20, LCD_SIZE_ROW); memset(ROW1, 0x20, LCD_SIZE_ROW);
			memset(ROW2, 0x20, LCD_SIZE_ROW); memset(ROW3, 0x20, LCD_SIZE_ROW);
			
//			memset(ROW0, ' ', LCD_SIZE_ROW); memset(ROW1, ' ', LCD_SIZE_ROW);
//			memset(ROW2, ' ', LCD_SIZE_ROW); memset(ROW3, ' ', LCD_SIZE_ROW);
			
			ST7920_SendString(3, 0, (char*)ROW3);
			ST7920_SendString(2, 0, (char*)ROW2);
			ST7920_SendString(1, 0, (char*)ROW1);
			ST7920_SendString(0, 0, (char*)ROW0);
			
			sprintf((char*)ROW0, "%s", Chute_Addr_Arr);
			//((char*)ROW0, "ID: %d", vRS485.ID_Board);
			
			switch(chute_status)
			{
			//-----------------------------------------------------------	
				case CHUTE_NORMAL:
				{
					if(count_packet == 0xFFFF){
						sprintf((char*)ROW1, "So luong: *** ");
					}
					else
						sprintf((char*)ROW1, "So luong: %d ", count_packet);
					
					/*Chute Status*/
					sprintf((char*)ROW2, "Mang Mo");	
						
					if(flag_button_lcd)
					{
						sprintf((char*)ROW3, "Nhan nut Dong");
						flag_button_lcd = false;
					}	
					break;
				}
			//-----------------------------------------------------------	
				case CHUTE_CLOSE:
				{
					if(count_packet == 0xFFFF){
						sprintf((char*)ROW1, "So luong: *** ");
					}
					else
						sprintf((char*)ROW1, "So luong: %d ", count_packet);
					
					/*Chute Status*/
					sprintf((char*)ROW2, "Mang Dong");
					
					if(flag_button_lcd)
					{
						sprintf((char*)ROW3, "Nhan nut Mo");
						flag_button_lcd = false;
					}
					break;
				}
			//-----------------------------------------------------------	
				case CHUTE_FULL:
				{
					if(count_packet == 0xFFFF){
						sprintf((char*)ROW1, "So luong: *** ");
					}
					else
						sprintf((char*)ROW1, "So luong: %d ", count_packet);
					
					/*Chute Status*/
					sprintf((char*)ROW2, "Mang day");	
					
					if(flag_button_lcd)
					{
						sprintf((char*)ROW3, "Nhan nut Dong");
						flag_button_lcd = false;
					}	
					break;
				}
			//-----------------------------------------------------------	
				case NOT_CONNECT:
				{
					sprintf((char*)ROW1, "Khong ket noi");
					break;
				}
			}		
			//-----------------------------------------------------------	
				
			/*Update LCD*/
			ST7920_SendString(3, 0, (char*)ROW3);
			ST7920_SendString(2, 0, (char*)ROW2);
			ST7920_SendString(1, 0, (char*)ROW1);
			ST7920_SendString(0, 0, (char*)ROW0);
			
			cnt_flag_display_lcd = 0;
		}
		else if(flag_newPacket_lcd && cnt_flag_display_lcd >= 100)
		{
			flag_newPacket_lcd = false;
			
//			memset(ROW0, ' ', LCD_SIZE_ROW); memset(ROW1, ' ', LCD_SIZE_ROW);
//			memset(ROW2, ' ', LCD_SIZE_ROW); memset(ROW3, ' ', LCD_SIZE_ROW);
			
			memset(ROW0, 0x20, LCD_SIZE_ROW); memset(ROW1, 0x20, LCD_SIZE_ROW);
			memset(ROW2, 0x20, LCD_SIZE_ROW); memset(ROW3, 0x20, LCD_SIZE_ROW);
			
			sprintf((char*)ROW0, "      ||||      ");
			sprintf((char*)ROW1, "      ||||      ");
			sprintf((char*)ROW2, "     \\\\\\///    ");
			sprintf((char*)ROW3, "       \\/      ");
			
			ST7920_SendString(0, 0, (char*)ROW0);
			ST7920_SendString(1, 0, (char*)ROW1);
			ST7920_SendString(2, 0, (char*)ROW2);
			ST7920_SendString(3, 0, (char*)ROW3);
		
			cnt_flag_display_lcd = cycle - 500;
		}				
	}
	
/*================================================================================*/
/*													HANDLE THAPCOI																				*/
/*================================================================================*/	
	void Handle_Thap_Coi(void)
	{
		if(status_thap_coi == 0x01)		
			Toggle_Speaker(500);
			//ON_SPEAKER();
		else if(status_thap_coi == 0x02)
			Toggle_Speaker(150);
		else
			OFF_SPEAKER();
		
	}

/*================================================================================*/
/*															HANDLE THAP LED																		*/
/*================================================================================*/
	void Handle_Thap_Led(void)
	{	
		if(status_thap_led == 1)
		{
			OFF_LED_RED();
			OFF_LED_YELLOW();
			ON_LED_GREEN();
		}
		else if(status_thap_led == 2)
		{
			OFF_LED_RED();
			//ON_LED_YELLOW();
			Toggle_Led_YELLOW(500);
			OFF_LED_GREEN();
			
		}
		else if(status_thap_led == 3)
		{
			//OFF_LED_RED();
			Toggle_Led_RED(500);
			OFF_LED_YELLOW();
			OFF_LED_GREEN();
		}
		else{
			OFF_LED_RED();
			OFF_LED_YELLOW();
			OFF_LED_GREEN();
		}
	}
	
/*================================================================================*/
/*															GET BUTTON																				*/
/*================================================================================*/
	bool isButton = false;
	uint16_t cnt_Button_on = 0, cnt_isButton, cnt_free1;
	
	bool isPressed_Reset = false;
	uint16_t cnt_Button_on_Reset = 0, cnt_isButton_Reset = 0;
	uint8_t cnt_Reset = 0;
	
	uint16_t cnt_time_reset;
	
	void Get_Button(void)
	{
		getButton = !HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin);
		if(flag_button_1ms)
		{
		//----------------------Press Button------------------------------------------------
		// Press and hold Button in 1 second for Press Fucntion
			if(getButton && isButton == false)
			{	
				if(cnt_Button_on == 1000)
				{
					isButton = true;		
					cnt_Reset = 0;
					
					if(chute_status == CHUTE_NORMAL || chute_status == CHUTE_FULL){
						button_Action = 0x01;
						flag_button_lcd = true;
					}
					else if(chute_status == CHUTE_CLOSE){
						button_Action = 0x02;
						flag_button_lcd = true;
					}
					else if(chute_status == CHUTE_READY)
						chute_status = CHUTE_NORMAL;
					cnt_free1++;		// use for debug
					cnt_flag_display_lcd = CYCLE_LCD - 100;
				}else{
					cnt_Button_on++;
				}
			}else{
				cnt_Button_on = 0;
			}
			
			if(isButton == true)
			{
				if(++cnt_isButton >= 5000){
					isButton = 0;
					cnt_isButton = 0;
				}
			}
		//-----------------------Soft Reset-----------------------------------------------
		// Press 5 times button to soft reset Chute Board
			if(getButton && isPressed_Reset == false)
			{
				if(cnt_Button_on_Reset == 100){
					isPressed_Reset = true;
					cnt_Reset++;
				}
				else{
					cnt_Button_on_Reset++;
				}
			}
			else{
				cnt_Button_on_Reset = 0;
			}
			if(isPressed_Reset == true){
				if(++cnt_isButton_Reset >= 500){
						isPressed_Reset = false;
						cnt_isButton_Reset = 0;
					}
			}
			
			if(cnt_Reset >= 5){			// Press 5 times to soft reset
				NVIC_SystemReset();
			}
			
			if(cnt_Reset > 0){
				if(++cnt_time_reset >= 5000){
					isPressed_Reset = false;
					cnt_Button_on_Reset = 0;
					cnt_isButton_Reset = 0;
					cnt_Reset = 0;
					cnt_time_reset = 0;
				}
			}	
		//----------------------------------------------------------------------
			flag_button_1ms = false;
		}
	}
	
/*================================================================================*/
/*												CHECK CONNECTION WITH APP		 	      									*/
/*================================================================================*/
	void Check_Connection_App(void)
	{
 		if(cnt_ping == TIME_CHECK_CONNECTION*1000)		
		{
			cnt_ping = TIME_CHECK_CONNECTION*1000 + 1;
			chute_status = NOT_CONNECT;
		}
	}
	
/*================================================================================*/
/*												CHECK UPDATE ADDRESS CHUTE		 	      									*/
/*================================================================================*/
	void Check_Update_Addr_Chute(void)
	{			
		if(Update_Addr_Chute() == UPDATE_SUCCESS)
		{
//***			/*Update Chute Address*/
//			for(uint8_t i=4; i<16; i++)
//				Chute_Addr_Arr[i] = Chute_Infor_update_arr[i-4];				
			
			memcpy(vRS485.txBuff_config, vRS485.rxBuff_config, RS485_NUM_BYTE_CONFIG);
			vRS485.TxFlag_config = 1;
		}
		else
		{
			/*Generate error response mess*/
			memcpy(vRS485.txBuff_config, vRS485.rxBuff_config, RS485_NUM_BYTE_CONFIG);
			for(uint8_t m=3; m<RS485_NUM_BYTE_CONFIG-1; m++)
				vRS485.txBuff_config[m] = 0xEE;
			vRS485.txBuff_config[RS485_NUM_BYTE_CONFIG-1] = Cal_CheckSum(vRS485.txBuff_config, RS485_NUM_BYTE_CONFIG);
			
			/*Clear Flash*/
			Flash_Erase(ADDRES_DATA_STORAGE);
			 
			vRS485.TxFlag_config = 1;
		}	
	}
	
/*========================================================================================*\
 *																	MAIN PROCESS																					*
\*========================================================================================*/	
	void Process_Chute(void)
	{
		/*Get and handle button*/
		Get_Button();
		
		/*Handle Thap LED+COI*/
		Handle_Thap_Led();
		Handle_Thap_Coi();
		
		switch(chute_status)
		{
		//-----------------------------------------------------------	
			case CHUTE_READY:
			{
				/*Supply Power for peripheral*/
				HAL_GPIO_WritePin(SLEEP_MODE_CTL_GPIO_Port,SLEEP_MODE_CTL_Pin,GPIO_PIN_RESET);
				
				/*Not display LCD periodically*/
				cnt_flag_display_lcd = 0;	
				flag_newPacket_lcd = 0;		
				
				/*First connect with app*/
				if(flag_connect_app){
					cnt_flag_display_lcd = CYCLE_LCD;
					chute_status = CHUTE_NORMAL;
				}			
				break;
			}
		/*---------------------------------------------------------------*/
			case CHUTE_NORMAL:
			{
				/*Supply Power for peripheral*/
				HAL_GPIO_WritePin(SLEEP_MODE_CTL_GPIO_Port,SLEEP_MODE_CTL_Pin,GPIO_PIN_RESET);
				
				/*Get sensor*/
				Get_Sensor();
				
				/*Turn Off LED_BAO*/
				OFF_LED_BAO();
				
				break;
			}
		/*---------------------------------------------------------------*/
			case CHUTE_CLOSE:
			{				
				/*Supply Power for peripheral*/
				HAL_GPIO_WritePin(SLEEP_MODE_CTL_GPIO_Port,SLEEP_MODE_CTL_Pin,GPIO_PIN_RESET);
				
				/*Turn On LED_BAO*/
				ON_LED_BAO();
				
				break;
			}
		/*---------------------------------------------------------------*/
			case CHUTE_FULL:
			{
				/*Supply Power for peripheral*/
				HAL_GPIO_WritePin(SLEEP_MODE_CTL_GPIO_Port,SLEEP_MODE_CTL_Pin,GPIO_PIN_RESET);
				
				/*Get sensor*/
				Get_Sensor();
				
				/*Blink Led Chute*/
				if(cnt_flag_led_chute >= 300)
				{
					HAL_GPIO_TogglePin(LED_BAO_GPIO_Port, LED_BAO_Pin);
					cnt_flag_led_chute = 0;
				}	

				break;
			}
		/*---------------------------------------------------------------*/
			case NOT_CONNECT:
			{				
//				if(HAL_GPIO_ReadPin(SLEEP_MODE_DET_GPIO_Port, SLEEP_MODE_DET_Pin) == 1)
//				{
//					/*Supply Power for peripheral*/
//					HAL_GPIO_WritePin(SLEEP_MODE_CTL_GPIO_Port,SLEEP_MODE_CTL_Pin,GPIO_PIN_RESET);
//				}
//				else{
//					/*Power Off for peripheral*/
//					HAL_GPIO_WritePin(SLEEP_MODE_CTL_GPIO_Port,SLEEP_MODE_CTL_Pin,GPIO_PIN_SET);
//				}
				if(vChute_Infor_temp.mode_sleep == 0x00)
				{
					/*Supply Power for peripheral*/
					HAL_GPIO_WritePin(SLEEP_MODE_CTL_GPIO_Port,SLEEP_MODE_CTL_Pin,GPIO_PIN_RESET);
				}
				else{
					/*Power Off for peripheral*/
					HAL_GPIO_WritePin(SLEEP_MODE_CTL_GPIO_Port,SLEEP_MODE_CTL_Pin,GPIO_PIN_SET);
				}
				
				/*Wake up Chute*/
				if(cnt_ping < TIME_CHECK_CONNECTION*1000)	
				{
					HAL_TIM_Base_Stop_IT(&htim1);
					HAL_GPIO_WritePin(SLEEP_MODE_CTL_GPIO_Port,SLEEP_MODE_CTL_Pin,GPIO_PIN_RESET);
					HAL_Delay(100);
					cnt_flag_display_lcd = CYCLE_LCD - 500;
					chute_status = CHUTE_NORMAL;
					
					HAL_TIM_Base_Start_IT(&htim1);
					ST7920_Init();
					ST7920_GraphicMode(0);
					ST7920_Clear();
				}		
				
				/*Turn Off LED_BAO*/
				OFF_LED_BAO();
				
				break;
			}
		/*---------------------------------------------------------------*/
			case CHUTE_CONFIG:
			{
				/*Turn On LED_BAO*/
				ON_LED_BAO();
				
				/*Stop interrupt timer to update Flash Memory*/
				HAL_TIM_Base_Stop_IT(&htim1);
				HAL_TIM_Base_Stop_IT(&htim2);
				
				/*Update Flash Memory*/
				Check_Update_Addr_Chute();
				
				//chute_status = CHUTE_READY;
				break;
			}
	
		}

	}
/*========================================================================================*\
 *													CALLBACK FUNCTION                                             *
\*========================================================================================*/	
	
/*______________________________________________________________________________*/
	
	void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) 
	{  
		/*RS485*/
		if(huart->Instance == USART3)			
		{    
			RS485_Read(&vRS485);
			RS485_Rx_Handle(&vRS485);
		}
		
		/*UART Debug*/
		if(huart->Instance == USART1)
		{    
			UART_Read(&vUART_DB);
			Uart_Rx_Handle(&vUART_DB);
		}
	}

/*______________________________________________________________________________*/	
	
	void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
			UNUSED(htim);
			
			if (htim->Instance == htim2.Instance) 	// every 1ms		
			{
				/*Increase cnt variable*/
				cnt_flag_led_DB++;
				cnt_flag_display_lcd++; 
				cnt_flag_led_chute++;
				if(cnt_ping < TIME_CHECK_CONNECTION*1000 && chute_status != CHUTE_CLOSE && chute_status != NOT_CONNECT) 
					cnt_ping++;
				cnt_flag_thap_red++;
				cnt_flag_thap_yellow++;
				cnt_flag_thap_green++;
				cnt_flag_thap_coi++;
				
				/*On Flag*/
				flag_sensor_1ms = true;
				flag_button_1ms = true;
				
				/*Handle function*/
				Flash_Led();
			}	

		}	

/**********************************************************************************************************************************/
/*******END PAGE********/



