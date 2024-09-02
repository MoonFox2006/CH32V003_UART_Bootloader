#ifndef __IAP_H
#define __IAP_H

#include <ch32v00x.h>

#define FLASH_PAGE_SIZE 64

#define UART_SIGN_1     0x55
#define UART_SIGN_2     0xAA

#define IAP_CMD_HELO    0x80
#define IAP_CMD_ERASE   0x81
#define IAP_CMD_PROG    0x82
#define IAP_CMD_VERIFY  0x83
#define IAP_CMD_END     0x84

#define IAP_SUCCESS     0x00
#define IAP_ERROR       0x01
#define IAP_TIMEOUT     0x02
#define IAP_END         0x03

typedef struct __attribute__((packed)) {
    uint8_t cmd;
    uint8_t len;
    uint8_t payload[FLASH_PAGE_SIZE];
} iap_t;

void systick_start(uint32_t us);
void systick_stop();

uint8_t uart_iap(void);

#endif
