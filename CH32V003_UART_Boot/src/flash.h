/********************************** (C) COPYRIGHT  *******************************
* File Name          : falsh.h
* Author             : WCH
* Version            : V1.0.0
* Date               : 2022/11/21
* Description        :
*******************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#ifndef __FLASH_H
#define __FLASH_H

#include <inttypes.h>

void CH32_IAP_Program(uint32_t addr, const uint32_t* buf);
//uint8_t CH32_IAP_Verity(uint32_t addr, const uint32_t* buf);
//void CH32_IAP_Erase(uint32_t start_addr, uint32_t end_addr);

#endif
