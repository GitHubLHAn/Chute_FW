/*
 * rs485.h version 2
 * Created on: 25-June-2024
 * Author: Le Huu An
 */


#include "rs485.h"
#include <string.h>

#include "user.h"

/*
BRIEF NOTE: Ver2 bo sung ham send uart theo tung byte

		+ Nhan ngat UART (Put in CODE BEGIN 0)
					void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {  
						if(huart->Instance == USART1) 
						{    
							RS485_Read(&vRS485);
							HAL_UART_Receive_IT(&huart1, &vRS485.rxByte, 1); 			
						}
					}
					
		+ Put in CODE BEGIN 2
			HAL_UART_Receive_IT(&huart3, &vRS485.rxByte, 1);
					
		+ RX (put in While): ham nay thuc hien khi co Rx dc set
		
			if(vRS485.RxFlag == 1){   
				HAL_GPIO_WritePin(DE_RS485_GPIO_Port, DE_RS485_Pin, GPIO_PIN_SET);
				vRS485.RxFlag = 0;   
				
				//Xu ly data nhan duoc o day

				//for(uint16_t index=0; index<1000; index++);			//delay with for 200us
				
				HAL_UART_Transmit(&huart1, vRS485.txBuff, SIZE_RS485, HAL_MAX_DELAY);
				HAL_GPIO_WritePin(DE_RS485_GPIO_Port, DE_RS485_Pin, GPIO_PIN_RESET);  
				vRS485.STATE = USART_WAIT;
			}	

		+ TX (put in While):	ham nay thuc hien khi co Tx dc set
		
			if(vRS485.TxFlag == 1){
				RS485_SendData(&vRS485, USART1, 2000);
			}
	
*/
/**********************************************************************************************************************************/

/******EXTERN*******/
extern UART_HandleTypeDef huart1;


/**********************************************************************************************************************************/
/*******DECLARE VARIABLE********/
rs485_t vRS485;		// variable RS485


/**********************************************************************************************************************************/
/*******FUNCTION********/
/*______________________________________________________________________________*/
	void RS485_Init(rs485_t *pRS, UART_HandleTypeDef *huart){
		pRS->huart = huart;
		__HAL_UART_CLEAR_FLAG(pRS->huart, UART_FLAG_TC);
		
		pRS->RxFlag = 0;		
		pRS->rxByte = 0; 
		pRS->rxPointer = 0;
		pRS->STATE = USART_WAIT;
		pRS->cmd_type = 0x00;
		pRS->cnt_mess_received = 0;
		pRS->cnt_mess_wrong = 0;
		
		pRS->TxFlag = 0;
		pRS->cnt_mess_sent = 0;
		pRS->cnt_byte_sent = 0;
		pRS->header = 0xAB;
		pRS->ender = 0xCD;
		memset(pRS->rxBuff, 0x00, RS485_NUM_BYTE);
		memset(pRS->txBuff, 0x00, RS485_NUM_BYTE);
		
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
		
		pRS->ID_Board = (pRS->ID_Byte_H << 8) | pRS->ID_Byte_L;
		
	}
/*______________________________________________________________________________*/
	void RS485_Read(rs485_t *pRx){			 //Read Data from RS485
	 //Process receive data 
		switch(pRx->STATE) 
		{
			case  USART_WAIT:                  //Wait for start of new message   
			{
				if(pRx->rxByte == pRx->header){       //check header
					pRx->rxBuff[0] = pRx->rxByte;
					pRx->STATE = USART_READING;      //switch to Reading state    
					pRx->rxPointer = 1;
				}   
			break;
			}  
			//-------------------------------------------------------------------------------------------
			case USART_READING:  
			{
				pRx->rxBuff[pRx->rxPointer] = pRx->rxByte; 			//Read data   
				if(pRx->rxPointer == RS485_NUM_BYTE-1)              //Check the last byte
				{    
					if(pRx->rxBuff[pRx->rxPointer] == pRx->ender && pRx->rxBuff[1] == pRx->ID_Byte_H  & pRx->rxBuff[2] == pRx->ID_Byte_L){    //Success -> Turn on the Flag and process data			  
					//if(pRx->rxBuff[pRx->rxPointer] == pRx->ender){	
						pRx->RxFlag = 1;  
						pRx->cnt_mess_received++;		
						HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET);
					}
					else{ 										//Fail -> Reset buffer, pointer and move to WAIT state                 
						pRx->rxPointer = 0;     
						pRx->RxFlag = 0;
						memset(pRx->rxBuff, 0x00, RS485_NUM_BYTE); 
						pRx->cnt_mess_wrong++;
						pRx->STATE = USART_WAIT;						
					}  
				}
				else{
					pRx->rxPointer = pRx->rxPointer + 1;  			//Increase the pointer    
					if(pRx->rxPointer > RS485_NUM_BYTE-1){           //Fail
						pRx->rxPointer = 0;
						pRx->RxFlag = 0;     
						memset(pRx->rxBuff, 0x00, RS485_NUM_BYTE);
						pRx->STATE = USART_WAIT;
					}   
				}   
			 break;  
			} 
		}
	}
	
/*______________________________________________________________________________*/
	static uint8_t isSending = 0;
	uint8_t TC = 0;
	
	bool RS485_SendData(rs485_t *pRS, USART_TypeDef * USARTx, uint16_t timeout){
		uint8_t cnt_clear_TC = 10;
		bool result = false;
		TC = USARTx->SR & USART_SR_TC;
		
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
	
/*______________________________________________________________________________*/ 
	uint8_t UartRx_Handle(rs485_t *pRS){
		if(pRS->RxFlag == 1){
			flag_flashLed = 1;
			
			/*Do command from App*/
			count_packet = (pRS->rxBuff[3] << 8) | pRS->rxBuff[4];
			if(chute_status == CHUTE_CHOT && pRS->rxBuff[5] == CHUTE_NORMAL)
				button_Action = 0;
			chute_status = pRS->rxBuff[5]; 
			status_thap_led = pRS->rxBuff[6];
			status_thap_coi = pRS->rxBuff[7];
			
			/*Response information to App*/
			pRS->txBuff[0] = pRS->header;
			pRS->txBuff[1] = pRS->ID_Byte_H;
			pRS->txBuff[2] = pRS->ID_Byte_L;
			pRS->txBuff[3] = chute_isFull;
			pRS->txBuff[4] = button_Action;	//button_Action = 0x00;
			pRS->txBuff[5] = 0xFF;
			pRS->txBuff[6] = 0xFF;
			pRS->txBuff[7] = 0xFF;
			pRS->txBuff[8] = 0xFF;
			pRS->txBuff[9] = pRS->ender;
			
			pRS->TxFlag = 1;
			flag_flashLed = 1;
			pRS->RxFlag = 0;
		}
		return true;
	}
	
/*______________________________________________________________________________*/
	uint8_t UartTx_Transmit(rs485_t *pRS){
//		if(pRS->TxFlag == 1){
//			if(RS485_SendData(&vRS485, USART1, 2000))	
//				HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);	
//		}
		if(pRS->TxFlag == 1){
			HAL_UART_Transmit(&huart1, pRS->txBuff, RS485_NUM_BYTE, HAL_MAX_DELAY);
			
			HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);
			pRS->cnt_mess_sent++;			
			pRS->TxFlag = 0;
		}
		return 0;
	}
	
	
	
	
/*______________________________________________________________________________*/




/*______________________________________________________________________________*/




