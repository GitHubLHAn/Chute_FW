/*
 * slave.h
 * Created on: 15-Jan-2025
 * Author: Le Huu An
 */


#ifndef SLAVE_H_
#define SLAVE_H_

#include "main.h"

#include <stdbool.h>


/*DEFINE*/
	#define NUMBER_SLAVE	31
	
	#define NUM_BYTE_SLAVE	3
	
	#define SLAVE_STATE_MASK 				0xC0		// 11 000000		(AND to hold bit 7,6)
	#define SLAVE_OPEN_CHUTE_CMD		0x00		// 00 000000		
	#define SLAVE_CLOSE_CHUTE_CMD		0x40		// 01 000000
	#define SLAVE_CONFIG_CMD				0x80		// 10 000000

//	#define SLAVE_CHECK_ID_MASK		0x3F
//	
//	#define SLAVE_CMD_MASK					0xC0
//	#define SLAVE_CONFIG_CMD				0x80		// 10 000000
//	#define SLAVE_OPEN_CHUTE_CMD		0x00		// 00 000000
//	#define SLAVE_CLOSE_CHUTE_CMD		0x40		// 01 000000

	#define SLAVE_READ_HD						0		// read ID
	#define SLAVE_READ_DT						1		// read quantity
	#define SLAVE_READ_CS						2		// read checksum
	#define SLAVE_READ_DT_CONFIG				3		// read checksum
	#define SLAVE_READ_CS_CONFIG				4		// read checksum


/************************************************************************************/
/*DECLARE STRUCT*/
	typedef struct
	{
		UART_HandleTypeDef *phuart;
		
		bool sl_enable;
		bool sl_connected;
		
		uint8_t sl_qty;
		uint8_t sl_status;
		
		uint8_t sl_isFull;
		uint8_t sl_flag_button;
		
		uint16_t cnt_disconnect;
		
		uint32_t cnt_mess_tx;
		uint32_t cnt_mess_rx;
		uint32_t cnt_mess_miss;
		float ratio_miss;
		
	}slave_t;
	
	typedef struct
	{
		UART_HandleTypeDef *phuart;
		
		uint8_t rxBuff[NUM_BYTE_SLAVE];
		uint8_t RxFlag;
		uint8_t STATE;
		uint8_t rxByte;		
		
		uint16_t cnt_mess_sent;				
		uint32_t cnt_mess_received;		
		uint16_t cnt_mess_wrong;		
		
	}slave_com_t;		// slave communication

	
/*DECLARE FUNCTION*/
	
	void Slave_Init_Func(UART_HandleTypeDef *huart);

	void Send_Slave_Sync(uint8_t *_vSync);
		
	uint8_t SendCMD_Slave(void);
	
	void On_Mode_Send_Slave(void);
	
	void On_Mode_Receive_Slave(void);
	
	void Wait_Send_Slave(void);

/*EXTERN*/
	
	extern slave_t List_Chute_Arr[NUMBER_SLAVE];

	extern uint8_t vSL_Ptr;
	
	extern bool flag_wait_sending_slave;

/************************************************************************************/
#endif /* SLAVE_H_*/
	

	