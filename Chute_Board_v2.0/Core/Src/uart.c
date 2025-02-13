/*
 * uart.h version 2
 * Created on: 4-Sep-2024
 * Author: Le Huu An
 */


#include "uart.h"
#include <string.h>

#include "user.h"
#include "flash_f103c8t6.h"

/*
BRIEF NOTE: 

*/
/**********************************************************************************************************************************/

/******EXTERN*******/
	extern UART_HandleTypeDef huart1;


/**********************************************************************************************************************************/
/*******DECLARE VARIABLE********/
	uart_t vUART_DB;	


/**********************************************************************************************************************************/
/*******FUNCTION********/
/*______________________________________________________________________________*/
	void UART_Init(uart_t *pRS, UART_HandleTypeDef *huart)
	{
		pRS->huart = huart;
		
		pRS->RxFlag = 0;		
		pRS->rxByte = 0; 
		pRS->rxPointer = 0;
		pRS->STATE = USART_WAIT;
		pRS->cmd_type = 0x00;
		
		pRS->cnt_mess_received = 0;
		pRS->cnt_mess_wrong = 0;
		pRS->cnt_mess_sent = 0;

		pRS->header = 0xAB;
		pRS->ender = 0xCD;
		
		memset(pRS->rxBuff, 0x00, UART_NUM_BYTE);
		memset(pRS->txBuff, 0x00, UART_NUM_BYTE);
	
		HAL_UART_Receive_IT(pRS->huart, &pRS->rxByte, 1);		
	}
	
/*______________________________________________________________________________*/
	void UART_Reset_Rx(uart_t *pUART)
	{	
		pUART->RxFlag = 0;		
		pUART->rxByte = 0; 
		pUART->rxPointer = 0;
		pUART->STATE = USART_WAIT;

		memset(pUART->rxBuff, 0x00, UART_NUM_BYTE);
	
		HAL_UART_Receive_IT(pUART->huart, &pUART->rxByte, 1);		
	}
/*______________________________________________________________________________*/
	void UART_Read(uart_t *pRx)			 //Read Data from RS485
	{	
		
	 //Process receive data 
		switch(pRx->STATE) 
		{
		//-------------------------------------------------------------------------------------------
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
				if(pRx->rxPointer == UART_NUM_BYTE-1)              //Check the last byte
				{    
					if(pRx->rxBuff[pRx->rxPointer] == pRx->ender){    //Success -> Turn on the Flag and process data			  
	
						pRx->RxFlag = 1;  
						pRx->cnt_mess_received++;		
						HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET);
					}
					else{ 										//Fail -> Reset buffer, pointer and move to WAIT state                 
						pRx->rxPointer = 0;     
						pRx->RxFlag = 0;
						memset(pRx->rxBuff, 0x00, UART_NUM_BYTE); 
						pRx->cnt_mess_wrong++;
						pRx->STATE = USART_WAIT;						
					}  
				}
				else{
					pRx->rxPointer = pRx->rxPointer + 1;  			//Increase the pointer    
					if(pRx->rxPointer > UART_NUM_BYTE-1){           //Fail
						pRx->rxPointer = 0;
						pRx->RxFlag = 0;     
						memset(pRx->rxBuff, 0x00, UART_NUM_BYTE);
						pRx->STATE = USART_WAIT;
					}   
				}   
			 break;  
			} 
		//-------------------------------------------------------------------------------------------	
		}
	}
	

/*______________________________________________________________________________*/ 
	uint8_t Uart_Rx_Handle(uart_t *pUART)
	{
		if(pUART->RxFlag == 1){

		}
		return true;
	}
	
/**********************************************************************************************************************************/
/*******END PAGE********/


