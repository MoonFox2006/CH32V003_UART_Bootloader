/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch32v00x_it.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/08/08
 * Description        : Main Interrupt Service Routines.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include "ch32v00x_it.h"

/*********************************************************************
 * @fn      NMI_Handler
 *
 * @brief   This function handles NMI exception.
 *
 * @return  none
 */
void __attribute__((interrupt("WCH-Interrupt-fast"))) NMI_Handler(void) {}

/*********************************************************************
 * @fn      HardFault_Handler
 *
 * @brief   This function handles Hard Fault exception.
 *
 * @return  none
 */
void __attribute__((interrupt("WCH-Interrupt-fast"))) HardFault_Handler(void) {
    while (1) {}
}

void __attribute__((interrupt("WCH-Interrupt-fast"))) SW_Handler(void) {
//    __asm("li  a6, 0x1800");
//    __asm("jr  a6");
//    while(1);
}
