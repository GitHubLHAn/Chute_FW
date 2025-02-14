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
	#define RS485_WAIT_STATE 						0
	#define RS485_READING_STATE 				1
	#define RS485_READING_CONFIG_STATE 	2

	#define RS485_NUM_BYTE  				3
	#define RS485_NUM_BYTE_CONFIG 	15

	#define RS485_MESS_OK		 2
	#define RS485_WRONG	 		 1

	#define RS485_CHECK_ID_MASK		0x3F
	
	#define RS485_CMD_MASK					0xC0
	#define RS485_CONFIG_CMD				0x80		// 10 000000
	#define RS485_OPEN_CHUTE_CMD		0x00		// 00 000000
	#define RS485_CLOSE_CHUTE_CMD		0x40		// 01 000000

/************************************************************************************/
/*DECLARE STRUCT*/
	typedef struct
	{
		UART_HandleTypeDef *huart;
		
		uint8_t rxBuff[RS485_NUM_BYTE];
		uint8_t txBuff[RS485_NUM_BYTE];
		uint8_t RxFlag, TxFlag;
		uint8_t STATE;
		uint8_t rxByte, rxPointer;
		uint8_t type_cmd;											
		uint8_t CS_byte;	
		
		uint16_t cnt_mess_sent;				
		uint16_t cnt_mess_received;		
		uint16_t cnt_mess_wrong;		
		
		uint8_t Sequence_number;				// So thu tu mang trong line (0->63)

		/*Use for config chute address*/
		uint8_t rxBuff_config[RS485_NUM_BYTE_CONFIG];
		uint8_t txBuff_config[RS485_NUM_BYTE];
		uint8_t RxFlag_config, TxFlag_config;
		
	}rs485_t;

	
/*DECLARE FUNCTION*/
//	uint8_t RS485_Read(rs485_t *pRx);
//	
//	void RS485_Init(rs485_t *pRS, UART_HandleTypeDef *huart);
//		
//	uint8_t RS485_Rx_Handle(rs485_t *pRS);
//	
//	uint8_t RS485_Transmit(rs485_t *pRS);

/*EXTERN*/
extern rs485_t vRS485;	

/************************************************************************************/
#endif /* RS485_H_*/
	



	