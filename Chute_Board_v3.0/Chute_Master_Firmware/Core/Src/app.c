/*
 * app.c
 * Created on: 15-Jan-2025
 * Author: Le Huu An
 */


#include "app.h"
#include <string.h>

#include "user.h"

/*
BRIEF NOTE: 
	
	
*/
/**********************************************************************************************************************************/
/******EXTERN*******/
	extern UART_HandleTypeDef huart1;


/**********************************************************************************************************************************/
/*******DECLARE VARIABLE********/
	rs485_t vRS485;		// variable RS485


/**********************************************************************************************************************************/
/*******FUNCTION********/

///*========================================================================================*\
// *																		INIT RS485																					*
//\*========================================================================================*/

//	void RS485_Init(rs485_t *pRS, UART_HandleTypeDef *huart)
//	{
//		pRS->huart = huart;
//		
//		pRS->RxFlag = 0;		pRS->RxFlag_config = 0;
//		pRS->rxByte = 0; 		
//		pRS->rxPointer = 0;
//		pRS->STATE = RS485_WAIT_STATE;
//		
//		pRS->CS_byte = 0;
//		pRS->cnt_mess_received = 0;
//		pRS->cnt_mess_wrong = 0;
//		
//		pRS->TxFlag = 0;		pRS->TxFlag_config = 0;
//		pRS->cnt_mess_sent = 0;
//		
//		memset(pRS->rxBuff, 0xAB, RS485_NUM_BYTE);
//		memset(pRS->txBuff, 0xBA, RS485_NUM_BYTE);
//		
//		memset(pRS->rxBuff_config, 0xBC, RS485_NUM_BYTE_CONFIG);
//		memset(pRS->txBuff_config, 0xCB, RS485_NUM_BYTE_CONFIG);
//		

//		
//		HAL_UART_Receive_IT(pRS->huart, &pRS->rxByte, 1);
//		HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);
//	}
//	
///*========================================================================================*\
// *															RESET RS485	RECEIVE PARAM																	*
//\*========================================================================================*/	
//	void RS485_Reset_Rx(rs485_t *pRS)
//	{
//		pRS->RxFlag = 0;		
//		pRS->rxByte = 0; 
//		pRS->rxPointer = 0;
//		
//		pRS->RxFlag_config = 0;
//		
//		pRS->STATE = RS485_WAIT_STATE;
//		
//		//memset(pRS->rxBuff, 0x00, RS485_NUM_BYTE);

//		HAL_UART_Receive_IT(pRS->huart, &pRS->rxByte, 1);
//	}

///*========================================================================================*\
// *															READ DATA FROM RS485																			*
//\*========================================================================================*/
//	
//	uint8_t RS485_Read(rs485_t *pRx)
//	{			 
//		uint8_t result = 0;
//		
//		/*Process receive data*/ 
//		switch(pRx->STATE) 
//		{
//		//---------------------------------------------------------------------------------	
//			/*Wait for start of new message*/
//			case  RS485_WAIT_STATE:                
//			{
//				/*Check Sequence number*/
//				if((pRx->rxByte & RS485_CHECK_ID_MASK) == pRx->Sequence_number)
//				{  
//					/*Check type of command*/
//					if((pRx->rxByte & RS485_CMD_MASK) == RS485_CONFIG_CMD)		// Config command
//					{
//						pRx->rxBuff_config[0] = pRx->rxByte;
//						pRx->STATE = RS485_READING_CONFIG_STATE;
//						pRx->rxPointer = 1;
//					}
//					else				// normal command
//					{
//						pRx->rxBuff[0] = pRx->rxByte;
//						pRx->STATE = RS485_READING_STATE;
//						pRx->rxPointer = 1;
//					}		
//				}
//				if(pRx->rxByte == 0xFF)
//					count_synchronize = 0;
//					
//				break;
//			}  
//		//---------------------------------------------------------------------------------
//			case RS485_READING_STATE:  
//			{
//				/*Read data each byte*/
//				pRx->rxBuff[pRx->rxPointer] = pRx->rxByte; 		

//				/*Read enough byte*/				
//				if(pRx->rxPointer == RS485_NUM_BYTE-1)              
//				{    
//					/*Cal checksum of received mess*/
//					pRx->CS_byte = Cal_CheckSum(pRx->rxBuff, RS485_NUM_BYTE);
//					
//					/*Mess OK -> Turn on the Flag and process data*/	
//					if(pRx->rxBuff[RS485_NUM_BYTE-1] == pRx->CS_byte)
//					{ 
//						HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET);
//						pRx->RxFlag = 1;  
//						pRx->cnt_mess_received++;				
//						pRx->type_cmd = pRx->rxBuff[0] & RS485_CMD_MASK;						
//					}
//					else
//					{
//						pRx->cnt_mess_wrong++;	
//						result = RS485_WRONG;	//wrong Check Sum byte			
//					}
//				}
//				else
//				{
//					/*Increase the Pointer*/
//					pRx->rxPointer = pRx->rxPointer + 1;  
//					
//					/*Fail*/
//					if(pRx->rxPointer > RS485_NUM_BYTE-1)     
//						result = RS485_WRONG;  
//				}   
//				break;			
//			}
//		//-----------------------------CONFIG-----------------------------------------------	 
//			case RS485_READING_CONFIG_STATE:
//			{
//				/*Read data CONFIG each byte*/
//				pRx->rxBuff_config[pRx->rxPointer] = pRx->rxByte; 
//			
//				/*Read enough byte CONFIG*/				
//				if(pRx->rxPointer == RS485_NUM_BYTE_CONFIG-1)              
//				{    
//					/*Cal checksum of received mess CONFIG*/
//					pRx->CS_byte = Cal_CheckSum(pRx->rxBuff_config, RS485_NUM_BYTE_CONFIG);

//					/*CONFIG Mess Ok -> Turn on the Flag and process data*/	
//					if(pRx->rxBuff_config[RS485_NUM_BYTE_CONFIG-1] == pRx->CS_byte)
//					{ 
//						HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET);	
//						pRx->RxFlag_config = 1;  
//						pRx->cnt_mess_received++;		
//						pRx->type_cmd = pRx->rxBuff[0] & RS485_CMD_MASK;	
//					}
//					else
//					{
//						pRx->cnt_mess_wrong++;	
//						result = RS485_WRONG;	//wrong Check Sum byte			
//					}	
//				}
//				else
//				{
//					/*Increase the Pointer*/
//					pRx->rxPointer = pRx->rxPointer + 1;  
//					
//					/*Fail CONFIG*/
//					if(pRx->rxPointer > RS485_NUM_BYTE_CONFIG-1)     
//						result = RS485_WRONG;  
//				}   
//				break;
//			}
//		}
//		
//		//---------------------------------------------------------
//		if(result == RS485_WRONG)	
//			RS485_Reset_Rx(pRx);
//		else
//			/*Set rs485 receive interrupt again*/
//			HAL_UART_Receive_IT(pRx->huart, &pRx->rxByte, 1);
//		
//		return result;
//	}
//	
///*========================================================================================*\
// *															HANDLING DATA RS485 RECEIVE																*
//\*========================================================================================*/
//	
//	uint8_t RS485_Rx_Handle(rs485_t *pRS)
//	{
//		if(pRS->RxFlag == 1)
//		{
//			flag_flashLed_trx = true;
//			cnt_ping = 0;
//			flag_run = true;
//			
//		/*Do command from App*/
//			qty_packet = pRS->rxBuff[1];		// get quantity of packet
////			if(chute_status == CHUTE_CLOSE && pRS->type_cmd == CHUTE_OPEN)		// clear button_Action when open chute
////				button_Action = 0; 
//			if(chute_status != NOT_CONNECT)
//				chute_status = pRS->rxBuff[0] & RS485_CMD_MASK; 				// update status
//			
//			
//		/*Response information to App*/
//			pRS->txBuff[0] = pRS->Sequence_number ;
//			pRS->txBuff[1] = (chute_isFull << 4) | (button_Action & 0x0F);		
//			pRS->txBuff[2] = Cal_CheckSum(pRS->txBuff, RS485_NUM_BYTE);
//			pRS->TxFlag = 1;
//			
//			button_Action = 0;
//			
//			RS485_Reset_Rx(pRS);
//		}
//		
//		if(pRS->RxFlag_config == 1)
//		{
//			flag_flashLed_trx = true;
//			
//			for(uint8_t i=0; i< 12; i++)
//				Chute_Infor_update_arr[i] = pRS->rxBuff_config[i+1];
//			
////******** che do ngu
//			
//			chute_status = CHUTE_CONFIG;
//			RS485_Reset_Rx(pRS);
//		}
//		return 1;
//	}
//	
///*========================================================================================*\
// *															TRANSMIT RS485 ALL BYTE																		*
//\*========================================================================================*/
//	
//	uint8_t RS485_Transmit(rs485_t *pRS)
//	{
//		if(pRS->TxFlag == 1)
//		{
//			HAL_UART_Transmit(pRS->huart, pRS->txBuff, RS485_NUM_BYTE, HAL_MAX_DELAY);
//			
//			HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);
//			memset(pRS->txBuff, 0xE0, RS485_NUM_BYTE);
//			pRS->cnt_mess_sent++;			
//			
//			pRS->TxFlag = 0;
//		}
//		if(pRS->TxFlag_config == 1)
//		{
//			pRS->TxFlag_config = 0;
//			
//			HAL_UART_Transmit(pRS->huart, pRS->txBuff_config, RS485_NUM_BYTE, HAL_MAX_DELAY);
//			
//			OFF_LED_BAO(); 	HAL_Delay(100);
//			ON_LED_BAO(); 	HAL_Delay(100);
//			OFF_LED_BAO(); 	HAL_Delay(100);
//			ON_LED_BAO(); 	HAL_Delay(100);
//			OFF_LED_BAO(); 	HAL_Delay(100);
//			ON_LED_BAO(); 	HAL_Delay(100);
//			
//			/*Soft reset MCU*/
//			NVIC_SystemReset();
//		}
//		return 0;
//	}

	
	/*______________________________________________________________________________*/
	
//	void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) 
//	{  
//		/*RS485*/
//		if(huart->Instance == USART1)			
//		{    
////			RS485_Read(&vRS485);
////			RS485_Rx_Handle(&vRS485);
//		}
//	}

/**********************************************************************************************************************************/
/*******END PAGE********/
