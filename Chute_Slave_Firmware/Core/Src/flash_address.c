/*
 * flash_f103c8t6.c
 * Created on: 23-Sep-2024
 * Author: Le Huu An
 * Used for Chute Board
 */

#include "flash_address.h"

/**********************************************************************************************************************************/
/*******DECLARE VARIABLE********/
	chute_info_t vChute_Infor_default;
	chute_info_t vChute_Infor_cache;	
	
	uint8_t Get_New_ID_arr[NUM_IDENTIFICATION];
	uint8_t Get_New_mode;

/**********************************************************************************************************************************/
/*******FUNCTION********/

//================================================================================*/
// Initiate information of chute
//================================================================================*/

	void Init_Chute_Infor(chute_info_t *infor)
	{
		infor->pw = PASSWORD_UPDATE;
		memset(infor->identification_arr, '*', NUM_IDENTIFICATION);
//		infor->identification_arr[0] = 'N';
//		infor->identification_arr[1] = '*';
//		infor->identification_arr[2] = '*';
		
		infor->cycle_lcd = CYCLE_LCD_1000ms;
		infor->time_check_connection = TIME_CHECK_CONNECTION_30s;
		infor->mode_baudrate = 1;
	}
	
//================================================================================*/
// Flash erase a sector
//================================================================================*/

	void Flash_Erase(uint32_t address)
	{
		HAL_FLASH_Unlock();
		FLASH_EraseInitTypeDef EraseInitStruct;
		EraseInitStruct.Banks = 1;
		EraseInitStruct.NbPages = 1;
		EraseInitStruct.PageAddress = address;
		EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
		uint32_t page_err;
		HAL_FLASHEx_Erase(&EraseInitStruct, &page_err);
		HAL_FLASH_Lock();
	}

//================================================================================*/
// Write an array into the flash
//================================================================================*/

	void Flash_Write_Array(uint32_t address, uint8_t *arr, uint16_t len)
	{
		uint16_t *pt = (uint16_t*)arr;
		HAL_FLASH_Unlock();
		for(uint8_t i=0; i<(len+1)/2; i++)
		{
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address + 2*i, *pt);
			pt++;
		}
		HAL_FLASH_Lock();

	}

//================================================================================*/
// Write a struct into the flash
//================================================================================*/

	void Flash_Write_Struct(uint32_t address, chute_info_t pDATA)
	{
		Flash_Write_Array(address, (uint8_t*)&pDATA, sizeof(pDATA));	
	}


//================================================================================*/
// Read an array from the flash
//================================================================================*/

	void Flash_Read_Array(uint32_t address, uint8_t *arr, uint16_t len)
	{
		uint16_t *pt = (uint16_t*)arr;
		for(uint8_t i=0; i<(len+1)/2; i++)
		{
			*pt = *(__IO uint16_t*)(address + 2*i);
			pt++;
		}

	}

//================================================================================*/
// Read a struct from the flash
//================================================================================*/

	void Flash_Read_Struct(uint32_t address, chute_info_t *pDATA)
	{
		Flash_Read_Array(address, (uint8_t*)pDATA, sizeof(chute_info_t));
	}

//================================================================================*/
// Read information of chute from flash if the password right
// Otherwise write the default information into the flash
//================================================================================*/

	void Load_Infor_Chute_Func(void)
	{
		Init_Chute_Infor(&vChute_Infor_default);
		
		Flash_Read_Struct(ADDRES_DATA_STORAGE, &vChute_Infor_cache);
		if(vChute_Infor_cache.pw == PASSWORD_UPDATE)
		{
			memset(Chute_Addr_Arr, 0x00, 16);
			for(uint8_t i=0; i<NUM_IDENTIFICATION; i++)
				Chute_Addr_Arr[i] = vChute_Infor_cache.identification_arr[i];
		}
		else		// no ever flash or change new version
		{			
			Flash_Erase(ADDRES_DATA_STORAGE);
			HAL_Delay(50);
			Flash_Write_Struct(ADDRES_DATA_STORAGE, vChute_Infor_default);
			
			Flash_Read_Struct(ADDRES_DATA_STORAGE, &vChute_Infor_cache);
			
			for(uint8_t i=0; i<16; i++)
				Chute_Addr_Arr[i] = 0;
			
			for(uint8_t j=0; j<NUM_IDENTIFICATION; j++)
				Chute_Addr_Arr[j] = vChute_Infor_cache.identification_arr[j];
		
			OFF_DEN_BAO();
			ON_DEN_BAO(); 	HAL_Delay(100);
			OFF_DEN_BAO(); 	HAL_Delay(500);
			ON_DEN_BAO(); 	HAL_Delay(100);
			OFF_DEN_BAO(); 	HAL_Delay(500);
			ON_DEN_BAO(); 	HAL_Delay(100);
			OFF_DEN_BAO(); 	HAL_Delay(500);
		}
		
		/*Upload mode*/
		vCycleLCD = vChute_Infor_cache.cycle_lcd;
		vTimeCheckCon = vChute_Infor_cache.time_check_connection;
	}
	
/*================================================================================*/
/*												UPDATE NEW ADDRESS INTO FLASH														*/
/*================================================================================*/

	uint8_t Update_NEW_Infor(void)
	{
		uint8_t result = UPDATE_ERROR;

		/*Clear Information in flash*/
		Flash_Erase(ADDRES_DATA_STORAGE);
		HAL_Delay(10);
		
		/*Update new cycle lcd*/
		if((Get_New_mode&0x03) == 0x00)
			vChute_Infor_cache.cycle_lcd = CYCLE_LCD_50ms;
		else if((Get_New_mode&0x03) == 0x01)
			vChute_Infor_cache.cycle_lcd = CYCLE_LCD_500ms;
		else if((Get_New_mode&0x03) == 0x02)
			vChute_Infor_cache.cycle_lcd = CYCLE_LCD_1000ms;
		else if((Get_New_mode&0x03) == 0x03)
			vChute_Infor_cache.cycle_lcd = CYCLE_LCD_5000ms;
		
		/*Update new time_check_connection*/
		if((Get_New_mode&0x0C) == 0x00)
			vChute_Infor_cache.time_check_connection = TIME_CHECK_CONNECTION_15s;
		else if((Get_New_mode&0x0C) == 0x04)
			vChute_Infor_cache.time_check_connection = TIME_CHECK_CONNECTION_30s;
		else if((Get_New_mode&0x0C) == 0x08)
			vChute_Infor_cache.time_check_connection = TIME_CHECK_CONNECTION_45s;
		else if((Get_New_mode&0x0C) == 0x0C)
			vChute_Infor_cache.time_check_connection = TIME_CHECK_CONNECTION_60s;
		
		/*Copy address array update new Identification*/
		memcpy(vChute_Infor_cache.identification_arr,	Get_New_ID_arr, NUM_IDENTIFICATION);
		
		/*Write struct to Flash*/
		Flash_Write_Struct(ADDRES_DATA_STORAGE, vChute_Infor_cache);
		HAL_Delay(1);
		/*Read and check again*/
		Flash_Read_Struct(ADDRES_DATA_STORAGE, &vChute_Infor_cache);
		HAL_Delay(1);
		
		if(memcmp(vChute_Infor_cache.identification_arr, Get_New_ID_arr, NUM_IDENTIFICATION) == 0)
			result = UPDATE_SUCCESS;
	
		return result;
	}
	
	

/**********************************************************************************************************************************/
/*******END PAGE********/
