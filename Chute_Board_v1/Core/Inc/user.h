/*
 * user.h
 * Created on: 18-July-2024
 * Author: Le Huu An
 */

#ifndef USER_H_
#define USER_H_

#include "main.h"
/*Include the type of stm32*/
#include "stm32f1xx_hal.h"

#include "stdbool.h"

#define		CHUTE_NORMAL 					0x00
#define		CHUTE_CHOT 		 				0x01
#define		CHUTE_FULL_NOT_CHOT 	0x02

#define		ON 		true
#define		OFF 	false
	
#define MASK_RED			0x04
#define MASK_YELLOW		0x02
#define MASK_GREEN		0x01

#define LCD_SIZE_ROW		16

#define CYCLE_LCD		1000 

/*DEFINE*/

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

/*EXTERN*/
	extern volatile bool flag_flashLed;
	extern uint16_t count_packet;
	extern uint8_t chute_status;
	extern bool chute_isFull;
	
	extern uint8_t status_thap_coi;	
	extern uint8_t status_thap_led;
	extern uint8_t button_Action;
	
	extern uint8_t ROW0[LCD_SIZE_ROW], ROW1[LCD_SIZE_ROW], ROW2[LCD_SIZE_ROW], ROW3[LCD_SIZE_ROW];

/************************************************************************************/
#endif /* USER_H_*/


