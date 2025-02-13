/*
 * slave.c
 * Created on: 15-Jan-2025
 * Author: Le Huu An
 */


#include "slave.h"

#include "user.h"

#include "string.h"

/*
BRIEF NOTE: 
	
	
*/
/**********************************************************************************************************************************/
/******EXTERN*******/
	extern UART_HandleTypeDef huart2;
	extern UART_HandleTypeDef huart3;


/**********************************************************************************************************************************/
/*******DECLARE VARIABLE********/

	uint8_t TX_toSlave[NUM_BYTE_SLAVE] __attribute__((aligned(4)));
	uint8_t RX_fromSlave[NUM_BYTE_SLAVE];
	
	slave_t List_Chute_Arr[NUMBER_SLAVE];
	
	uint8_t vSL_Ptr = 0;			// Slave Pointer
	
	bool flag_wait_sending_slave = false;
	
	slave_com_t vSlaveCom;


/**********************************************************************************************************************************/
/*******FUNCTION********/

//========================================================================================
//
//========================================================================================
 void On_Mode_Send_Slave(void)
 {
		HAL_GPIO_WritePin(RS485_SLAVEA_DE_GPIO_Port, RS485_SLAVEA_DE_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(RS485_SLAVEA_DE_GPIO_Port, RS485_SLAVEA_DE_Pin, GPIO_PIN_SET);
 }

//========================================================================================
//
//========================================================================================
	void On_Mode_Receive_Slave(void)
  {
		HAL_GPIO_WritePin(RS485_SLAVEA_DE_GPIO_Port, RS485_SLAVEA_DE_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(RS485_SLAVEA_DE_GPIO_Port, RS485_SLAVEA_DE_Pin, GPIO_PIN_RESET);
	}

//========================================================================================
//
//========================================================================================
	void Slave_Init_Func(UART_HandleTypeDef *huart)
	{
		for(uint8_t index=0; index<NUMBER_SLAVE; index++)
		{
			List_Chute_Arr[index].phuart = huart;
			
			List_Chute_Arr[index].sl_enable = false;
			List_Chute_Arr[index].sl_status = SLAVE_OPEN_CHUTE_CMD;
			
			List_Chute_Arr[index].sl_connected = true;
			List_Chute_Arr[index].cnt_disconnect = 0;
			
			List_Chute_Arr[index].sl_qty = 0;
			List_Chute_Arr[index].sl_isFull = 0;
			List_Chute_Arr[index].sl_flag_button = 0;
			
			List_Chute_Arr[index].cnt_mess_tx = 0;
			List_Chute_Arr[index].cnt_mess_rx = 0;
			List_Chute_Arr[index].cnt_mess_miss = 0;
			List_Chute_Arr[index].ratio_miss = 0;
		}
		
		List_Chute_Arr[1].sl_enable = true;
		List_Chute_Arr[2].sl_enable = true;
		List_Chute_Arr[3].sl_enable = true;
		List_Chute_Arr[4].sl_enable = true;
		List_Chute_Arr[5].sl_enable = true;
		List_Chute_Arr[6].sl_enable = true;
		
		vSlaveCom.STATE = SLAVE_READ_HD;
		vSlaveCom.rxByte = 0;	vSlaveCom.RxFlag = 0;
		vSlaveCom.cnt_mess_received = 0;
		vSlaveCom.phuart = huart;

		memset(TX_toSlave, 0xAE, NUM_BYTE_SLAVE);
		memset(RX_fromSlave, 0xEA, NUM_BYTE_SLAVE);
		
		On_Mode_Receive_Slave();
//		HAL_UARTEx_ReceiveToIdle_DMA(huart, RX_fromSlave, NUM_BYTE_SLAVE);
//		__HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
		HAL_UART_Receive_IT(vSlaveCom.phuart, &vSlaveCom.rxByte, 1);

	}

//========================================================================================
//
//========================================================================================
	uint32_t start_send_slave = 0;
	uint32_t interval_send_slave = 0;
	
	void Wait_Send_Slave(void)
	{
		if(flag_wait_sending_slave)
		{
			interval_send_slave = getMicroSecond() - start_send_slave;
			if(interval_send_slave >= 2000)
			{
				interval_send_slave = 0;
				On_Mode_Receive_Slave();
				flag_wait_sending_slave = false;
				//cnt_db_tx++;
			}
		}
	}


//========================================================================================
//
//========================================================================================
	void Send_Slave_Sync(uint8_t _vSync)
	{
		On_Mode_Send_Slave();
		
		HAL_UART_Transmit_DMA(List_Chute_Arr[vSL_Ptr].phuart, &_vSync, 1);
		
		On_Mode_Receive_Slave();
	}

//========================================================================================
//
//========================================================================================
	uint8_t cnt_chute1_debug = 0;
	
	uint8_t SendCMD_Slave(void)
	{
		bool flag_found = false;
		uint8_t cnt_scan_slave = NUMBER_SLAVE;
		
		/*Find the Enable Slave to send CMD*/
		while(cnt_scan_slave > 0)
		{
			if(List_Chute_Arr[vSL_Ptr].sl_enable)
			{
				On_Mode_Send_Slave();
				flag_found = true;
				break;
			}
			else
			{
				if(++vSL_Ptr == NUMBER_SLAVE)
					vSL_Ptr = 0;
			
				cnt_scan_slave--;		
			}		
		}
		
		/*Send CMD to Slave be found that be enable*/
		if(flag_found)
		{
//******DEBUG
			if(vSL_Ptr == 0){
				interval = getMicroSecond() - start_time;
				start_time = getMicroSecond();
			}
//			if(vSL_Ptr == 1)
//				List_Chute_Arr[1].sl_qty += 2;
		List_Chute_Arr[vSL_Ptr].sl_qty+=1;
		List_Chute_Arr[vSL_Ptr].cnt_mess_miss = List_Chute_Arr[vSL_Ptr].cnt_mess_tx - 1 - List_Chute_Arr[vSL_Ptr].cnt_mess_rx;
		List_Chute_Arr[vSL_Ptr].ratio_miss = ((float)List_Chute_Arr[vSL_Ptr].cnt_mess_rx*100)/((float)List_Chute_Arr[vSL_Ptr].cnt_mess_tx);
//***********
		
			TX_toSlave[0] = (List_Chute_Arr[vSL_Ptr].sl_status & SLAVE_STATE_MASK) | (vSL_Ptr & 0x3F);
			TX_toSlave[1] = List_Chute_Arr[vSL_Ptr].sl_qty;
			TX_toSlave[NUM_BYTE_SLAVE-1] = Cal_CheckSum(TX_toSlave, NUM_BYTE_SLAVE);
			/*
			 * Byte 0: bit7,6 define state of Slave, bit 5,4,3,2,1,0 define the sequense of Slave
			 * Byte 1: quantity of packet in chute slave
			 * Byte 2: Checksum Byte
			 */
		
			HAL_UART_Transmit_DMA(List_Chute_Arr[vSL_Ptr].phuart, TX_toSlave, NUM_BYTE_SLAVE);
			//HAL_UART_Transmit(List_Chute_Arr[vSL_Ptr].phuart, TX_toSlave, NUM_BYTE_SLAVE, HAL_MAX_DELAY);
			
			flag_wait_sending_slave = true;
			start_send_slave = getMicroSecond();
			//On_Mode_Receive_Slave();
			List_Chute_Arr[vSL_Ptr].cnt_mess_tx++;
		}	

		
		if(++vSL_Ptr == NUMBER_SLAVE)
			vSL_Ptr = 0;
		
		return 1;
	}


//========================================================================================
//
//========================================================================================
//	volatile uint16_t cnt_db_tx = 0;
//	void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
//	{
//			if (huart->Instance == USART2)
//			{
//					// Truy?n hoàn t?t, có th? báo hi?u cho b? nh?n ho?c x? lý ti?p
//				cnt_db_tx++;
//			}
//	}
	
//========================================================================================
//
//========================================================================================
	
	void Read_Slave(slave_com_t *pCOM)
	{
		switch(pCOM->STATE)
		{
		//---------------------
			case SLAVE_READ_HD:
			{
				if(pCOM->rxByte == 0xAC)
				{
					RX_fromSlave[0] = pCOM->rxByte;
					pCOM->STATE = SLAVE_READ_DT;
				}
			
				break;
			}
		//---------------------
			case SLAVE_READ_DT:
			{
				RX_fromSlave[1] = pCOM->rxByte;
				pCOM->STATE = SLAVE_READ_CS;
				break;
			}
		//---------------------
			case SLAVE_READ_CS:
			{
				RX_fromSlave[2] = pCOM->rxByte;
				uint8_t check_CS = 0;
				check_CS = Cal_CheckSum(RX_fromSlave, 3);
				if(check_CS == RX_fromSlave[2])
				{
					pCOM->RxFlag = 1;				
				}
				pCOM->STATE = SLAVE_READ_HD;
				break;
			}	
		}
		
		HAL_UART_Receive_IT(pCOM->phuart, &pCOM->rxByte, 1);
	}




//========================================================================================
//
//========================================================================================
	extern uint32_t db_i, db_s;
	
	void Handle_Slave_RX(slave_com_t *pCOM)
	{
//		uint8_t CS_byte = 0;
		uint8_t Slave_sequence = 0;
//		
//		CS_byte = Cal_CheckSum(RX_fromSlave, NUM_BYTE_SLAVE);
//		if(CS_byte == RX_fromSlave[NUM_BYTE_SLAVE-1])
//		{
//			Slave_sequence = RX_fromSlave[0];
//			List_Chute_Arr[Slave_sequence].sl_isFull = (RX_fromSlave[1] >> 4) & 0x0F;
//			List_Chute_Arr[Slave_sequence].sl_flag_button = RX_fromSlave[1] & 0x0F;
//			List_Chute_Arr[Slave_sequence].cnt_mess_rx++;
//			List_Chute_Arr[Slave_sequence].cnt_disconnect = 0;
//			List_Chute_Arr[Slave_sequence].sl_connected = true;
//			
//			db_i = getMicroSecond() - db_s;
//		}	
//		
//		HAL_UARTEx_ReceiveToIdle_DMA(List_Chute_Arr[vSL_Ptr].phuart, RX_fromSlave, NUM_BYTE_SLAVE);
//		__HAL_DMA_DISABLE_IT(List_Chute_Arr[vSL_Ptr].phuart->hdmarx, DMA_IT_HT);
		if(pCOM->RxFlag == 1)
		{
			Slave_sequence = RX_fromSlave[1] & 0x3F;
			List_Chute_Arr[Slave_sequence].sl_isFull = (RX_fromSlave[1] >> 7) & 0x0F;
			List_Chute_Arr[Slave_sequence].sl_flag_button = (RX_fromSlave[1] >> 6) & 0x0F;
			List_Chute_Arr[Slave_sequence].cnt_mess_rx++;
			List_Chute_Arr[Slave_sequence].cnt_disconnect = 0;
			List_Chute_Arr[Slave_sequence].sl_connected = true;
		
			pCOM->RxFlag = 0;
		}
		
	}


//========================================================================================
// Callback function receive DMA mess from slave
//========================================================================================
//	volatile uint16_t cnt_irq_uart = 0;
//	void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
//	{
//		/* Prevent unused argument(s) compilation warning */
//		UNUSED(huart);
//		UNUSED(Size);

//		if (huart->Instance == USART2) 
//		{
//			Handle_Slave_RX();
//			flag_flashLed_trxA = true;
//			cnt_irq_uart++;
//    }
//		if (huart->Instance == USART3) 
//		{
//      Handle_Slave_RX();
//			flag_flashLed_trxB = true;
//    }
//	}
	
/*______________________________________________________________________________*/
	
	volatile uint16_t cnt1 = 0;
	
	void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) 
	{  
		/*RS485*/
		if(huart->Instance == USART2)			
		{    
			Read_Slave(&vSlaveCom);
			Handle_Slave_RX(&vSlaveCom);
			cnt1++;
		}
	}


/**********************************************************************************************************************************/
/*******END PAGE********/
