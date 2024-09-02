#include <ch32v00x.h>
#include <ch32v00x_rcc.h>
#include <ch32v00x_flash.h>
#include "iap.h"

#define BTN_RCC     RCC_APB2Periph_GPIOD
#define BTN_GPIO    GPIOD
#define BTN_PIN     7

static uint8_t checkButton(void) {
    uint8_t result = 1;

    RCC->APB2PCENR |= BTN_RCC;
    BTN_GPIO->CFGLR &= ~(0x0F << (BTN_PIN * 4));
    BTN_GPIO->CFGLR |= (0x08 << (BTN_PIN * 4));
    BTN_GPIO->BSHR |= (1 << BTN_PIN);

    systick_start(100000); // 100 ms.
    while (! (SysTick->SR & 0x01)) {
        if (BTN_GPIO->INDR & (1 << BTN_PIN)) { // Button released
            result = 0;
            break;
        }
    }
    systick_stop();

    BTN_GPIO->CFGLR &= ~(0x0F << (BTN_PIN * 4));
    BTN_GPIO->CFGLR |= (0x04 << (BTN_PIN * 4));
    BTN_GPIO->BCR |= (1 << BTN_PIN);
    RCC->APB2PCENR &= ~BTN_RCC;

    return result;
}

int main(void) {
    if (checkButton()) {
        uart_iap();
    }

    RCC_ClearFlag();
    SystemReset_StartMode(Start_Mode_USER);
    NVIC_SystemReset();

    while(1) {}
}
