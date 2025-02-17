/*
 * user.h
 * Created on: 18-DEC-2025
 * Author: Le Huu An (anlh55@viettel.com.vn)
 * This version of chute firmware is upgraded with new system of
 * chute with 2 device (master and slave, 1 master manages 32 slave).
 */

#ifndef USER_H_
#define USER_H_

#include "main.h"
#include "stdbool.h"

#include "slave.h"

/*DEFINE*/

	#define		CHUTE_OPEN			0x00				// 0000 0000
	#define		CHUTE_CLOSE 		0x40				// 0100 0000
	#define		CHUTE_CONFIG		0x80

	#define		ON 		true
	#define		OFF 	false


	#define CYCLE_SEND_SLAVE		10		// mili second
	
	#define TIME_CHECK_CONNECT_SL	30		// second, time check connection slave
	
	
	/*Porting Lamp*/
	#define ON_LAMP_RED( )			HAL_GPIO_WritePin(LAMP_RED_GPIO_Port, LAMP_RED_Pin, GPIO_PIN_SET);
	#define OFF_LAMP_RED( )			HAL_GPIO_WritePin(LAMP_RED_GPIO_Port, LAMP_RED_Pin, GPIO_PIN_RESET);
	
	#define ON_LAMP_YELLOW( )		HAL_GPIO_WritePin(LAMP_YELLOW_GPIO_Port, LAMP_YELLOW_Pin, GPIO_PIN_SET);
	#define OFF_LAMP_YELLOW( )	HAL_GPIO_WritePin(LAMP_YELLOW_GPIO_Port, LAMP_YELLOW_Pin, GPIO_PIN_RESET);
	
	#define ON_LAMP_GREEN( )		HAL_GPIO_WritePin(LAMP_GREEN_GPIO_Port, LAMP_GREEN_Pin, GPIO_PIN_SET);
	#define OFF_LAMP_GREEN( )		HAL_GPIO_WritePin(LAMP_GREEN_GPIO_Port, LAMP_GREEN_Pin, GPIO_PIN_RESET);
	
	/*Porting Speaker*/
	#define ON_SPEAKER( )				HAL_GPIO_WritePin(SPEAKER_GPIO_Port, SPEAKER_Pin, GPIO_PIN_SET);
	#define OFF_SPEAKER( )			HAL_GPIO_WritePin(SPEAKER_GPIO_Port, SPEAKER_Pin, GPIO_PIN_RESET);
	
	/*Porting LED*/
	#define ON_LED_DEBUG( )			HAL_GPIO_WritePin(LED_DEBUG_GPIO_Port, LED_DEBUG_Pin, GPIO_PIN_SET);
	#define OFF_LED_DEBUG( )		HAL_GPIO_WritePin(LED_DEBUG_GPIO_Port, LED_DEBUG_Pin, GPIO_PIN_RESET);
	
	#define ON_LED_APP( )			HAL_GPIO_WritePin(LED_TRX_APP_GPIO_Port, LED_TRX_APP_Pin, GPIO_PIN_SET);
	#define OFF_LED_APP( )		HAL_GPIO_WritePin(LED_TRX_APP_GPIO_Port, LED_TRX_APP_Pin, GPIO_PIN_RESET);
	
	#define ON_LED_TRX_A( )			HAL_GPIO_WritePin(LED_TRX_SLAVEA_GPIO_Port, LED_TRX_SLAVEA_Pin, GPIO_PIN_SET);
	#define OFF_LED_TRX_A( )		HAL_GPIO_WritePin(LED_TRX_SLAVEA_GPIO_Port, LED_TRX_SLAVEA_Pin, GPIO_PIN_RESET);
	
	#define ON_LED_TRX_B( )			HAL_GPIO_WritePin(LED_TRX_SLAVEB_GPIO_Port, LED_TRX_SLAVEB_Pin, GPIO_PIN_SET);
	#define OFF_LED_TRX_B( )		HAL_GPIO_WritePin(LED_TRX_SLAVEB_GPIO_Port, LED_TRX_SLAVEB_Pin, GPIO_PIN_RESET);
	

/************************************************************************************/
/*DECLARE STRUCT*/


/*DECLARE FUNCTION*/
	void BlinkLed_Debug(void);

	uint8_t Cal_CheckSum(uint8_t *Buff, uint8_t length);
	
	void Master_Init_Func(void);
	
	uint8_t Send_Slave_Period(void);
	
	uint32_t getMicroSecond(void);
	
	void Master_Process(void);
	
	void Debug_Function(void);
	
	void Handle_Lamp(void);
	
/*EXTERN*/

	extern UART_HandleTypeDef *pSlavePort;
	
	extern uint8_t ID_Master;
	
	extern volatile uint16_t cnt_debug;
	
	extern volatile uint32_t start_time, interval;
	
	extern volatile bool flag_flashLed_debug,
								flag_flashLed_app, 
								flag_flashLed_trxA, 
								flag_flashLed_trxB;
	
	extern uint8_t Full_check_arr[NUMBER_SLAVE];
	


/************************************************************************************/
#endif /* USER_H_*/


