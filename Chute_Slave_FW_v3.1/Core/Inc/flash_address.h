/*
 * flash_address.c
 * Created on: 23-Sep-2024
 * Author: Le Huu An
 * Used for Chute Board
 */

#ifndef __FLASH_ADDRESS_H
#define __FLASH_ADDRESS_H
 
#include "main.h"

#include <string.h>
#include "user.h"

/*DEFINE*/
	#define ADDRES_DATA_STORAGE 0x800FC00
	
	#define UPDATE_ERROR 0x4E
	#define UPDATE_SUCCESS 0x59
	
	#define PASSWORD_UPDATE 0x12

	#define NUM_IDENTIFICATION 		15
/************************************************************************************/
/*DECLARE STRUCT*/

	#pragma pack(1)
	typedef struct{
		uint8_t pw;
		uint8_t identification_arr[NUM_IDENTIFICATION];
		uint16_t cycle_lcd;
		uint8_t time_check_connection;
		uint8_t mode_baudrate;
	}chute_info_t;
	#pragma pack()


/*DECLARE FUNCTION*/
	
	void Flash_Erase(uint32_t address);
	
	void Flash_Write_Array(uint32_t address, uint8_t *arr, uint16_t len);
	
	void Flash_Write_Struct(uint32_t address, chute_info_t data);
	
	void Flash_Read_Array(uint32_t address, uint8_t *arr, uint16_t len);
	
	void Flash_Read_Struct(uint32_t address, chute_info_t *data);

	void Load_Infor_Chute_Func(void);
	
	uint8_t Update_NEW_Infor(void);
	
	void Init_Chute_Infor(chute_info_t *infor);


/*EXTERN*/

	extern chute_info_t vChute_Infor_default;
	extern chute_info_t vChute_Infor_cache;	
	
	extern uint8_t Get_New_ID_arr[NUM_IDENTIFICATION];
	extern uint8_t Get_New_mode;
	

/************************************************************************************/
#endif /* FLASH_ADDRESS_H*/


