/*
 * rs485.h version 2
 * Created on: 25-June-2024
 * Author: Le Huu An
 */

#ifndef RS485_H_
#define RS485_H_

#include <main.h>

#include <stdbool.h>


/*DEFINE*/
#define RS485_WAIT 0
#define RS485_READING 1
#define RS485_READING_CONFIG 2

#define RS485_NUM_BYTE  			10
#define RS485_NUM_BYTE_CONFIG 20

#define RS485_OK		 1
#define RS485_WRONG	 0



/************************************************************************************/
/*DECLARE STRUCT*/
	typedef struct
	{
		uint8_t rxBuff[RS485_NUM_BYTE];
		uint8_t txBuff[RS485_NUM_BYTE];
		uint8_t RxFlag, TxFlag;
		uint8_t STATE;
		uint8_t rxByte, rxPointer;
		uint8_t header;					// header: 0xAB						
		uint8_t CS_byte;							// loai lenh
		uint16_t cnt_mess_sent;				// dem so ban tin da gui
		uint16_t cnt_mess_received;		// dem so ban tin nhan duoc
		uint16_t cnt_mess_wrong;		// dem so ban tin nhan duoc
		
		uint8_t cnt_byte_sent;
		
		uint8_t ID_Byte_H, ID_Byte_L;
		uint16_t ID_Board;
		
		UART_HandleTypeDef *huart;
		
		/*Use for config chute address*/
		uint8_t rxBuff_config[RS485_NUM_BYTE_CONFIG];
		uint8_t txBuff_config[RS485_NUM_BYTE_CONFIG];
		uint8_t RxFlag_config, TxFlag_config;
		uint8_t header_config;
		
	}rs485_t;

	
/*DECLARE FUNCTION*/
	uint8_t RS485_Read(rs485_t *pRx);
	
	void RS485_Init(rs485_t *pRS, UART_HandleTypeDef *huart);
	
	bool RS485_SendData(rs485_t *pRS, USART_TypeDef * USARTx, uint16_t timeout);
	
	uint8_t RS485_Rx_Handle(rs485_t *pRS);
	
	uint8_t RS485_Transmit(rs485_t *pRS);

/*EXTERN*/
extern rs485_t vRS485;	

/************************************************************************************/
#endif /* RS485_H_*/
	

	