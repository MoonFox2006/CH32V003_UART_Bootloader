/********************************** (C) COPYRIGHT *******************************
 * File Name          : system_ch32v00x.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/08/08
 * Description        : CH32V00x Device Peripheral Access Layer System Source File.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include <ch32v00x.h>

/*********************************************************************
 * @fn      SystemInit
 *
 * @brief   Setup the microcontroller system Initialize the Embedded Flash Interface,
 *        the PLL and update the SystemCoreClock variable.
 *
 * @return  none
 */
void SystemInit(void) {
    RCC->CTLR |= 0x00000001;
    RCC->CFGR0 &= 0xFCFF0000;
    RCC->CTLR &= 0xFEF6FFFF;
    RCC->CTLR &= 0xFFFBFFFF;
    RCC->CFGR0 &= 0xFFFEFFFF;
    RCC->INTR = 0x009F0000;

    /* Flash 0 wait state */
    FLASH->ACTLR &= ~(uint32_t)FLASH_ACTLR_LATENCY;
//    FLASH->ACTLR |= (uint32_t)FLASH_ACTLR_LATENCY_0;

    /* HCLK = SYSCLK = APB1 */
//    RCC->CFGR0 |= RCC_HPRE_DIV1;
}
