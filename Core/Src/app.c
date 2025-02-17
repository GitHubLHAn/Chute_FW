/*
 * app.c
 * Created on: 15-Jan-2025
 * Author: Le Huu An
 */


#include "app.h"
#include <string.h>

#include "user.h"

#include "slave.h"

/*
BRIEF NOTE: 
	
	
*/
/**********************************************************************************************************************************/
/******EXTERN*******/
	extern UART_HandleTypeDef huart1;
	
	extern UART_HandleTypeDef huart2;
	extern UART_HandleTypeDef huart3;


/**********************************************************************************************************************************/
/*******DECLARE VARIABLE********/
	app_com_t vApp;
	
	uint8_t TX_toApp[NUM_BYTE_APP];
	uint8_t RX_fromApp[NUM_BYTE_APP];


/**********************************************************************************************************************************/
/*******FUNCTION********/


/*========================================================================================*\
 *																		INIT RS485																					*
\*========================================================================================*/

	void App_Init_Func(app_com_t *pApp, UART_HandleTypeDef *huart)
	{
		pApp->huart = huart;
		
		pApp->RxFlag = 0;		
		pApp->rxByte = 0; 	
		pApp->STATE = READ_HD_APP;
		
		pApp->cnt_mess_received = 0;
		pApp->cnt_mess_wrong = 0;
		pApp->cnt_mess_sent = 0;
		
		memset(TX_toApp, 0xDA, NUM_BYTE_APP);
		memset(RX_fromApp, 0xAD, NUM_BYTE_APP);
	
		MODE_RECEIVE_APP();
		HAL_UART_Receive_IT(pApp->huart, &pApp->rxByte, 1);
		
	}
	

/*========================================================================================*\
 *															READ DATA FROM APP																			*
\*========================================================================================*/
	void Read_App(app_com_t *pAPP)
	{
		uint8_t get_sequence = 0x00;
		
		switch(pAPP->STATE)
		{
		//---------------------
			case READ_HD_APP:
			{
				get_sequence = pAPP->rxByte & 0x3F;
				
				
				if(get_sequence == ID_Master)
				{
					RX_fromApp[0] = pAPP->rxByte;
					pAPP->STATE = READ_DT_APP;
				}		
				break;
			}
		//---------------------
			case READ_DT_APP:
			{
				RX_fromApp[1] = pAPP->rxByte;
				pAPP->STATE = READ_CS_APP;
				break;
			}
		//---------------------
			case READ_CS_APP:
			{
				RX_fromApp[2] = pAPP->rxByte;
				uint8_t check_CS = 0;
				check_CS = Cal_CheckSum(RX_fromApp, 3);
				if(check_CS == RX_fromApp[2])
				{
					//pCOM->RxFlag = 1;	
					On_Mode_Send_Slave();
					flag_flashLed_app = 1;
						

					pAPP->cnt_mess_received++;
				}
				else
				{
					memset(RX_fromApp, 0xEE, NUM_BYTE_APP);
					pAPP->cnt_mess_wrong++;
				}
				pAPP->STATE = READ_HD_APP;
				break;
			}	
		}
		
		HAL_UART_Receive_IT(pAPP->huart, &pAPP->rxByte, 1);
	}	


/**********************************************************************************************************************************/
/*******END PAGE********/
