/*
 * app.h
 * Created on: 15-Jan-2025
 * Author: Le Huu An
 */

#ifndef APP_H_
#define APP_H_

#include "main.h"

#include <stdbool.h>


/*DEFINE*/
	
	#define NUM_BYTE_APP	3
	
	#define READ_HD_APP				0		// read ID
	#define READ_DT_APP				1		// read quantity
	#define READ_CS_APP				2		// read checksum
	
	
	#define MODE_SEND_APP(	)				HAL_GPIO_WritePin(RS485_APP_DE_GPIO_Port, RS485_APP_DE_Pin, GPIO_PIN_SET)
	#define MODE_RECEIVE_APP(		)		HAL_GPIO_WritePin(RS485_APP_DE_GPIO_Port, RS485_APP_DE_Pin, GPIO_PIN_RESET)


/************************************************************************************/
/*DECLARE STRUCT*/
	
	typedef struct
	{
		UART_HandleTypeDef *huart;
		
		uint8_t RxFlag;
		uint8_t STATE;
		uint8_t rxByte;		
		
		uint16_t cnt_mess_sent;				
		uint16_t cnt_mess_received;		
		uint16_t cnt_mess_wrong;		
		
	}app_com_t;		// application communication

	
/*DECLARE FUNCTION*/
	
	void App_Init_Func(app_com_t *pApp, UART_HandleTypeDef *huart);
	
	void Read_App(app_com_t *pAPP);

/*EXTERN*/
	
	extern app_com_t vApp;
	
	extern bool flag_wait_sending_slave;
	
	extern uint8_t TX_toApp[NUM_BYTE_APP];
	extern uint8_t RX_fromApp[NUM_BYTE_APP];

/************************************************************************************/
#endif /* RS485_H_*/
	



	