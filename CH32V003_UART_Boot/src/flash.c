/********************************** (C) COPYRIGHT  *******************************
* File Name          : iap.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2022/11/21
* Description        : CH32V003  fast program
*******************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include <ch32v00x_flash.h>
#include "flash.h"

/*********************************************************************
 * @fn      CH32_IAP_Program
 *
 * @brief   addr - 64Byte stand
 *          buf - 64Byte stand
 *
 * @return  none
 */
void CH32_IAP_Program(uint32_t addr, const uint32_t* buf) {
    addr &= 0xFFFFFFC0;
    FLASH_BufReset();
    for (uint8_t i = 0; i < 16; ++i) {
        FLASH_BufLoad(addr + i * 4, buf[i]);
    }
    FLASH_ProgramPage_Fast(addr);
}
