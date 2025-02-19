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
#include "flash_address.h"

/*DEFINE*/
	#define		CHUTE_READY 		0xF0				// no mean
	#define		CHUTE_OPEN			0x00				// 00 000000
	#define		CHUTE_CLOSE 		0x40				// 01 000000
	#define		NOT_CONNECT 		0xFE				// no mean
	#define		CHUTE_CONFIG		0x80				// 10 000000
	
	#define		BUTTON_FREE							0				// 		button is free
	#define		BUTTON_ACTIVE						1				// 		button is pushed and waiting deactive
	#define		BUTTON_WAIT_NEXT_PUSH		2				// 		button is action and waiting the next push

	#define		ON 		true
	#define		OFF 	false
		
	#define LCD_SIZE_ROW		16

	#define CYCLE_LCD_50ms			50 
	#define CYCLE_LCD_500ms			500 		
	#define CYCLE_LCD_1000ms		1000 
	#define CYCLE_LCD_5000ms		500 	
	
	#define TIME_CHECK_CONNECTION_15s		15 			// second
	#define TIME_CHECK_CONNECTION_30s		30 			// second
	#define TIME_CHECK_CONNECTION_45s		45 			// second
	#define TIME_CHECK_CONNECTION_60s		60 			// second
	
	
	#define ON_DEN_BAO( )			HAL_GPIO_WritePin(LED_BAO_CTL_GPIO_Port,LED_BAO_CTL_Pin,GPIO_PIN_SET);
	#define OFF_DEN_BAO( )		HAL_GPIO_WritePin(LED_BAO_CTL_GPIO_Port,LED_BAO_CTL_Pin,GPIO_PIN_RESET);

/************************************************************************************/
/*DECLARE STRUCT*/


/*DECLARE FUNCTION*/
	void BlinkLed_Debug(void);
	
	uint32_t getMicroSecond(void);
	
	void Handle_Thap_Led(void);
	
	void Display_LCD(uint16_t cycle);
	
	void Process_Chute(void);
	
	uint8_t Cal_CheckSum(uint8_t *Buff, uint8_t length);
	
	void Check_Connection_Master(void);
	
	void Check_Update_Addr_Chute(void);

/*EXTERN*/
	extern volatile uint32_t count_synchronize;

	extern volatile bool flag_flashLed_trx;
	
	extern volatile uint16_t cnt_ping;
	
	extern bool flag_run;
		
	extern volatile uint8_t qty_packet;
	
	extern volatile uint8_t chute_status;
	
	extern volatile uint8_t chute_isFull;
	
	extern uint8_t button_Action;
	
	extern char	ROW0[LCD_SIZE_ROW], 
									ROW1[LCD_SIZE_ROW], 
									ROW2[LCD_SIZE_ROW], 
									ROW3[LCD_SIZE_ROW];
	
	extern uint8_t Chute_Addr_Arr[16];
	
	extern uint16_t vCycleLCD;					// time refresh LCD
	
	extern uint16_t vTimeCheckCon;	
	
	void Calib__mode(void);

/************************************************************************************/
#endif /* USER_H_*/


