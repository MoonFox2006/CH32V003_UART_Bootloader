#include <stdint.h>
#include <stdio.h>
#include <Windows.h>

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

typedef struct __attribute__((packed)) _iap_t {
    uint8_t cmd;
    uint8_t len;
    uint8_t payload[FLASH_PAGE_SIZE];
} iap_t;

HANDLE hCom = INVALID_HANDLE_VALUE;
FILE *firmware = NULL;

static void comDone();

static BOOL comInit(const char *name, uint32_t baudrate) {
    DCB dcb;
    COMMTIMEOUTS cto;
    char fullname[strlen(name) + 5];

    strcpy(fullname, "\\\\.\\");
    strcat(fullname, name);
    hCom = CreateFile(fullname, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hCom == INVALID_HANDLE_VALUE)
        return FALSE;
    dcb.DCBlength = sizeof(DCB);
    if (! GetCommState(hCom, &dcb)) {
        comDone();
        return FALSE;
    }
    dcb.BaudRate = baudrate;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    if (! SetCommState(hCom, &dcb)) {
        comDone();
        return FALSE;
    }
    cto.ReadIntervalTimeout = 50; // 50 ms.
    cto.ReadTotalTimeoutMultiplier = 50; // 50 ms.
    cto.ReadTotalTimeoutConstant = 0;
    cto.WriteTotalTimeoutMultiplier = 50; // 50 ms.
    cto.WriteTotalTimeoutConstant = 0;
    if (! SetCommTimeouts(hCom, &cto)) {
        comDone();
        return FALSE;
    }
    return TRUE;
}

static void comDone() {
    if (hCom != INVALID_HANDLE_VALUE) {
        CloseHandle(hCom);
        hCom = INVALID_HANDLE_VALUE;
    }
}

static int16_t comRead() {
    DWORD readed;
    uint8_t buf;

    if ((! ReadFile(hCom, &buf, sizeof(buf), &readed, NULL)) || (readed != sizeof(buf)))
        return -1;
    return buf;
}

static BOOL comWrite(const uint8_t *data, uint8_t size) {
    DWORD written;

    if ((! WriteFile(hCom, data, size, &written, NULL)) || (written != size))
        return FALSE;
    return TRUE;
}

static BOOL receiveAnswer(uint8_t *state) {
    int16_t data;

    if ((data = comRead()) == UART_SIGN_2) {
        if ((data = comRead()) == UART_SIGN_1) {
            data = comRead();
            if ((data >= IAP_SUCCESS) && (data <= IAP_ERROR)) {
                *state = data;
                return TRUE;
            }
        }
    }
    return FALSE;
}

static BOOL sendFrame(const iap_t *frame) {
    const uint8_t UART_SIGN[2] = { UART_SIGN_1, UART_SIGN_2 };

    if (comWrite(UART_SIGN, sizeof(UART_SIGN))) {
        if (comWrite((uint8_t*)frame, sizeof(frame->cmd) + sizeof(frame->len) + frame->len)) {
            uint8_t crc = 0;

            crc += frame->cmd;
            crc += frame->len;
            for (uint8_t i = 0; i < frame->len; ++i) {
                crc += frame->payload[i];
            }
            return comWrite(&crc, sizeof(crc));
        }
    }
    return FALSE;
}

static uint8_t transactFrame(const iap_t *frame, uint8_t retries) {
    uint8_t state;

    while (1) {
        if (! PurgeComm(hCom, PURGE_RXCLEAR | PURGE_TXCLEAR))
            return IAP_ERROR;
        if (! sendFrame(frame))
            return IAP_ERROR;
        if (receiveAnswer(&state))
            return state;
        if (! retries--)
            return IAP_TIMEOUT;
        Sleep(100);
    }
}

static uint8_t iapProcess() {
    iap_t frame;
    uint8_t state;

    frame.cmd = IAP_CMD_HELO;
    frame.len = 0;
    printf("Connecting... ");
    if ((state = transactFrame(&frame, 50)) == IAP_SUCCESS) { // HELO confirmed
        frame.cmd = IAP_CMD_ERASE;
//        frame.len = 0;
        printf("OK\n"
            "Erasing... ");
        if ((state = transactFrame(&frame, 0)) == IAP_SUCCESS) { // ERASE confirmed
            size_t readed;

            fseek(firmware, 0, SEEK_SET);
            frame.cmd = IAP_CMD_PROG;
            printf("OK\n"
                "Flashing... ");
            do {
                readed = fread(frame.payload, 1, FLASH_PAGE_SIZE, firmware);
                if (readed) {
                    frame.len = readed;
//                    Sleep(50);
                    if ((state = transactFrame(&frame, 0)) != IAP_SUCCESS)
                        break;
                }
            } while (readed == FLASH_PAGE_SIZE);

            if (state == IAP_SUCCESS) { // PROG confirmed
//                Sleep(50);
                fseek(firmware, 0, SEEK_SET);
                frame.cmd = IAP_CMD_VERIFY;
                printf("OK\n"
                    "Verifying... ");
                do {
                    readed = fread(frame.payload, 1, FLASH_PAGE_SIZE, firmware);
                    if (readed) {
                        frame.len = readed;
                        if ((state = transactFrame(&frame, 0)) != IAP_SUCCESS)
                            break;
                    }
                } while (readed == FLASH_PAGE_SIZE);

                if (state == IAP_SUCCESS) { // VERIFY confirmed
                    frame.cmd = IAP_CMD_END;
                    frame.len = 0;
                    printf("OK\n"
                        "Restarting... ");
                    if ((state = transactFrame(&frame, 0)) == IAP_SUCCESS) { // END confirmed
                        printf("OK\n");
                        return IAP_END;
                    }
                }
            }
        }
    }
    printf("FAIL!\n");

    return state;
}

int main(int argc, char *argv[]) {
    uint8_t state, retries;

    if (argc != 3) {
        printf("Missing argument(s)!\n"
            "Usage: CH32_UART_Flasher.exe COMx firmware.bin\n");
        return 1;
    }

    if (! comInit(argv[1], 115200)) {
        printf("Error open COM port \"%s\"!\n", argv[1]);
        return 1;
    }

    if (! (firmware = fopen(argv[2], "rb"))) {
        comDone();
        printf("Error open firmware file \"%s\"!\n", argv[2]);
        return 1;
    }

    printf("Flashing firmware...\n");

    retries = 5;
    do {
        state = iapProcess();
        if (state != IAP_END) {
            if (! retries--)
                break;
            Sleep(100); // 100 ms.
            printf("Retrying...\n");
        }
    } while (state != IAP_END);

    fclose(firmware);
    comDone();

    if (state == IAP_END)
        return 0;

    return 1;
}
