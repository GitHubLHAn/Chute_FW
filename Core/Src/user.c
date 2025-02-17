/*
 * user.c
 * Created on: 18-DEC-2025
 * Author: Le Huu An (anlh55@viettel.com.vn)
 * This version of chute firmware is upgraded with new system of
 * chute with 2 device (master and slave, 1 master manages 32 slave).
 */
 
 
#include "user.h"


#include "app.h"

#include "slave.h"

#include <string.h>
#include <stdio.h>

/*
NOTE: 


*/
/**********************************************************************************************************************************/
/******EXTERN*******/
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;

/**********************************************************************************************************************************/
/*******DECLARE VARIABLE********/
	
	uint8_t ID_Master = 0;
	
	UART_HandleTypeDef *pSlavePort;
	UART_HandleTypeDef *pDebugPort;
	
	//uint8_t vStateMaster = MASTER_READY;
	
	
	volatile uint32_t start_time = 0, interval = 0;
	volatile uint32_t _s = 0, _i = 0;
	
	uint8_t Full_check_arr[NUMBER_SLAVE];
	
	uint8_t mode_master = 0;
	uint8_t ptr_config = 0;
	
	uint8_t TX_config[20];
	
/**********************************************************************************************************************************/
/*******FUNCTION********/

//================================================================================
//	Initate master information															
//================================================================================
	
	void Master_Init_Func(void)
 	{
		uint8_t get_port_slave = 0;
		
		ID_Master |= !HAL_GPIO_ReadPin(BIT0_GPIO_Port, BIT0_Pin) << 0;
		ID_Master |= !HAL_GPIO_ReadPin(BIT1_GPIO_Port, BIT1_Pin) << 1;
		ID_Master |= !HAL_GPIO_ReadPin(BIT2_GPIO_Port, BIT2_Pin) << 2;
		ID_Master |= !HAL_GPIO_ReadPin(BIT3_GPIO_Port, BIT3_Pin) << 3;
		ID_Master |= !HAL_GPIO_ReadPin(BIT4_GPIO_Port, BIT4_Pin) << 4;
		ID_Master |= !HAL_GPIO_ReadPin(BIT5_GPIO_Port, BIT5_Pin) << 5;
		
		get_port_slave = HAL_GPIO_ReadPin(BIT6_GPIO_Port, BIT6_Pin);
		if(get_port_slave){
			pSlavePort = &huart2;
			pDebugPort = &huart3;
		}
		else{
			pSlavePort = &huart3;
			pDebugPort = &huart2;
		}
		memset(Full_check_arr, 0, NUMBER_SLAVE);
	}

//================================================================================
//	Get current microsecond time tick	
//================================================================================
	volatile uint32_t tim_tick_1ms = 0;
	
	uint32_t getMicroSecond(void)
 	{
		return tim_tick_1ms*1000 + __HAL_TIM_GetCounter(&htim1);
	}

//================================================================================
//	Calculate Checksum	
//================================================================================
		
	uint8_t Cal_CheckSum(uint8_t *Buff, uint8_t length)
	{
		uint8_t result = 0x00;
		
		for(uint8_t i=0; i<length-1; i++)
			result += Buff[i];
		
		return result;
	}
	
//================================================================================
//	Flashing led 	
//================================================================================
	volatile uint16_t cnt_flashLed_debug = 0;
	volatile bool flag_flashLed_debug = 0;
	
	volatile uint16_t cnt_flashLed_app = 0, cnt_flashLed_trxA = 0, cnt_flashLed_trxB = 0;
	volatile bool flag_flashLed_app = 0, flag_flashLed_trxA = 0, flag_flashLed_trxB = 0;
	
	void Flash_Led(void)
	{
		/*Flash led debug*/
		if(flag_flashLed_debug)
		{
			ON_LED_DEBUG();
			if(++cnt_flashLed_debug >= 10)
			{
				OFF_LED_DEBUG();
				cnt_flashLed_debug = 0;
				flag_flashLed_debug = 0;
			}
		}
		
		/*Flag led transceiver App*/
		if(flag_flashLed_app)
		{
			ON_LED_APP();
			if(++cnt_flashLed_app >= 10)
			{
				OFF_LED_APP();
				cnt_flashLed_app = 0;
				flag_flashLed_app = 0;
			}
		}
		
		/*Flag led transceiver Slave Line A*/
		if(flag_flashLed_trxA)
		{
			ON_LED_TRX_A();
			if(++cnt_flashLed_trxA >= 5)
			{
				OFF_LED_TRX_A();
				cnt_flashLed_trxA = 0;
				flag_flashLed_trxA = 0;
			}
		}
		
		/*Flag led transceiver Slave Line A*/
		if(flag_flashLed_trxB)
		{
			ON_LED_TRX_B();
			if(++cnt_flashLed_trxB >= 5)
			{
				OFF_LED_TRX_B();
				cnt_flashLed_trxB = 0;
				flag_flashLed_trxB = 0;
			}
		}
	}

//================================================================================
//	Blink the debug led															
//================================================================================
	volatile uint16_t cnt_toggle_led_db = 0;
	
	void BlinkLed_Debug(void)
	{
		if(cnt_toggle_led_db >= 300)
		{
			HAL_GPIO_TogglePin(LED_DEBUG_GPIO_Port, LED_DEBUG_Pin);
			cnt_toggle_led_db = 0;	
		}
	}
	
//================================================================================
//	Send Command to Slave periodically															
//================================================================================
	volatile uint16_t cnt_send_slave = 0;
	bool flag_synchronize_slave = true;
	volatile uint32_t cnt_synchronize_slave = 0;
	
	uint8_t Send_Slave_Period(void)
	{	
		if(flag_synchronize_slave)
		{
			On_Mode_Send_Slave();
			uint8_t SYNC_arr[2] = {0xEF, 0xAE};
			Send_Slave_Sync(SYNC_arr);	
			flag_synchronize_slave = false;
			return 0;
		}

		if(cnt_send_slave >= CYCLE_SEND_SLAVE)
		{
			SendCMD_Slave();
			cnt_send_slave = 0;
		}	
		return 1;
	}
	
//================================================================================
//	Check connection slave											
//================================================================================	
	volatile uint16_t cnt_check_connect_slave = 0;
	
	void Check_connection_Slave(uint8_t _cycle)
	{
		/*Check every 5s*/
		if(++cnt_check_connect_slave >= _cycle)
		{
			for(uint8_t i=0; i<NUMBER_SLAVE; i++)
			{
				if(List_Chute_Arr[i].sl_connected && List_Chute_Arr[i].sl_enable)
				{
					List_Chute_Arr[i].cnt_disconnect += _cycle;
					if(List_Chute_Arr[i].cnt_disconnect >= TIME_CHECK_CONNECT_SL)
						List_Chute_Arr[i].sl_connected = false;			
				}
				
//				if(List_Chute_Arr[i].cnt_disconnect < TIME_CHECK_CONNECT_SL)
//				{
//					List_Chute_Arr[i].sl_connected = true;
//					List_Chute_Arr[i].cnt_disconnect += _cycle;
//				}
//				
//				if(List_Chute_Arr[i].cnt_disconnect >= TIME_CHECK_CONNECT_SL)
//					List_Chute_Arr[i].sl_connected = false;					
			}
			cnt_check_connect_slave = 0;
		}
	}

	
//================================================================================
//	Hanlde status of Lamp											
//================================================================================
	volatile uint16_t cnt_check_full = 0;
	volatile uint8_t flag_check_button = 0;
	uint8_t sum_check = 0;
	bool flag_full = 0;
	
	void Handle_Lamp(void)
	{
		flag_full = false;
		if(cnt_check_full >= 50)
		{
			sum_check = 0;
			
			for(uint8_t i=0; i<NUMBER_SLAVE; i++)
				sum_check += List_Chute_Arr[i].sl_isFull;
		
			if(sum_check != 0)
				flag_full = true;
			
			if(flag_full)
			{
				ON_LAMP_RED();
				ON_LAMP_GREEN();
				ON_LAMP_YELLOW();	
				//ON_SPEAKER();	
			}
			else{
				OFF_LAMP_RED();
				OFF_LAMP_GREEN();
				OFF_LAMP_YELLOW();	
				OFF_SPEAKER();	
			}
		
			cnt_check_full = 0;
		}
		
		if(flag_check_button == 1)
		{
			for(uint8_t j=0; j<NUMBER_SLAVE; j++)
			{
				if(List_Chute_Arr[j].sl_flag_button == 1)
				{
					if(List_Chute_Arr[j].sl_status == 0x00)
						List_Chute_Arr[j].sl_status = 0x40;
					else if(List_Chute_Arr[j].sl_status == 0x40)
						List_Chute_Arr[j].sl_status = 0x00;
				
					List_Chute_Arr[j].sl_flag_button = 0;
				}
			
			}
			
			flag_check_button = 0;
		}
	}
	
//================================================================================
//	Hanlde status of Speaker										
//================================================================================
	
	void Handle_Speaker(void)
	{

		
	}
	
//================================================================================
//	Master State Machine function														
//================================================================================
	volatile uint16_t cnt_flag_config = 0;
	uint32_t start_wait = 0;
	
	void Master_Process(void)
	{
		if(mode_master == 0)
		{
			ON_LED_TRX_A();
			ON_LED_TRX_B();
			ON_LAMP_RED();
			ON_LAMP_GREEN();
			ON_LAMP_YELLOW();	
			if(cnt_flag_config >= 500)
			{		
				ON_LAMP_RED();
				ON_LAMP_GREEN();
				ON_LAMP_YELLOW();
				if(++ptr_config > NUMBER_SLAVE)
				{
					mode_master = 1;
					start_wait = getMicroSecond();
					ON_LED_TRX_A();
					ON_LED_TRX_B();
				}
				else
				{
					On_Mode_Send_Slave();
					uint8_t addr_arr[15];
					memset(addr_arr, ' ',15); 
					sprintf((char*)addr_arr, "Dong chi so 0%d", ptr_config);
					
					TX_config[0] = 0x80 | (ptr_config & 0x3F);
					for(uint8_t i=0; i<15; i++)
						TX_config[i+1] = addr_arr[i];
					TX_config[16] = 0xA6;
					TX_config[17] = 0xA0;
					TX_config[18] = 0x80;
					TX_config[19] = Cal_CheckSum(TX_config, 20);
					
					HAL_UART_Transmit(List_Chute_Arr[ptr_config].phuart, TX_config, 20, HAL_MAX_DELAY);
					
					
					On_Mode_Receive_Slave();
				}
				
				cnt_flag_config = 0;
			}
		}
		
		if(mode_master == 1)
		{
			uint32_t interval_wait = getMicroSecond() - start_wait;
			
			if(interval_wait > 2000000)		// after 3s
			{
				mode_master = 2;
				OFF_LED_TRX_A();
				OFF_LED_TRX_B();
				OFF_LAMP_RED();
				OFF_LAMP_GREEN();
				OFF_LAMP_YELLOW();	
			}
		}

		if(mode_master == 2)
		{
			
			Send_Slave_Period();
			Handle_Lamp();
		}

	}
	
	
	
	
//================================================================================
//	Timer Callback function														
//================================================================================	

	volatile uint16_t cnt_debug = 0;
	extern volatile uint16_t auto_send;
	
	
	void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
	{
		UNUSED(htim);
			
		//---------------------------------------------------------
			if (htim->Instance == htim1.Instance) 	// every 1ms		
			{
				/*Increase cnt variable*/	
				tim_tick_1ms++;				
				cnt_toggle_led_db++;
				cnt_send_slave++;
				cnt_debug++;
				auto_send++;
				cnt_check_full++;
				cnt_flag_config++;
					
				/*Handle function*/
				Flash_Led();
				Handle_Lamp();
				Handle_Speaker();	
			}

		//---------------------------------------------------------
			if (htim->Instance == htim2.Instance) 	// every 1s		
			{
				/*Increase cnt variable*/
				if(!flag_synchronize_slave)
				{
					if(++cnt_synchronize_slave == 60)
						cnt_synchronize_slave = 0;
					if(cnt_synchronize_slave%30 ==0)
					{
						flag_synchronize_slave = true;
						//cnt_synchronize_slave = 0;
					}
				}

				/*Handle function*/
				Check_connection_Slave(5);	// check every 5s
			}			
		//---------------------------------------------------------
	}	
	
//================================================================================
//	Debug function														
//================================================================================	
	
	void Debug_Function(void)
	{
	//---------------------------------------------------------
//		if(cnt_debug >= 100)
//		{
////			flag_flashLed_debug = true;
////			flag_flashLed_app = true;
////			flag_flashLed_trxA = true;
////			flag_flashLed_trxB = true;
//			//HAL_GPIO_TogglePin(LED_DEBUG_GPIO_Port, LED_DEBUG_Pin);
//			//HAL_UART_Transmit_DMA(&huart2, (uint8_t*)"Hello\n", 6);	
//			cnt_debug = 0;
//			_i = getMicroSecond() - _s;
//			_s = getMicroSecond();
//		}
	//---------------------------------------------------------


	//---------------------------------------------------------
			
	//---------------------------------------------------------
	}	

/**********************************************************************************************************************************/
/*******END PAGE********/



