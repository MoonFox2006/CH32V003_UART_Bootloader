# CH32V003_UART_Bootloader
UART bootloader and flashing utility for CH32V003

Бутлоадер изначально шить с помощью WCH-LinkE утилитой WCH-LinkUtility по адресу 0x1FFFF000 и настроить PD7 как GPIO (не как ресет!)
Если на плату подать питание, удерживая PD7 на землю не менее 100 мс., то активируется UART бутлоадер на скорости 115200.
Утилита прошивки CH32_UART_Flasher первым параметром принимает COM порт, вторым имя прошивки в формате .bin
При успешной прошивке плата перезагрузится, иначе будет крутиться в бутлоадере.
