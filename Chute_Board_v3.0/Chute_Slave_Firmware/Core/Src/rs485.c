/*
 * rs485.c
 * Created on: 25-June-2024
 * Author: Le Huu An
 */


#include "rs485.h"
#include <string.h>

#include "user.h"
#include "flash_address.h"

/*
BRIEF NOTE: 
	
	
*/
/**********************************************************************************************************************************/
/******EXTERN*******/
	//extern UART_HandleTypeDef huart3;


/**********************************************************************************************************************************/
/*******DECLARE VARIABLE********/
	rs485_t vRS485;		// variable RS485
	
	uint32_t start_wait_send_master = 0;
	uint32_t interval_wait_send_master = 0;
	
	uint8_t Debug_RX_Array[200];
	uint16_t ptr_db = 0;


/**********************************************************************************************************************************/
/*******FUNCTION********/

/*========================================================================================*\
 *																		INIT RS485																					*
\*========================================================================================*/

	void RS485_Init(rs485_t *pRS, UART_HandleTypeDef *huart)
	{
		pRS->huart = huart;
		
		pRS->RxFlag = 0;		pRS->RxFlag_config = 0;
		pRS->rxByte = 0; 		
		pRS->rxPointer = 0;
		pRS->STATE = RS485_WAIT_STATE;
		
		pRS->CS_byte = 0;
		pRS->cnt_mess_received = 0;
		pRS->cnt_mess_wrong = 0;
		
		pRS->TxFlag = 0;		pRS->TxFlag_config = 0;
		pRS->cnt_mess_sent = 0;
		
		memset(pRS->rxBuff, 0xAB, RS485_NUM_BYTE);
		memset(pRS->txBuff, 0xBA, RS485_NUM_BYTE);
		
		memset(pRS->rxBuff_config, 0xBC, RS485_NUM_BYTE_CONFIG);
		memset(pRS->txBuff_config, 0xCB, RS485_NUM_BYTE_CONFIG);
		
		pRS->Sequence_number |= !HAL_GPIO_ReadPin(BIT_5_GPIO_Port, BIT_5_Pin) << 5;
		pRS->Sequence_number |= !HAL_GPIO_ReadPin(BIT_4_GPIO_Port, BIT_4_Pin) << 4;
		pRS->Sequence_number |= !HAL_GPIO_ReadPin(BIT_3_GPIO_Port, BIT_3_Pin) << 3;
		pRS->Sequence_number |= !HAL_GPIO_ReadPin(BIT_2_GPIO_Port, BIT_2_Pin) << 2;
		pRS->Sequence_number |= !HAL_GPIO_ReadPin(BIT_1_GPIO_Port, BIT_1_Pin) << 1;
		pRS->Sequence_number |= !HAL_GPIO_ReadPin(BIT_0_GPIO_Port, BIT_0_Pin) << 0;
	
		//pRS->Sequence_number = 0x01;
		
		HAL_UART_Receive_IT(pRS->huart, &pRS->rxByte, 1);
		HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);
		
		memset(Debug_RX_Array, 0xCC, 200);
	}
	
/*========================================================================================*\
 *															RESET RS485	RECEIVE PARAM																	*
\*========================================================================================*/	
	void RS485_Reset_Rx(rs485_t *pRS)
	{
		pRS->RxFlag = 0;		
		pRS->rxByte = 0; 
		pRS->rxPointer = 0;
		
		pRS->RxFlag_config = 0;
		
		pRS->STATE = RS485_WAIT_STATE;
		
		//memset(pRS->rxBuff, 0x00, RS485_NUM_BYTE);

		HAL_UART_Receive_IT(pRS->huart, &pRS->rxByte, 1);
	}

/*========================================================================================*\
 *															READ DATA FROM RS485																			*
\*========================================================================================*/
	
	uint8_t RS485_Read(rs485_t *pRx)
	{			 
		uint8_t result = 0;
		
		Debug_RX_Array[ptr_db] = vRS485.rxByte;
		if(++ptr_db == 200)
		{
			ptr_db = 0;
			memset(Debug_RX_Array, 0, 200);
		}
		
		/*Process receive data*/ 
		switch(pRx->STATE) 
		{
		//---------------------------------------------------------------------------------	
			/*Wait for start of new message*/
			case  RS485_WAIT_STATE:                
			{
				/*Check Sequence number*/
				if((pRx->rxByte & RS485_CHECK_ID_MASK) == pRx->Sequence_number)
				{  
					/*Check type of command*/
					if((pRx->rxByte & RS485_CMD_MASK) == RS485_CONFIG_CMD)		// Config command
					{
						pRx->rxBuff_config[0] = pRx->rxByte;
						pRx->STATE = RS485_READING_CONFIG_STATE;
						pRx->rxPointer = 1;
					}
					else				// normal command
					{
						pRx->rxBuff[0] = pRx->rxByte;
						pRx->STATE = RS485_READING_STATE;
						pRx->rxPointer = 1;
					}		
				}
				if(pRx->rxByte == 0xFF)
					count_synchronize = 0;
					
				break;
			}  
		//---------------------------------------------------------------------------------
			case RS485_READING_STATE:  
			{
				/*Read data each byte*/
				pRx->rxBuff[pRx->rxPointer] = pRx->rxByte; 		

				/*Read enough byte*/				
				if(pRx->rxPointer == RS485_NUM_BYTE-1)              
				{    
					/*Cal checksum of received mess*/
					pRx->CS_byte = Cal_CheckSum(pRx->rxBuff, RS485_NUM_BYTE);
					
					/*Mess OK -> Turn on the Flag and process data*/	
					if(pRx->rxBuff[RS485_NUM_BYTE-1] == pRx->CS_byte)
					{ 
						HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET);
						pRx->RxFlag = 1;  
						pRx->cnt_mess_received++;				
						pRx->type_cmd = pRx->rxBuff[0] & RS485_CMD_MASK;						
					}
					else
					{
						pRx->cnt_mess_wrong++;	
						result = RS485_WRONG;	//wrong Check Sum byte			
					}
				}
				else
				{
					/*Increase the Pointer*/
					pRx->rxPointer = pRx->rxPointer + 1;  
					
					/*Fail*/
					if(pRx->rxPointer > RS485_NUM_BYTE-1)     
						result = RS485_WRONG;  
				}   
				break;			
			}
		//-----------------------------CONFIG-----------------------------------------------	 
			case RS485_READING_CONFIG_STATE:
			{
				/*Read data CONFIG each byte*/
				pRx->rxBuff_config[pRx->rxPointer] = pRx->rxByte; 
			
				/*Read enough byte CONFIG*/				
				if(pRx->rxPointer == RS485_NUM_BYTE_CONFIG-1)              
				{    
					/*Cal checksum of received mess CONFIG*/
					pRx->CS_byte = Cal_CheckSum(pRx->rxBuff_config, RS485_NUM_BYTE_CONFIG);

					/*CONFIG Mess Ok -> Turn on the Flag and process data*/	
					if(pRx->rxBuff_config[RS485_NUM_BYTE_CONFIG-1] == pRx->CS_byte)
					{ 
						HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET);	
						pRx->RxFlag_config = 1;  
						pRx->cnt_mess_received++;		
						pRx->type_cmd = pRx->rxBuff[0] & RS485_CMD_MASK;	
					}
					else
					{
						pRx->cnt_mess_wrong++;	
						result = RS485_WRONG;	//wrong Check Sum byte			
					}	
				}
				else
				{
					/*Increase the Pointer*/
					pRx->rxPointer = pRx->rxPointer + 1;  
					
					/*Fail CONFIG*/
					if(pRx->rxPointer > RS485_NUM_BYTE_CONFIG-1)     
						result = RS485_WRONG;  
				}   
				break;
			}
		}
		
		//---------------------------------------------------------
		if(result == RS485_WRONG)	
			RS485_Reset_Rx(pRx);
		else
			/*Set rs485 receive interrupt again*/
			HAL_UART_Receive_IT(pRx->huart, &pRx->rxByte, 1);
		
		return result;
	}
	
/*========================================================================================*\
 *															HANDLING DATA RS485 RECEIVE																*
\*========================================================================================*/
	
	uint8_t RS485_Rx_Handle(rs485_t *pRS)
	{
		if(pRS->RxFlag == 1)
		{
			flag_flashLed_trx = true;
			cnt_ping = 0;
			flag_run = true;
			
		/*Do command from App*/
			qty_packet = pRS->rxBuff[1];		// get quantity of packet
//			if(chute_status == CHUTE_CLOSE && pRS->type_cmd == CHUTE_OPEN)		// clear button_Action when open chute
//				button_Action = 0; 
//			if(chute_status != NOT_CONNECT)
			chute_status = pRS->rxBuff[0] & RS485_CMD_MASK; 				// update status
			
			
		/*Response information to App*/
//			pRS->txBuff[0] = pRS->Sequence_number ;
//			pRS->txBuff[1] = (chute_isFull << 4) | (button_Action & 0x0F);		
//			pRS->txBuff[2] = Cal_CheckSum(pRS->txBuff, RS485_NUM_BYTE);
			
			pRS->txBuff[0] = 0xAC;
			pRS->txBuff[1] = (chute_isFull << 7) | (button_Action <<6) | (pRS->Sequence_number & 0x3F);		
			pRS->txBuff[2] = Cal_CheckSum(pRS->txBuff, RS485_NUM_BYTE);
			
			pRS->TxFlag = 1;
			start_wait_send_master = getMicroSecond();
			
			button_Action = 0;
		}
		
		if(pRS->RxFlag_config == 1)
		{
			flag_flashLed_trx = true;
			
			for(uint8_t i=0; i< 12; i++)
				Chute_Infor_update_arr[i] = pRS->rxBuff_config[i+1];
			
//******** che do ngu
			
			chute_status = CHUTE_CONFIG;
			RS485_Reset_Rx(pRS);
		}
		return 1;
	}
	
/*========================================================================================*\
 *															TRANSMIT RS485 ALL BYTE																		*
\*========================================================================================*/
	
	uint8_t RS485_Transmit(rs485_t *pRS)
	{
		if(pRS->TxFlag == 1)
		{
			interval_wait_send_master = getMicroSecond() - start_wait_send_master;
			if(interval_wait_send_master >= 5000)
			{
				HAL_UART_Transmit(pRS->huart, pRS->txBuff, RS485_NUM_BYTE, HAL_MAX_DELAY);
				
				HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);
				memset(pRS->txBuff, 0xE0, RS485_NUM_BYTE);
				pRS->cnt_mess_sent++;			
				
				pRS->TxFlag = 0;
				RS485_Reset_Rx(pRS);
				//interval_wait_send_master = 0;
			}
		}
		
		if(pRS->TxFlag_config == 1)
		{
			interval_wait_send_master = getMicroSecond() - start_wait_send_master;
			
			if(interval_wait_send_master >= 2000)
			{
				HAL_UART_Transmit(pRS->huart, pRS->txBuff_config, RS485_NUM_BYTE, HAL_MAX_DELAY);
				
				OFF_DEN_BAO(); 	HAL_Delay(100);
				ON_DEN_BAO(); 	HAL_Delay(100);
				OFF_DEN_BAO(); 	HAL_Delay(100);
				ON_DEN_BAO(); 	HAL_Delay(100);
				OFF_DEN_BAO(); 	HAL_Delay(100);
				ON_DEN_BAO(); 	HAL_Delay(100);
				
				pRS->TxFlag_config = 0;
				
				/*Soft reset MCU*/
				NVIC_SystemReset();
			}
		}
		return 0;
	}


/**********************************************************************************************************************************/
/*******END PAGE********/
