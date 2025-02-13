/*
 * rs485.h version 2
 * Created on: 25-June-2024
 * Author: Le Huu An
 */

#ifndef RS485_H_
#define RS485_H_

#include<main.h>
/*Include the type of stm32*/
#include "stm32f1xx_hal.h"

#include <stdbool.h>


/*DEFINE*/
#define USART_RX_END 2
#define USART_READING 1
#define USART_WAIT 0

#define RS485_NUM_BYTE 10



/************************************************************************************/
/*DECLARE STRUCT*/
	typedef struct{
		uint8_t rxBuff[RS485_NUM_BYTE];
		uint8_t txBuff[RS485_NUM_BYTE];
		uint8_t RxFlag, TxFlag;
		uint8_t STATE;
		uint8_t rxByte, rxPointer;
		uint8_t header, ender;					// header: 0xFE, ender: 0xFF						
		uint8_t cmd_type;							// loai lenh
		uint16_t cnt_mess_sent;				// dem so ban tin da gui
		uint16_t cnt_mess_received;		// dem so ban tin nhan duoc
		uint16_t cnt_mess_wrong;		// dem so ban tin nhan duoc
		
		uint8_t cnt_byte_sent;
		
		uint8_t ID_Byte_H, ID_Byte_L;
		uint16_t ID_Board;
		
		UART_HandleTypeDef *huart;
		
	}rs485_t;

	
/*DECLARE FUNCTION*/
	void RS485_Read(rs485_t *pRx);
	void RS485_Init(rs485_t *pRS, UART_HandleTypeDef *huart);
	
	bool RS485_SendData(rs485_t *pRS, USART_TypeDef * USARTx, uint16_t timeout);
	
	uint8_t UartRx_Handle(rs485_t *pRS);
	uint8_t UartTx_Transmit(rs485_t *pRS);

/*EXTERN*/
extern rs485_t vRS485;	

/************************************************************************************/
#endif /* RS485_H_*/
	
	
//			if(cnt_flag_autosend >= 1000){
//			HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET);
//			
//			if(++ID_need_send >= NUMBER_BOARD)
//				ID_need_send = 1;
//		
//			vRS485.txBuff[0] = vRS485.header;
//			vRS485.txBuff[1] = (ID_BOARD_ARRAY[ID_need_send] >> 8) & 0xFF;
//			vRS485.txBuff[2] = ID_BOARD_ARRAY[ID_need_send] & 0xFF;
//			vRS485.txBuff[3] = 0xAA;
//			for(uint8_t i=4; i<SIZE_RS485-1; i++){
//				vRS485.txBuff[i] = 0xFF;
//			}
//			vRS485.txBuff[SIZE_RS485-1] = vRS485.ender;
//			
//			vRS485.TxFlag = 1;
//			cnt_flag_autosend = 0;
//		}
//		
//		if(vRS485.TxFlag == 1){
//				if(RS485_SendData(&vRS485, USART1, 2000)){
//					num_sent++;
//				}
//		}


