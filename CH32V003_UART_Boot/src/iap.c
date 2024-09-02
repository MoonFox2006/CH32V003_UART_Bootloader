#include <string.h>
#include "iap.h"
#include "flash.h"

#define UART_TIMEOUT    50000 // 50 ms.

void systick_start(uint32_t us) {
    SysTick->SR = 0;
    SysTick->CMP = us * (FCPU / 8000000);
    SysTick->CNT = 0;
    SysTick->CTLR = (1 << 0);
}

void systick_stop() {
    SysTick->CTLR = 0;
}

static void uart_done(void) {
    USART1->CTLR1 = 0;

    GPIOD->CFGLR &= 0xF00FFFFF;
    GPIOD->CFGLR |= 0x04400000;

    RCC->APB2PCENR &= ~(RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1);
}

static int16_t uart_read(uint32_t timeout) {
    if (! (USART1->STATR & USART_STATR_RXNE)) {
        systick_start(timeout);
        while ((! (SysTick->SR & 0x01)) && (! (USART1->STATR & USART_STATR_RXNE))) {}
        systick_stop();
    }
    if (USART1->STATR & USART_STATR_RXNE)
        return USART1->DATAR;
    return -1;
}

static void uart_write(uint8_t data) {
    USART1->DATAR = data;
    while (! (USART1->STATR & USART_STATR_TC)) {}
}

static void uart_writes(const uint8_t *data, uint8_t size) {
    while (size--) {
        USART1->DATAR = *data++;
        while (! (USART1->STATR & USART_STATR_TC)) {}
    }
}

static uint8_t iap_read(iap_t *frame, uint32_t timeout) {
    int16_t data;
    uint8_t crc = 0;

    if ((data = uart_read(timeout)) == UART_SIGN_1) {
        if ((data = uart_read(timeout)) == UART_SIGN_2) {
            data = uart_read(timeout);
            if ((data >= IAP_CMD_HELO) && (data <= IAP_CMD_END)) {
                frame->cmd = data;
                crc += data;
                data = uart_read(timeout);
                if ((data >= 0) && (data <= FLASH_PAGE_SIZE)) {
                    memset(frame->payload, 0xFF, FLASH_PAGE_SIZE);
                    frame->len = data;
                    crc += data;
                    for (uint8_t i = 0; i < frame->len; ++i) {
                        data = uart_read(timeout);
                        if (data < 0)
                            return 0;
                        frame->payload[i] = data;
                        crc += data;
                    }
                    data = uart_read(timeout);
                    if ((data >= 0) && ((uint8_t)data == crc))
                        return 1;
                }
            }
        }
    }
    return 0; // Wrong data or timeout
}

static void iap_answer(uint8_t state) {
    uart_write(UART_SIGN_2);
    uart_write(UART_SIGN_1);
    uart_write(state);
}

static void uart_init(void) {
    RCC->APB2PCENR |= (RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1);

    GPIOD->CFGLR &= 0xF00FFFFF;
    GPIOD->CFGLR |= 0x04900000;

    USART1->BRR = 0xD0; /* Set 115200 baudrate */
    USART1->CTLR2 = 0;
    USART1->CTLR3 = 0;
    USART1->CTLR1 = USART_CTLR1_UE | USART_CTLR1_RE | USART_CTLR1_TE;
}

uint8_t uart_iap(void) {
    iap_t frame;
    uint32_t prog_addr = FLASH_BASE;
    uint8_t *verify_addr = (uint8_t*)FLASH_BASE;
    uint8_t erased = 0, complete = 0;

    uart_init();
    while (! complete) {
        if (iap_read(&frame, UART_TIMEOUT)) {
            uint8_t state;

            switch (frame.cmd) {
                case IAP_CMD_HELO:
                    erased = 0;
                    state = IAP_SUCCESS;
                    break;
                case IAP_CMD_ERASE:
                    FLASH_Unlock_Fast();
                    if (FLASH_EraseAllPages() == FLASH_COMPLETE) {
                        prog_addr = FLASH_BASE;
                        verify_addr = (uint8_t*)FLASH_BASE;
                        erased = 1;
                        state = IAP_SUCCESS;
                    } else
                        state = IAP_ERROR;
                    break;
                case IAP_CMD_PROG:
                    if (erased) {
                        CH32_IAP_Program(prog_addr, (const uint32_t*)frame.payload);
                        prog_addr += FLASH_PAGE_SIZE;
                        state = IAP_SUCCESS;
                    } else
                        state = IAP_ERROR;
                    break;
                case IAP_CMD_VERIFY:
                    state = IAP_SUCCESS;
                    for (uint8_t i = 0; i < frame.len; ++i) {
                        if (*verify_addr++ != frame.payload[i]) {
                            state = IAP_ERROR;
                            break;
                        }
                    }
                    break;
                case IAP_CMD_END:
                    complete = 1;
                    state = IAP_SUCCESS;
                    break;
                default:
                    state = IAP_ERROR;
                    break;
            }
            iap_answer(state);
        }
    }
    uart_done();

    return IAP_END;
}
