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
	chute_info_t vChute_Infor_temp;	

/**********************************************************************************************************************************/
/*******FUNCTION********/

//================================================================================*/
// Initiate information of chute
//================================================================================*/

	void Init_Chute_Infor(chute_info_t *infor)
	{
		infor->pw = PASSWORD_UPDATE;
		memset(infor->address_arr, 0x00, 12);
		infor->address_arr[0] = '*';
		infor->address_arr[1] = '*';
		infor->address_arr[2] = '*';
		
		infor->mode_sleep = 0;
	
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
		
		Flash_Read_Struct(ADDRES_DATA_STORAGE, &vChute_Infor_temp);
		if(vChute_Infor_temp.pw == PASSWORD_UPDATE)
		{
			for(uint8_t i=4; i<16; i++)
				Chute_Addr_Arr[i] = vChute_Infor_temp.address_arr[i-4];
		}
		else
		{			
			Flash_Erase(ADDRES_DATA_STORAGE);
			HAL_Delay(50);
			Flash_Write_Struct(ADDRES_DATA_STORAGE, vChute_Infor_default);
			
			Flash_Read_Struct(ADDRES_DATA_STORAGE, &vChute_Infor_temp);
			for(uint8_t i=4; i<16; i++)
				Chute_Addr_Arr[i] = vChute_Infor_temp.address_arr[i-4];
		
			ON_DEN_BAO(); HAL_Delay(300);
			OFF_DEN_BAO(); HAL_Delay(300);
			ON_DEN_BAO(); HAL_Delay(300);
			OFF_DEN_BAO(); HAL_Delay(300);
			ON_DEN_BAO(); HAL_Delay(300);
			OFF_DEN_BAO(); 
		}
	}
	
/*================================================================================*/
/*												UPDATE NEW ADDRESS INTO FLASH														*/
/*================================================================================*/

	uint8_t Update_Addr_Chute(void)
	{
		uint8_t result = UPDATE_ERROR;

		Flash_Erase(ADDRES_DATA_STORAGE);
		HAL_Delay(10);
		
		/*Copy address array update into struct vChute_Infor_temp*/
		memcpy(vChute_Infor_temp.address_arr,	Chute_Infor_update_arr, 12);
		
		/*Write struct to Flash*/
		Flash_Write_Struct(ADDRES_DATA_STORAGE, vChute_Infor_temp);
		HAL_Delay(1);
		/*Read and check again*/
		Flash_Read_Struct(ADDRES_DATA_STORAGE, &vChute_Infor_temp);
		if(memcmp(vChute_Infor_temp.address_arr, Chute_Infor_update_arr, 12) == 0)
			result = UPDATE_SUCCESS;
	
		return result;
	}
	
	

/**********************************************************************************************************************************/
/*******END PAGE********/
