/*
 * user.c
 * Created on: 18-DEC-2024
 * Author: Le Huu An (anlh55@viettel.com.vn)
 * This version of chute firmware is upgraded with new system of
 * chute with 2 device (master and slave, 1 master manages 30-60 slave).
 */
 
 
#include "user.h"

#include "st7920.h"
#include "rs485.h"
#include "flash_address.h"

#include <string.h>
#include <stdio.h>

/*
NOTE: 
	- Slave FW Version 3.0
	- No VIETTEL POST bitmap
	- Have Calib mode
	- Handle button as state machine: hold press to reset

*/
/**********************************************************************************************************************************/
/******EXTERN*******/
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;

/**********************************************************************************************************************************/
/*******DECLARE VARIABLE********/
	volatile uint32_t count_synchronize = 0;
		
	volatile uint8_t chute_isFull = 0;
	volatile uint8_t chute_status =  CHUTE_READY;
	
	char ROW0[LCD_SIZE_ROW] = {0};
	char ROW1[LCD_SIZE_ROW] = {0};
	char ROW2[LCD_SIZE_ROW] = {0};
	char ROW3[LCD_SIZE_ROW] = {0};
	
	volatile uint8_t qty_packet = 0;
	
	volatile uint16_t cnt_flag_led_DB = 0;
	volatile bool flag_led_DB = 0;
	
	volatile bool flag_sensor_1ms = false;
//	volatile bool flag_button_1ms = false;
	
	uint8_t status_sensor = 0;
	volatile uint16_t cnt_sensor_on = 0;

	volatile uint16_t cnt_flag_display_lcd = 0;
	bool flag_button_lcd = false;
	bool flag_newPacket_lcd = false;
	
	uint8_t button_Action = 0x00;

	bool flag_run = false;					// start running variable
	volatile uint16_t cnt_ping = 0; //TIME_CHECK_CONNECTION*1000 + 1;
	
	uint8_t Chute_Addr_Arr[16];
	uint16_t vCycleLCD = 0;					// time refresh LCD
	uint16_t vTimeCheckCon = 0;			// time check connection with Master
	
	volatile uint32_t _s = 0, _i = 0;
	uint8_t Debug_arr[30];   

	
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
	volatile uint16_t cnt_flag_flashLed_debug = 0;
	volatile bool flag_flashLed_debug = 0;
	volatile uint16_t cnt_flag_flashLed_trx = 0;
	volatile bool flag_flashLed_trx = 0;
	
	void Flash_Led(void)
	{
		/*Flash led debug*/
		if(flag_flashLed_debug)
		{
			HAL_GPIO_WritePin(LED_DEBUG_GPIO_Port, LED_DEBUG_Pin, GPIO_PIN_SET);
			if(++cnt_flag_flashLed_debug >= 20)
			{
				HAL_GPIO_WritePin(LED_DEBUG_GPIO_Port, LED_DEBUG_Pin, GPIO_PIN_RESET);
				cnt_flag_flashLed_debug = 0;
				flag_flashLed_debug = 0;
			}
		}
		
		/*Flag led transceiver rs485*/
		if(flag_flashLed_trx)
		{
			HAL_GPIO_WritePin(LED_RS485_TRX_GPIO_Port, LED_RS485_TRX_Pin, GPIO_PIN_SET);
			if(++cnt_flag_flashLed_trx >= 2)
			{
				HAL_GPIO_WritePin(LED_RS485_TRX_GPIO_Port, LED_RS485_TRX_Pin, GPIO_PIN_RESET);
				cnt_flag_flashLed_trx = 0;
				flag_flashLed_trx = 0;
			}
		}
	}

/*================================================================================*/
/*																BLINK THE LED DEBUG															*/
/*================================================================================*/
	void BlinkLed_Debug(void)
	{
		if(cnt_flag_led_DB >= 2000)
		{
			HAL_GPIO_TogglePin(LED_DEBUG_GPIO_Port, LED_DEBUG_Pin);
			cnt_flag_led_DB = 0;	
			_i = getMicroSecond() - _s;
			_s = getMicroSecond();
		}
	}
	
/*================================================================================*/
/*																BLINK THE LED DEBUG															*/
/*================================================================================*/
	bool Check_Even_Number(uint32_t _num)
	{
		if(_num %2 == 0)
			return true;
		else
			return false;
	}

/*================================================================================*/
/*																GET SENSOR																			*/
/*================================================================================*/	
	uint8_t Get_Sensor(void)
	{	
		/*Notice: status_sensor = 1 when readpin = 0*/
		status_sensor = !HAL_GPIO_ReadPin(SENSOR_DET_GPIO_Port, SENSOR_DET_Pin);
//		status_sensor = HAL_GPIO_ReadPin(SENSOR_BACKUP_DET_GPIO_Port, SENSOR_BACKUP_DET_Pin);
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
			/*Clear LCD*/
			sprintf(ROW0, "                ");
			ST7920_SendString(3, 0, (char*)ROW3);
			sprintf(ROW1, "                ");
			ST7920_SendString(2, 0, (char*)ROW2);
			sprintf(ROW2, "                ");
			ST7920_SendString(1, 0, (char*)ROW1);
			sprintf(ROW3, "                ");
			ST7920_SendString(0, 0, (char*)ROW0);

			switch(chute_status)
			{
			//-----------------------------------------------------------	
				case CHUTE_OPEN:
				{
					for(uint8_t i=0; i<NUM_IDENTIFICATION; i++)
						ROW0[i] = vChute_Infor_cache.identification_arr[i];
					ST7920_SendString(0, 0, ROW0);
									
					if(qty_packet == 0xFF){
						sprintf(ROW1, "So luong: *** ");
					}
					else
						sprintf(ROW1, "So luong: %d ", qty_packet);
					ST7920_SendString(1, 0, ROW1);
					
					/*Chute Status*/
					if(chute_isFull == 1)
						sprintf(ROW2, "Mang day");
					else
						sprintf(ROW2, "Mang Mo");
					ST7920_SendString(2, 0, ROW2);
					
					if(flag_button_lcd)
					{
						sprintf(ROW3, "Nhan nut Dong");
						flag_button_lcd = false;
					}
					else{
						//sprintf(ROW3, "%d, %d, %d", vRS485.cnt_mess_received, vRS485.cnt_mess_exact, vRS485.cnt_mess_wrong);
					}
					ST7920_SendString(3, 0, ROW3);
					
					break;
				}
			//-----------------------------------------------------------	
				case CHUTE_CLOSE:
				{
					for(uint8_t i=0; i<NUM_IDENTIFICATION; i++)
						ROW0[i] = vChute_Infor_cache.identification_arr[i];
					ST7920_SendString(0, 0, ROW0);
					
					if(qty_packet == 0xFF){
						sprintf(ROW1, "So luong: *** ");
					}
					else
						sprintf(ROW1, "So luong: %d ", qty_packet);
					ST7920_SendString(1, 0, ROW1);
					
					/*Chute Status*/
					sprintf(ROW2, "Mang Dong");
					ST7920_SendString(2, 0, ROW2);
					
					if(flag_button_lcd)
					{
						sprintf(ROW3, "Nhan nut Mo");
						flag_button_lcd = false;
					}
					ST7920_SendString(3, 0, ROW3);
					
					break;
				}
			//-----------------------------------------------------------	
				case NOT_CONNECT:
				{
					sprintf(ROW0, "STT: %d", vRS485.Sequence_number);
					ST7920_SendString(0, 0, ROW0);
					sprintf(ROW1, "Khong ket noi");
					ST7920_SendString(1, 0, ROW1);
					
					break;
				}
			}		
			//-----------------------------------------------------------	
				
//			/*Update LCD*/
//			ST7920_SendString(3, 0, (char*)ROW3);	
//			ST7920_SendString(2, 0, (char*)ROW2);
//			ST7920_SendString(1, 0, (char*)ROW1);
//			ST7920_SendString(0, 0, (char*)ROW0);
				
			flag_button_lcd = false;
			cnt_flag_display_lcd = 0;
		}
		else if(flag_newPacket_lcd && cnt_flag_display_lcd >= cycle/3)
		{
			flag_newPacket_lcd = false;
			
			/*Clear LCD*/
			sprintf(ROW0, "                ");
			ST7920_SendString(3, 0, ROW3);
			sprintf(ROW1, "                ");
			ST7920_SendString(2, 0, ROW2);
			sprintf(ROW2, "                ");
			ST7920_SendString(1, 0, ROW1);
			sprintf(ROW3, "                ");
			ST7920_SendString(0, 0, ROW0);
				
			sprintf(ROW3, "       \\/      ");
			ST7920_SendString(3, 0, ROW3);
			sprintf(ROW2, "     \\\\\\///    ");
			ST7920_SendString(2, 0, ROW2);
			sprintf(ROW1, "      ||||      ");
			ST7920_SendString(1, 0, ROW1);
			sprintf(ROW0, "      ||||      ");
			ST7920_SendString(0, 0, ROW0);
	
			cnt_flag_display_lcd = cycle/3;
		}				
	}
	
/*================================================================================*/
/*															GET BUTTON																				*/
/*================================================================================*/
	uint8_t getButton = 0;
	uint16_t cnt_pushed = 0, cnt_return_free = 0, cnt_wait_next_push = 0;
	uint8_t button_status = BUTTON_FREE;	
	uint16_t cnt_pushed_reset = 0;
	uint16_t cnt_debug_bt = 0;
	
	void Get_Button(void)
	{
		getButton = !HAL_GPIO_ReadPin(BUTTON_DET_GPIO_Port, BUTTON_DET_Pin);

		//----------------------Push Button------------------------------------------------
		switch(button_status)
		{
			case BUTTON_FREE:
			{
				if(getButton){
					if(++cnt_pushed == 50){
						button_status = BUTTON_ACTIVE;
						cnt_pushed = 0;
					}
				}
				else{
					cnt_pushed = 0;
				}			
				break;
			}
			case BUTTON_ACTIVE:
			{
				if(!getButton)
				{
					if(++cnt_return_free == 100)
					{
						//button is action
						if(chute_status == CHUTE_OPEN || chute_status == CHUTE_CLOSE){
							button_Action = 1;
							flag_button_lcd = true;
						}
						else{
							button_Action = 0;
							if(chute_status == CHUTE_READY)
								flag_run = true;
						}

						cnt_debug_bt++;		// use for debug
						cnt_flag_display_lcd = vCycleLCD - 20;
					
						button_status = BUTTON_WAIT_NEXT_PUSH;
						cnt_return_free = 0;
					}			
				}
				else{
					cnt_return_free = 0;
				}
				
				break;
			}
			case BUTTON_WAIT_NEXT_PUSH:
			{
				if(++cnt_wait_next_push >= 3000)
				{
					button_status = BUTTON_FREE;
					cnt_wait_next_push = 0;
				}
				break;
			}	
		}
		
		//-----------------------Soft Reset-----------------------------------------------
			// Press and hold in 5 second to soft reset Chute 
			if(getButton)
			{
				if(++cnt_pushed_reset >= 5000)
					NVIC_SystemReset();			
			}
			else
			{
				cnt_pushed_reset = 0;
			}
	}
	

	
/*================================================================================*/
/*												CHECK CONNECTION WITH Master		 	      								*/
/*================================================================================*/
	void Check_Connection_Master(void)
	{
		if(cnt_ping < vTimeCheckCon*1000 && chute_status != NOT_CONNECT) 
			cnt_ping++;
		
 		if(cnt_ping == vTimeCheckCon*1000)		
		{
			chute_status = NOT_CONNECT;
			cnt_ping = vTimeCheckCon*1000 + 1;
		}
	}
	
/*================================================================================*/
/*												CHECK UPDATE ADDRESS CHUTE		 	      									*/
/*================================================================================*/
	void Check_Update_NEW_Infor(void)
	{			
		if(Update_NEW_Infor() == UPDATE_SUCCESS)
		{			
			vRS485.txBuff_config[0] = 0xFD;
			vRS485.txBuff_config[1] = vRS485.Sequence_number;
			vRS485.txBuff_config[RS485_NUM_BYTE-1] = Cal_CheckSum(vRS485.txBuff_config, RS485_NUM_BYTE);

			vRS485.TxFlag_config = 1;
			HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET);
			start_wait_send_master = getMicroSecond();
		}
		else
		{		
			/*Clear Flash*/
			Flash_Erase(ADDRES_DATA_STORAGE);
		}	
	}
	
/*========================================================================================*\
 *																	MAIN PROCESS																					*
\*========================================================================================*/	
	void Process_Chute(void)
	{		
		switch(chute_status)
		{
		//-----------------------------------------------------------	
			case CHUTE_READY:
			{				
				/*Not display LCD periodically*/
				cnt_flag_display_lcd = 0;	
				flag_newPacket_lcd = 0;		
				
				/*First connect with master*/
				if(flag_run)
				{
					cnt_flag_display_lcd = vCycleLCD-50;
					chute_status = CHUTE_OPEN;
					cnt_ping = 0;
				}			
				break;
			}
		/*---------------------------------------------------------------*/
			case CHUTE_OPEN:
			{				
				/*Get sensor*/
				Get_Sensor();
				
				/*Handle LED_BAO*/
				if(chute_isFull == 1)
				{
					/*Blink Led Chute when chute full*/
					if(Check_Even_Number(count_synchronize/300)){
						ON_DEN_BAO();
					}
					else{
						OFF_DEN_BAO();
					}
				}
				else
				{
					OFF_DEN_BAO();
				}
				break;
			}
		/*---------------------------------------------------------------*/
			case CHUTE_CLOSE:
			{				
				/*Turn On LED_BAO*/
				ON_DEN_BAO();
				
				break;
			}

		/*---------------------------------------------------------------*/
			case NOT_CONNECT:
			{										
				/*Turn Off LED_BAO*/
				OFF_DEN_BAO();
				
				/*Reconnect Master*/
				if(cnt_ping < vTimeCheckCon*1000)	
				{
					cnt_flag_display_lcd = 0;
					chute_status = CHUTE_OPEN;
				}		
				
				break;
			}
		/*---------------------------------------------------------------*/
			case CHUTE_CONFIG:
			{
				/*Turn On LED_BAO*/
				ON_DEN_BAO();
				HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET);
				
				/*Stop interrupt timer to update Flash Memory*/
				HAL_TIM_Base_Stop_IT(&htim1);
				HAL_TIM_Base_Stop_IT(&htim2);
				
				/*Update Flash Memory*/
				Check_Update_NEW_Infor();
				
				HAL_Delay(1);
				
				HAL_UART_Transmit(vRS485.huart, vRS485.txBuff_config, RS485_NUM_BYTE, HAL_MAX_DELAY);
				
				OFF_DEN_BAO(); 	HAL_Delay(50);
				ON_DEN_BAO(); 	HAL_Delay(50);
				OFF_DEN_BAO(); 	HAL_Delay(50);
				ON_DEN_BAO(); 	HAL_Delay(50);
				OFF_DEN_BAO(); 	HAL_Delay(50);
				ON_DEN_BAO(); 	HAL_Delay(50);
			
				
				/*Soft reset MCU*/
				NVIC_SystemReset();
				
				break;
			}
	
		}

	}
	
	
/*========================================================================================*\
 *DEBUG FUNCTION                                             *
\*========================================================================================*/	
	uint16_t cnt_send_debug = 0;
	
	void Debug_func(void) 
	{  
		if(cnt_send_debug >= 1000)
		{
			memset(Debug_arr, 0x00, 30);
			sprintf((char*)Debug_arr, "%d, %d, %d", vRS485.Sequence_number, 
																							vRS485.cnt_mess_received, 
																							vRS485.cnt_mess_wrong);
			
			HAL_UART_Transmit_DMA(&huart3, Debug_arr, sizeof(Debug_arr));
		
			cnt_send_debug = 0;
		}
	}
	
/*========================================================================================*\
 *													CALLBACK FUNCTION                                             *
\*========================================================================================*/	
	
/*______________________________________________________________________________*/
	
	void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) 
	{  
		/*RS485*/
		if(huart->Instance == USART1)			
		{    
			RS485_Read(&vRS485);
			RS485_Rx_Handle(&vRS485);
		}
	}

/*______________________________________________________________________________*/	
	
	void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
	{
		UNUSED(htim);
		
		if (htim->Instance == htim2.Instance) 	// every 1ms		
		{
			/*Increase cnt variable*/
			count_synchronize++;
			tim_tick_1ms++;
			cnt_flag_led_DB++;
			cnt_flag_display_lcd++; 
			cnt_send_debug++;
			
			/*On Flag*/
			flag_sensor_1ms = true;
			
			/*Handle function*/
			Flash_Led();
			Check_Connection_Master();
			Get_Button();
		}	
	}	
	
/*================================================================================*/
/*												CHECK CONNECTION WITH Master		 	      								*/
/*================================================================================*/
 	void Calib__mode(void)
	{
		uint16_t cnt_mode_calib = 0;
		uint8_t get_ss = 0;
		uint8_t get_bt = 0;
		bool calib_mode = false;
		uint16_t cnt_bt_on;
		
		get_bt = !HAL_GPIO_ReadPin(BUTTON_DET_GPIO_Port, BUTTON_DET_Pin);
		
		while(get_bt)
		{
			get_bt = !HAL_GPIO_ReadPin(BUTTON_DET_GPIO_Port, BUTTON_DET_Pin);
			if(++cnt_mode_calib == 2000)
			{
				calib_mode = true;
				break;
			}
			HAL_Delay(1);
		}
		
		if(calib_mode)
		{
			ST7920_GraphicMode(0);
			HAL_Delay(1);
			ST7920_Clear();
			
			sprintf(ROW0, "Calib mode");
			ST7920_SendString(0, 0, ROW0);
			sprintf(ROW1, "                ");
			ST7920_SendString(1, 0, ROW1);
			sprintf(ROW2, "                ");
			ST7920_SendString(2, 0, ROW2);
			sprintf(ROW3, "                ");
			ST7920_SendString(3, 0, ROW3);
		}
		
		while(calib_mode)
		{
			get_bt = !HAL_GPIO_ReadPin(BUTTON_DET_GPIO_Port, BUTTON_DET_Pin);
			get_ss = !HAL_GPIO_ReadPin(SENSOR_DET_GPIO_Port, SENSOR_DET_Pin);
		
			if(get_bt == 1 || get_ss == 1)
			{
				ON_DEN_BAO();
			}
			else{
				OFF_DEN_BAO();
			}
		
			if(get_bt == 1)
			{
				cnt_bt_on++;
			}
			else
				cnt_bt_on = 0;
			
			if(cnt_bt_on == 2000)
			{
				OFF_DEN_BAO();
				break;
			}
			
			HAL_Delay(1);
		}
		
	}

	

/**********************************************************************************************************************************/
/*******END PAGE********/



