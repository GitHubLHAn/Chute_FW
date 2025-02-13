/*
 * uart.h version 2
 * Created on: 4-Sep-2024
 * Author: Le Huu An
 */

#ifndef UART_H_
#define UART_H_

#include "main.h"

#include <stdbool.h>


/*DEFINE*/
#define USART_RX_END 2
#define USART_READING 1
#define USART_WAIT 0

#define UART_NUM_BYTE 12


/************************************************************************************/
/*DECLARE STRUCT*/
	typedef struct
	{
		uint8_t rxBuff[UART_NUM_BYTE];
		uint8_t txBuff[UART_NUM_BYTE];
		uint8_t RxFlag;
		uint8_t STATE;
		uint8_t rxByte, rxPointer;
		
		uint8_t header, ender;					// header: 0xFE, ender: 0xFF						
		uint8_t cmd_type;							// loai lenh
		uint16_t cnt_mess_sent;				// dem so ban tin da gui
		uint16_t cnt_mess_received;		// dem so ban tin nhan duoc
		uint16_t cnt_mess_wrong;		// dem so ban tin nhan duoc
			
		UART_HandleTypeDef *huart;		
	}uart_t;

	
/*DECLARE FUNCTION*/
	void UART_Read(uart_t *pRx);
	void UART_Init(uart_t *pRS, UART_HandleTypeDef *huart);
	
	uint8_t Uart_Rx_Handle(uart_t *pRS);

/*EXTERN*/
extern uart_t vUART_DB;	

/************************************************************************************/
#endif /* UART_H_*/
	
