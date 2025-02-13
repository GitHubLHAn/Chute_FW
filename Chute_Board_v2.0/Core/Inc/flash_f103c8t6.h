/*
 * flash_f103c8t6.c
 * Created on: 23-Sep-2024
 * Author: Le Huu An
 * Used for Chute Board
 */

#ifndef __FLASH_F103C8T6_H
#define __FLASH_F103C8T6_H
 
#include "main.h"

#include <string.h>
#include "user.h"

/*DEFINE*/
	#define ADDRES_DATA_STORAGE 0x800FC00
	
	#define UPDATE_ERROR 0x4E
	#define UPDATE_SUCCESS 0x59
	
	#define PASSWORD_UPDATE 0xDA

/************************************************************************************/
/*DECLARE STRUCT*/

	#pragma pack(1)
	typedef struct{
		uint8_t pw;
		uint8_t address_arr[12];
		uint8_t mode_sleep;
	}chute_info_t;
	#pragma pack()


/*DECLARE FUNCTION*/
	
	void Flash_Erase(uint32_t address);

	void Flash_Write_Int(uint32_t address, int value);
	
	void Flash_Write_Float(uint32_t address, float f);
	
	void Flash_Write_Array(uint32_t address, uint8_t *arr, uint16_t len);
	
	void Flash_Write_Struct(uint32_t address, chute_info_t data);

	int Flash_Read_Int(uint32_t address);
	
	float Flash_Read_Float(uint32_t address);
	
	void Flash_Read_Array(uint32_t address, uint8_t *arr, uint16_t len);
	
	void Flash_Read_Struct(uint32_t address, chute_info_t *data);

	void Get_Address_Chute(void);
	
	uint8_t Update_Addr_Chute(void);
	
	void Init_Chute_Infor(chute_info_t *infor);


/*EXTERN*/

	extern chute_info_t vChute_Infor_default;
	extern chute_info_t vChute_Infor_temp;	
	

/************************************************************************************/
#endif /* FLASH_F103C8T6_H*/