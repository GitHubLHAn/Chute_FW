#include <user.h>
#include <string.h>
#include "st7920.h"
#include "rs485.h"
#include <stdio.h>

/*
 * user.h
 * Created on: 18-July-2024
 * Author: Le Huu An
 */
 
/*
NOTE: 


*/
/**********************************************************************************************************************************/
/******EXTERN*******/
extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim2;

/**********************************************************************************************************************************/
/*******DECLARE VARIABLE********/
	bool chute_isFull = false;
	uint8_t chute_status = CHUTE_NORMAL;
	
	uint8_t ROW0[LCD_SIZE_ROW] = {0};
	uint8_t ROW1[LCD_SIZE_ROW] = {0};
	uint8_t ROW2[LCD_SIZE_ROW] = {0};
	uint8_t ROW3[LCD_SIZE_ROW] = {0};
	
	uint16_t count_packet = 0;
	
	volatile uint16_t cnt_flag_led = 0;
	volatile uint16_t flag_led = 0;
	volatile uint16_t cnt_flag_display_lcd = 0;
	volatile uint16_t flag_sensor_1ms = 0;
	volatile uint16_t flag_button_1ms = 0;
	volatile uint16_t cnt_flag_thapLed_Red = 0;
	volatile uint16_t cnt_flag_thapLed_Yelow = 0;
	volatile uint16_t cnt_flag_led_chute = 0;
	
	uint8_t status_sensor = 0;
	uint16_t cnt_sensor_on = 0;
	
	bool flag_inform = false;
	bool flag_display_lcd = false;
	
	uint8_t status_thap_coi = 0x00;	
	uint8_t status_thap_led = 0x00;
	
	uint8_t getButton = 0;
	uint16_t cnt_button_on = 0;
	uint8_t button_Action = 0x00;

/**********************************************************************************************************************************/
/*******FUNCTION********/

/*================================================================================*/
/*																																								*/
/*================================================================================*/
	volatile uint32_t tim_tick_1ms = 0;
	
	uint32_t getMicroSecond(void){
		return tim_tick_1ms*1000 + __HAL_TIM_GetCounter(&htim2);
	}
	
/*================================================================================*/
/*																																								*/
/*================================================================================*/
	volatile uint32_t cnt_flag_flashLed = 0;
	volatile bool flag_flashLed = 0;
	
	void Flash_Led(void){
		if(flag_flashLed){
			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
			if(++cnt_flag_flashLed >= 10){
				HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
				cnt_flag_flashLed = 0;
				flag_flashLed = 0;
			}
		}
	}


	
/*================================================================================*/
/*																																								*/
/*================================================================================*/
	void BlinkLed_Debug(void){
//		if(cnt_flag_led >= 100){
//			HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
//			cnt_flag_led = 0;	
//		}
	}

/*================================================================================*/
/*																																								*/
/*================================================================================*/	
	uint8_t Get_Sensor(void){	
		status_sensor = HAL_GPIO_ReadPin(SENSOR_GPIO_Port, SENSOR_Pin);
		if(flag_sensor_1ms){
			if(status_sensor == 1){
				if(cnt_sensor_on == 2000){
					chute_isFull = 1;
				}
				else{
					cnt_sensor_on++;
					if(cnt_sensor_on == 2)	flag_display_lcd = true;
				}
			}
			else{
				cnt_sensor_on = 0;
				chute_isFull = 0;
			}
			
			flag_sensor_1ms = 0;			
		}		
		return chute_isFull;
	}

/*================================================================================*/
/*																																								*/
/*================================================================================*/
	void Printf_ROW_LCD(uint8_t row, const char *str){
		if(row == 0){
			memset(ROW0, ' ', LCD_SIZE_ROW); 
			memcpy(ROW0, str, strlen(str));
		}
		else if(row == 1){
			memset(ROW1, ' ', LCD_SIZE_ROW);
			memcpy(ROW1, str, strlen(str));
		}
		else if(row == 2){
			memset(ROW2, ' ', LCD_SIZE_ROW);
			memcpy(ROW2, str, strlen(str));
		}
		else if(row == 3){
			memset(ROW3, ' ', LCD_SIZE_ROW);
			memcpy(ROW3, str, strlen(str));
		}
	}

/*================================================================================*/
/*																																								*/
/*================================================================================*/
	void Display_LCD(uint16_t cycle){
		char str[LCD_SIZE_ROW];
		
		if(cnt_flag_display_lcd >= cycle || flag_display_lcd == true){
			memset(ROW0, ' ', LCD_SIZE_ROW); 
			memset(ROW1, ' ', LCD_SIZE_ROW);
			memset(ROW2, ' ', LCD_SIZE_ROW);
			memset(ROW3, ' ', LCD_SIZE_ROW);
			
			ST7920_SendString(0, 0, (char*)ROW0);
			ST7920_SendString(1, 0, (char*)ROW1);
			ST7920_SendString(2, 0, (char*)ROW2);
			ST7920_SendString(3, 0, (char*)ROW3);
			
			if(flag_inform){
				
				flag_inform = 0;
			}
			else{
				sprintf((char*)ROW0, "ID: %d", vRS485.ID_Board);
				//Printf_ROW_LCD(0, str);
				
				sprintf((char*)ROW1, "So luong: %d ", count_packet);
				//sprintf((char*)ROW1, "So luong: %d ", cnt_flag_led);
				//Printf_ROW_LCD(1, str);
				
				switch(chute_status){
					case CHUTE_CHOT:
					{
						sprintf((char*)ROW2, "Dang Chot Tai");	
						//Printf_ROW_LCD(2, str);	
						break;
					}
					case CHUTE_FULL_NOT_CHOT:
					{
						sprintf((char*)ROW2, "Mang day");	
						//Printf_ROW_LCD(2, str);	
						sprintf((char*)ROW3, "Chua Chot Tai");	
						//Printf_ROW_LCD(3, str);	
						break;
					}
					default: break;
				}		
			}
				
			ST7920_SendString(3, 0, (char*)ROW3);
			ST7920_SendString(2, 0, (char*)ROW2);
			ST7920_SendString(1, 0, (char*)ROW1);
			ST7920_SendString(0, 0, (char*)ROW0);
			
			cnt_flag_display_lcd = 0;
			flag_display_lcd = false;
		}
	}
	
/*================================================================================*/
/*																																								*/
/*================================================================================*/	
	void Handle_Thap_Coi(void){
//		if(status_thap_coi == 0x01)		
//			HAL_GPIO_WritePin(THAPCOI_GPIO_Port, THAPCOI_Pin, GPIO_PIN_SET);
//		else													
//			HAL_GPIO_WritePin(THAPCOI_GPIO_Port, THAPCOI_Pin, GPIO_PIN_RESET);
	}

/*================================================================================*/
/*																																								*/
/*================================================================================*/
	void Handle_Thap_Led(void){	
		if(status_thap_led == 1){
			HAL_GPIO_WritePin(THAPLED3_GPIO_Port,THAPLED3_Pin,GPIO_PIN_SET);
			HAL_GPIO_WritePin(THAPLED2_GPIO_Port, THAPLED2_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(THAPLED1_GPIO_Port, THAPLED1_Pin, GPIO_PIN_RESET);
		}
		else if(status_thap_led == 2){
			HAL_GPIO_WritePin(THAPLED3_GPIO_Port,THAPLED3_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(THAPLED2_GPIO_Port, THAPLED2_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(THAPLED1_GPIO_Port, THAPLED1_Pin, GPIO_PIN_RESET);
		
		}
		else if(status_thap_led == 3){
			HAL_GPIO_WritePin(THAPLED3_GPIO_Port,THAPLED3_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(THAPLED2_GPIO_Port, THAPLED2_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(THAPLED1_GPIO_Port, THAPLED1_Pin, GPIO_PIN_SET);
		}
		else{
			HAL_GPIO_WritePin(THAPLED3_GPIO_Port,THAPLED3_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(THAPLED2_GPIO_Port, THAPLED2_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(THAPLED1_GPIO_Port, THAPLED1_Pin, GPIO_PIN_RESET);
		}
		
//		//-----------------------------------------------------
//		if((status_thap_led & MASK_RED) == MASK_RED){				// on led red
////			if(++cnt_flag_thapLed_Red >= 500){
////				HAL_GPIO_TogglePin(THAPLED1_GPIO_Port, THAPLED1_Pin);
////				cnt_flag_thapLed_Red = 0;
////			} 
//			HAL_GPIO_WritePin(THAPLED1_GPIO_Port, THAPLED1_Pin, GPIO_PIN_SET);
//		}else{
//			HAL_GPIO_WritePin(THAPLED1_GPIO_Port,THAPLED1_Pin, GPIO_PIN_RESET);
//		}
//		//-----------------------------------------------------
//		if((status_thap_led & MASK_YELLOW) == MASK_YELLOW){		// on led yellow
////			if(++cnt_flag_thapLed_Yelow >= 500){
////				HAL_GPIO_TogglePin(THAPLED2_GPIO_Port, THAPLED2_Pin);
////				cnt_flag_thapLed_Yelow = 0;
////			}
//			HAL_GPIO_WritePin(THAPLED2_GPIO_Port, THAPLED2_Pin, GPIO_PIN_RESET);
//		}else{
//			HAL_GPIO_WritePin(THAPLED2_GPIO_Port,THAPLED2_Pin,GPIO_PIN_RESET);
//		}
//		//-----------------------------------------------------
//		if((status_thap_led & MASK_GREEN) == MASK_GREEN){			// on led green
//			HAL_GPIO_WritePin(THAPLED3_GPIO_Port,THAPLED3_Pin,GPIO_PIN_SET);		
//		}else{
//			HAL_GPIO_WritePin(THAPLED3_GPIO_Port,THAPLED3_Pin,GPIO_PIN_RESET);
//		}
	}
	
/*================================================================================*/
/*																																								*/
/*================================================================================*/
	bool isButton = false;
	uint16_t cnt_Button_on = 0, cnt_isButton, cnt_free1;;
	
	void Get_Button(void){
		getButton = !HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin);
		if(flag_button_1ms){
			if(getButton && isButton == 0)
			{	
				if(cnt_Button_on == 100){
					isButton = 1;		
					//flag_inform = 1;
					if(chute_status == CHUTE_NORMAL || chute_status == CHUTE_FULL_NOT_CHOT){
						button_Action = 0x01;
					}
					else if(chute_status == CHUTE_CHOT){
						button_Action = 0x02;
					}
					
					cnt_free1++;
					
				}else{
					cnt_Button_on++;
				}
			}else{
				cnt_Button_on = 0;
			}
			
			if(isButton == 1){
				
				if(++cnt_isButton >= 10000){
					isButton = 0;
					cnt_isButton = 0;
					//button_Action = 0x00;
				}
			}
			
			flag_button_1ms = 0;
		}
	
	
	}

/******************************************************************************************\
 *																	MAIN PROCESS																					*
\******************************************************************************************/	
	void Process_Chute(void)
	{
		Get_Button();
		Handle_Thap_Led();
		Handle_Thap_Coi();
		
		switch(chute_status){
			/*---------------------------------------------------------------*/
			case CHUTE_NORMAL:
			{
				Get_Sensor();
				HAL_GPIO_WritePin(LED_BAO_GPIO_Port,LED_BAO_Pin,GPIO_PIN_RESET);
				break;
			}
			/*---------------------------------------------------------------*/
			case CHUTE_CHOT:
			{				
				HAL_GPIO_WritePin(LED_BAO_GPIO_Port,LED_BAO_Pin,GPIO_PIN_SET);
				break;
			}
			/*---------------------------------------------------------------*/
			case CHUTE_FULL_NOT_CHOT:
			{
				Get_Sensor();
				/*Blink Led Chute*/
				if(cnt_flag_led_chute >= 300){
					HAL_GPIO_TogglePin(LED_BAO_GPIO_Port, LED_BAO_Pin);
					cnt_flag_led_chute = 0;
				}		
			}
			
			default: break;
	
		}

	}
/*******************************************************************************************\
														CALLBACK FUNCTION
\*******************************************************************************************/	
	
/*______________________________________________________________________________*/
	void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {  
		if(huart->Instance == USART1){    
			RS485_Read(&vRS485);
			HAL_UART_Receive_IT(&huart1, &vRS485.rxByte, 1); 
			//num_irq++;		
			UartRx_Handle(&vRS485);
		}
	}

/*______________________________________________________________________________*/	
	void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
			UNUSED(htim);
			
			if (htim->Instance == htim2.Instance){ 	// every 1ms			
				/*Increase cnt variable*/
				cnt_flag_led++;
				cnt_flag_display_lcd++;
				cnt_flag_led_chute++;
				
				/*On Flag*/
				flag_sensor_1ms = 1;
				flag_button_1ms = 1;
				
				/*Handle function*/
				Flash_Led();
//				Handle_Thap_Led();
//				Handle_Thap_Coi();
				//UartTx_Transmit(&vRS485);
			}	

		}	

/*______________________________________________________________________________*/





