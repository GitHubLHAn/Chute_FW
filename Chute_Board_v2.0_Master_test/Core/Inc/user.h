/*
 * user.h
 * Created on: 18-July-2024
 * Author: Le Huu An
 * Used for chute yody
 */

#ifndef USER_H_
#define USER_H_

#include "main.h"
#include "stdbool.h"

/*DEFINE*/
	#define		CHUTE_READY 		0x0E
	#define		CHUTE_NORMAL 		0x00			// ~ chute open
	#define		CHUTE_FULL 		 	0x01
	#define		CHUTE_CLOSE 		0x02
	#define		NOT_CONNECT 		0xEE
	#define		CHUTE_CONFIG 		0xCF

	#define		ON 		true
	#define		OFF 	false
		
	#define LCD_SIZE_ROW		16

	#define CYCLE_LCD		1000 		// mili second
	#define TIME_CHECK_CONNECTION		60 			// second
	
	#define ON_LED_RED( )			HAL_GPIO_WritePin(THAPLED_REG_GPIO_Port, THAPLED_REG_Pin, GPIO_PIN_RESET)
	#define OFF_LED_RED( )		HAL_GPIO_WritePin(THAPLED_REG_GPIO_Port, THAPLED_REG_Pin, GPIO_PIN_SET)
	
	#define ON_LED_GREEN( )			HAL_GPIO_WritePin(THAPLED_GREEN_GPIO_Port, THAPLED_GREEN_Pin, GPIO_PIN_RESET)
	#define OFF_LED_GREEN( )		HAL_GPIO_WritePin(THAPLED_GREEN_GPIO_Port, THAPLED_GREEN_Pin, GPIO_PIN_SET)
	
	#define ON_LED_YELLOW( )		HAL_GPIO_WritePin(THAPLED_YELLOW_GPIO_Port, THAPLED_YELLOW_Pin, GPIO_PIN_RESET)
	#define OFF_LED_YELLOW( )		HAL_GPIO_WritePin(THAPLED_YELLOW_GPIO_Port, THAPLED_YELLOW_Pin, GPIO_PIN_SET)
	
	#define ON_SPEAKER( ) 		HAL_GPIO_WritePin(THAPCOI_GPIO_Port, THAPCOI_Pin, GPIO_PIN_RESET)
	#define OFF_SPEAKER( ) 		HAL_GPIO_WritePin(THAPCOI_GPIO_Port, THAPCOI_Pin, GPIO_PIN_SET)
	
	#define ON_LED_BAO( )		HAL_GPIO_WritePin(LED_BAO_GPIO_Port,LED_BAO_Pin,GPIO_PIN_SET);
	#define OFF_LED_BAO( )		HAL_GPIO_WritePin(LED_BAO_GPIO_Port,LED_BAO_Pin,GPIO_PIN_RESET);

/************************************************************************************/
/*DECLARE STRUCT*/


/*DECLARE FUNCTION*/
	void BlinkLed_Debug(void);
	void Handle_Thap_Led(void);
	void Display_LCD(uint16_t cycle);
	void Handle_Thap_Coi(void);
	void Handle_Thap_Led(void);	

	void Process_Chute(void);

	void Printf_ROW_LCD(uint8_t row, const char *str);
	
	uint8_t Cal_CheckSum(uint8_t *Buff, uint8_t length);
	
	void Check_Connection_App(void);
	void Check_Update_Addr_Chute(void);

/*EXTERN*/
	extern volatile bool flag_flashLed;
	
	extern bool flag_connected_app;	
	
	extern volatile uint16_t cnt_ping;
		
	extern uint16_t count_packet;
	
	extern uint8_t chute_status;
	
	extern uint8_t chute_isFull;
	
	extern uint8_t status_thap_coi;	
	
	extern uint8_t status_thap_led;
	
	extern uint8_t button_Action;
	
	extern uint8_t 	ROW0[LCD_SIZE_ROW], 
									ROW1[LCD_SIZE_ROW], 
									ROW2[LCD_SIZE_ROW], 
									ROW3[LCD_SIZE_ROW];
	
	extern bool flag_update_address;
	extern uint8_t Chute_Addr_Arr[16];
	extern uint8_t Chute_Infor_update_arr[12];

/************************************************************************************/
#endif /* USER_H_*/


