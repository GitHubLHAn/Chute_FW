/*
 * user.c
 * Created on: 18-DEC-2024
 * Author: Le Huu An (anlh55@viettel.com.vn)
 * This version of chute firmware is upgraded with new system of
 * chute with 2 device (master and slave, 1 master manages 32 slave).
 */
 
 
#include "user.h"

#include "st7920.h"
#include "rs485.h"
#include "flash_address.h"

#include <string.h>
#include <stdio.h>

/*
NOTE: 


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
	
	uint8_t ROW0[LCD_SIZE_ROW] = {0};
	uint8_t ROW1[LCD_SIZE_ROW] = {0};
	uint8_t ROW2[LCD_SIZE_ROW] = {0};
	uint8_t ROW3[LCD_SIZE_ROW] = {0};
	
	volatile uint8_t qty_packet = 0;
	
	volatile uint16_t cnt_flag_led_DB = 0;
	volatile bool flag_led_DB = 0;
	
	volatile bool flag_sensor_1ms = false;
	volatile bool flag_button_1ms = false;
	
	uint8_t status_sensor = 0;
	volatile uint16_t cnt_sensor_on = 0;

	volatile uint16_t cnt_flag_display_lcd = 0;
	bool flag_button_lcd = false;
	bool flag_newPacket_lcd = false;
	
	uint8_t button_Action = 0x00;

	bool flag_run = false;			// start running variable
	volatile uint16_t cnt_ping = 0; //TIME_CHECK_CONNECTION*1000 + 1;
	
	uint8_t Chute_Addr_Arr[16] = {'D','C',':',' ',0,0,0,0,0,0,0,0,0,0,0,0};
	uint8_t Chute_Infor_update_arr[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
	
	volatile uint32_t _s = 0, _i = 0;
	

	
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
		if(cnt_flag_led_DB >= 1000)
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
				case CHUTE_OPEN:
				{
					if(qty_packet == 0xFF){
						sprintf((char*)ROW1, "So luong: *** ");
					}
					else
						sprintf((char*)ROW1, "So luong: %d ", qty_packet);
					
					/*Chute Status*/
					if(chute_isFull == 1)
						sprintf((char*)ROW2, "Mang day");
					else
						sprintf((char*)ROW2, "Mang Mo");
					
					if(flag_button_lcd)
					{
						sprintf((char*)ROW3, "Nhan nut Dong");
						flag_button_lcd = false;
					}
					else
					break;
				}
			//-----------------------------------------------------------	
				case CHUTE_CLOSE:
				{
					if(qty_packet == 0xFF){
						sprintf((char*)ROW1, "So luong: *** ");
					}
					else
						sprintf((char*)ROW1, "So luong: %d ", qty_packet);
					
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
			
			flag_button_lcd = false;
			cnt_flag_display_lcd = 0;
		}
		else if(flag_newPacket_lcd && cnt_flag_display_lcd >= cycle/3)
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
		
			cnt_flag_display_lcd = cycle/3;
		}				
	}
	
/*================================================================================*/
/*															GET BUTTON																				*/
/*================================================================================*/
	uint8_t getButton = 0;
	bool isButton = false;
	uint16_t cnt_Button_on = 0, cnt_isButton, cnt_free1;
	
	bool isPressed_Reset = false;
	uint16_t cnt_Button_on_Reset = 0, cnt_isButton_Reset = 0;
	uint8_t cnt_Reset = 0;
	
	uint16_t cnt_time_reset;
	
	void Get_Button(void)
	{
		getButton = !HAL_GPIO_ReadPin(BUTTON_DET_GPIO_Port, BUTTON_DET_Pin);
//		if(flag_button_1ms)
//		{
		//----------------------Press Button------------------------------------------------
		// Press and hold Button in 1 second for Press Fucntion
			if(getButton && isButton == false)
			{	
				if(cnt_Button_on == 750)
				{
					isButton = true;		
					cnt_Reset = 0;
					
//					if(chute_status == CHUTE_OPEN){
//						button_Action = 0x01;
//						flag_button_lcd = true;
//					}
//					else if(chute_status == CHUTE_CLOSE){
//						button_Action = 0x02;
//						flag_button_lcd = true;
//					}
//					else if(chute_status == CHUTE_READY)
//						chute_status = CHUTE_OPEN;
					
					if(chute_status == CHUTE_READY)
						flag_run = true;
					else if(chute_status == CHUTE_OPEN || chute_status == CHUTE_CLOSE)
						button_Action = 1;
					else
						button_Action = 0;
					flag_button_lcd = true;
					
					cnt_free1++;		// use for debug
					cnt_flag_display_lcd = CYCLE_LCD - 10;
				}
				else{
					cnt_Button_on++;
				}
			}else{
				cnt_Button_on = 0;
			}
			
			// Wait 5s for the next push 
			if(isButton == true)
			{
				cnt_Reset = 0;
				if(++cnt_isButton >= 5000){ 
					isButton = false;
					cnt_isButton = 0;
					button_Action = 0;
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
				if(++cnt_isButton_Reset >= 750){
						isPressed_Reset = false;
						cnt_isButton_Reset = 0;
					}
			}
			
			if(cnt_Reset >= 3){			// Press 3 times to soft reset
				NVIC_SystemReset();
			}
			
			/*Press < 5 times, then 5s without press, soft-reset-function will be deleted*/
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
//			flag_button_1ms = false;
//		}
	}
	
/*================================================================================*/
/*												CHECK CONNECTION WITH Master		 	      								*/
/*==================0==============================================================*/
	void Check_Connection_Master(void)
	{
		if(cnt_ping < TIME_CHECK_CONNECTION*1000 && chute_status != NOT_CONNECT) 
			cnt_ping++;
		
 		if(cnt_ping == TIME_CHECK_CONNECTION*1000)		
		{
			chute_status = NOT_CONNECT;
			cnt_ping = TIME_CHECK_CONNECTION*1000 + 1;
		}
	}
	
/*================================================================================*/
/*												CHECK UPDATE ADDRESS CHUTE		 	      									*/
/*================================================================================*/
	void Check_Update_Addr_Chute(void)
	{			
		if(Update_Addr_Chute() == UPDATE_SUCCESS)
		{			
			vRS485.txBuff_config[0] = vRS485.rxBuff_config[0];
			vRS485.txBuff_config[1] = 1;
			vRS485.txBuff_config[RS485_NUM_BYTE-1] = Cal_CheckSum(vRS485.txBuff_config, RS485_NUM_BYTE);

			vRS485.TxFlag_config = 1;
			start_wait_send_master = getMicroSecond();
		}
		else
		{
			vRS485.txBuff_config[0] = vRS485.rxBuff_config[0];
			vRS485.txBuff_config[1] = 0;
			vRS485.txBuff_config[RS485_NUM_BYTE-1] = Cal_CheckSum(vRS485.txBuff_config, RS485_NUM_BYTE);
			
			/*Clear Flash*/
			Flash_Erase(ADDRES_DATA_STORAGE);
			 
			vRS485.TxFlag_config = 1;
			start_wait_send_master = getMicroSecond();
		}	
	}
	
/*========================================================================================*\
 *																	MAIN PROCESS																					*
\*========================================================================================*/	
	void Process_Chute(void)
	{
		/*Get and handle button*/
		//Get_Button();
		
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
					cnt_flag_display_lcd = CYCLE_LCD-10;
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
				if(cnt_ping < TIME_CHECK_CONNECTION*1000)	
				{
					cnt_flag_display_lcd = CYCLE_LCD-10;
					chute_status = CHUTE_OPEN;
				}		
				
				break;
			}
		/*---------------------------------------------------------------*/
			case CHUTE_CONFIG:
			{
				/*Turn On LED_BAO*/
				ON_DEN_BAO();
				
				/*Stop interrupt timer to update Flash Memory*/
				HAL_TIM_Base_Stop_IT(&htim1);
				HAL_TIM_Base_Stop_IT(&htim2);
				
				/*Update Flash Memory*/
				Check_Update_Addr_Chute();
				
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
				
				/*On Flag*/
				flag_sensor_1ms = true;
				flag_button_1ms = true;
				
				/*Handle function*/
				Flash_Led();
				Check_Connection_Master();
				Get_Button();
			}	

		}	

/**********************************************************************************************************************************/
/*******END PAGE********/



