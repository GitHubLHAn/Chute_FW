/*
 * rs485.h version 2
 * Created on: 25-June-2024
 * Author: Le Huu An
 */


#include "rs485.h"
#include <string.h>

#include "user.h"

/*
BRIEF NOTE: 
	+ Su dung cho mach chute version YODY
	
*/
/**********************************************************************************************************************************/
/******EXTERN*******/
	extern UART_HandleTypeDef huart3;


/**********************************************************************************************************************************/
/*******DECLARE VARIABLE********/
	rs485_t vRS485;		// variable RS485


/**********************************************************************************************************************************/
/*******FUNCTION********/

/*========================================================================================*\
 *																		INIT RS485																					*
\*========================================================================================*/

	void RS485_Init(rs485_t *pRS, UART_HandleTypeDef *huart)
	{
		pRS->huart = huart;
		__HAL_UART_CLEAR_FLAG(pRS->huart, UART_FLAG_TC);
		
		pRS->RxFlag = 0;		pRS->RxFlag_config = 0;
		pRS->rxByte = 0; 		
		pRS->rxPointer = 0;
		pRS->STATE = RS485_WAIT;
		
		pRS->CS_byte = 0;
		pRS->cnt_mess_received = 0;
		pRS->cnt_mess_wrong = 0;
		
		pRS->TxFlag = 0;		pRS->TxFlag_config = 0;
		pRS->cnt_mess_sent = 0; 
		pRS->cnt_byte_sent = 0;
		pRS->header = 0xAB;		pRS->header_config = 0xBC;
		
		memset(pRS->rxBuff, 0x00, RS485_NUM_BYTE);
		memset(pRS->txBuff, 0x00, RS485_NUM_BYTE);
		memset(pRS->rxBuff_config, 0x00, RS485_NUM_BYTE);
		memset(pRS->txBuff_config, 0x00, RS485_NUM_BYTE);
		
		pRS->ID_Byte_L |= !HAL_GPIO_ReadPin(ADDR0_GPIO_Port, ADDR0_Pin) << 0;
		pRS->ID_Byte_L |= !HAL_GPIO_ReadPin(ADDR1_GPIO_Port, ADDR1_Pin) << 1;
		pRS->ID_Byte_L |= !HAL_GPIO_ReadPin(ADDR2_GPIO_Port, ADDR2_Pin) << 2;
		pRS->ID_Byte_L |= !HAL_GPIO_ReadPin(ADDR3_GPIO_Port, ADDR3_Pin) << 3;
		pRS->ID_Byte_L |= !HAL_GPIO_ReadPin(ADDR4_GPIO_Port, ADDR4_Pin) << 4;
		pRS->ID_Byte_L |= !HAL_GPIO_ReadPin(ADDR5_GPIO_Port, ADDR5_Pin) << 5;
		pRS->ID_Byte_L |= !HAL_GPIO_ReadPin(ADDR6_GPIO_Port, ADDR6_Pin) << 6;
		pRS->ID_Byte_L |= !HAL_GPIO_ReadPin(ADDR7_GPIO_Port, ADDR7_Pin) << 7;
		
		pRS->ID_Byte_H |= !HAL_GPIO_ReadPin(ADDR8_GPIO_Port, ADDR8_Pin) << 0;
		pRS->ID_Byte_H |= !HAL_GPIO_ReadPin(ADDR9_GPIO_Port, ADDR9_Pin) << 1;
		pRS->ID_Byte_H |= !HAL_GPIO_ReadPin(ADDR10_GPIO_Port, ADDR10_Pin) << 2;
		pRS->ID_Byte_H |= !HAL_GPIO_ReadPin(ADDR11_GPIO_Port, ADDR11_Pin) << 3;
		
		pRS->ID_Board = (pRS->ID_Byte_H << 8) | pRS->ID_Byte_L;
		
		HAL_UART_Receive_IT(pRS->huart, &pRS->rxByte, 1);
		HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);
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
		
		pRS->STATE = RS485_WAIT;
		
		//memset(pRS->rxBuff, 0x00, RS485_NUM_BYTE);

		HAL_UART_Receive_IT(pRS->huart, &pRS->rxByte, 1);
	}

/*========================================================================================*\
 *															READ DATA FROM RS485																			*
\*========================================================================================*/
	
	uint8_t RS485_Read(rs485_t *pRx)
	{			 
		uint8_t result = RS485_OK;
		
		/*Process receive data*/ 
		switch(pRx->STATE) 
		{
		//---------------------------------------------------------------------------------	
			/*Wait for start of new message*/
			case  RS485_WAIT:                
			{
				/*Check header high*/
				if(pRx->rxByte == pRx->header){       
					pRx->rxBuff[0] = pRx->rxByte;
					
					/*Switch to Reading state  */
					pRx->STATE = RS485_READING;    
					pRx->rxPointer = 1;
				}   
				else if(pRx->rxByte == pRx->header_config){
					pRx->rxBuff_config[0] = pRx->rxByte;
					
					/*Switch to Reading CONFIG CMD*/
					pRx->STATE = RS485_READING_CONFIG;    
					pRx->rxPointer = 1;
				
				}
			break;
			}  
		//---------------------------------------------------------------------------------
			case RS485_READING:  
			{
				/*Read data each byte*/
				pRx->rxBuff[pRx->rxPointer] = pRx->rxByte; 		

				/*Read enough byte*/				
				if(pRx->rxPointer == RS485_NUM_BYTE-1)              
				{    
					/*Cal checksum of received mess*/
					pRx->CS_byte = Cal_CheckSum(pRx->rxBuff, RS485_NUM_BYTE);
					
					/*Check ID of mess*/
					if(pRx->rxBuff[1] == pRx->ID_Byte_H && pRx->rxBuff[2] == pRx->ID_Byte_L)		
					{
						/*Success -> Turn on the Flag and process data*/	
						if(pRx->rxBuff[RS485_NUM_BYTE-1] == pRx->CS_byte)
						{ 
							pRx->RxFlag = 1;  
							pRx->cnt_mess_received++;		
							HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET);	
						}
						else	result = RS485_WRONG;	//wrong checksum byte
					}
					else result = RS485_WRONG;	//wrong ID				
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
			case RS485_READING_CONFIG:
			{
				/*Read data CONFIG each byte*/
				pRx->rxBuff_config[pRx->rxPointer] = pRx->rxByte; 
			
				/*Read enough byte CONFIG*/				
				if(pRx->rxPointer == RS485_NUM_BYTE_CONFIG-1)              
				{    
					/*Cal checksum of received mess CONFIG*/
					pRx->CS_byte = Cal_CheckSum(pRx->rxBuff_config, RS485_NUM_BYTE_CONFIG);
					
					/*Check ID of CONFIG mess*/
					if(pRx->rxBuff_config[1] == pRx->ID_Byte_H && pRx->rxBuff_config[2] == pRx->ID_Byte_L)		
					{
						/*CONFIG Success -> Turn on the Flag and process data*/	
						if(pRx->rxBuff_config[RS485_NUM_BYTE_CONFIG-1] == pRx->CS_byte)
						{ 
							pRx->RxFlag_config = 1;  
							pRx->cnt_mess_received++;		
							HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET);	
						}
						else	result = RS485_WRONG;	//wrong checksum byte
					}
					else result = RS485_WRONG;	//wrong ID				
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
			flag_flashLed = 1;
			cnt_ping = 0;
			
		/*Do command from App*/
			count_packet = (pRS->rxBuff[3] << 8) | pRS->rxBuff[4];		// get quantity of packet
			if(chute_status == CHUTE_CLOSE && pRS->rxBuff[5] == CHUTE_NORMAL)		// clear button_Action when open chute
				button_Action = 0;
			chute_status = pRS->rxBuff[5]; 				// update status
			status_thap_led = pRS->rxBuff[6];			// control thap led
			status_thap_coi = pRS->rxBuff[7];			// control speaker
			
		/*Response information to App*/
			pRS->txBuff[0] = pRS->header;
			pRS->txBuff[1] = pRS->ID_Byte_H;
			pRS->txBuff[2] = pRS->ID_Byte_L;
			pRS->txBuff[3] = chute_isFull;		// return full status
			pRS->txBuff[4] = button_Action;		
			pRS->txBuff[5] = 0xFE;
			pRS->txBuff[6] = 0xFE;
			pRS->txBuff[7] = 0xFE;
			pRS->txBuff[8] = 0xFE;
			pRS->txBuff[9] = Cal_CheckSum(pRS->txBuff, RS485_NUM_BYTE);
			pRS->TxFlag = 1;
			
			RS485_Reset_Rx(pRS);
		}
		
		if(pRS->RxFlag_config == 1)
		{
			flag_flashLed = 1;
			
			for(uint8_t i=0; i< 12; i++)
				Chute_Infor_update_arr[i] = pRS->rxBuff_config[i+3];
			
			chute_status = CHUTE_CONFIG;
			flag_update_address = true;
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
			pRS->TxFlag = 0;
			
			HAL_UART_Transmit(pRS->huart, pRS->txBuff, RS485_NUM_BYTE, HAL_MAX_DELAY);
			
			HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);
			memset(pRS->txBuff, 0xE0, RS485_NUM_BYTE);
			pRS->cnt_mess_sent++;			
		}
		if(pRS->TxFlag_config == 1)
		{
			pRS->TxFlag_config = 0;
			
			HAL_UART_Transmit(pRS->huart, pRS->txBuff_config, RS485_NUM_BYTE_CONFIG, HAL_MAX_DELAY);
			
			HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);
			pRS->cnt_mess_sent++;
			memset(pRS->txBuff_config, 0xE0, RS485_NUM_BYTE_CONFIG);
			
			NVIC_SystemReset();
		}
		return 0;
	}
	
	
/*========================================================================================*\
 *													SEND DATA ONE BYTE		(**NOT USED)															*
\*========================================================================================*/
	static uint8_t isSending = 0;	
	bool RS485_SendData(rs485_t *pRS, USART_TypeDef * USARTx, uint16_t timeout)
	{
		//uint8_t TC = 0;
		uint8_t cnt_clear_TC = 10;
		bool result = false;
		//TC = USARTx->SR & USART_SR_TC;
		
		if((isSending == 1) && ((USARTx->SR & USART_SR_TC) != 0)){
			if(++pRS->cnt_byte_sent == RS485_NUM_BYTE){
				pRS->TxFlag = 0;
				pRS->cnt_mess_sent++;
				pRS->cnt_byte_sent = 0;
				result = true;
			}
			__HAL_UART_CLEAR_FLAG(pRS->huart, UART_FLAG_TC);	
			isSending = 0;
		}
		
		if(isSending == 0 && pRS->TxFlag == 1){
				// clear TC flag
			__HAL_UART_CLEAR_FLAG(pRS->huart, UART_FLAG_TC);		
			
			// Confirm TC is off
			while(!(USARTx->SR & USART_SR_TXE)){		
				if(--cnt_clear_TC == 0) break;
			}
			
			// transmit byte
			USARTx->DR = pRS->txBuff[pRS->cnt_byte_sent];
			
			isSending = 1;
		}	
		return result;
	}



/**********************************************************************************************************************************/
/*******END PAGE********/
