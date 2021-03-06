/*
 / ____)              _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
(C)2017 Semtech

*/

#include "mbed.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "Registers1308.h"
#include "string.h"
#include "CmdUSB.h"
#include "board.h"
USBMANAGER Usbmanager;
void Error_Handler(void);

#if DEBUG_MAIN == 0
#define DEBUG_MSG(str)                pc.printf(str)
#define DEBUG_PRINTF(fmt, args...)    pc.printf("%s:%d: "fmt, __FUNCTION__, __LINE__, args)
#define DEBUG_ARRAY(a,b,c)           for(a=0;a!=0;){}
#define CHECK_NULL(a)                if(a==NULL){return LGW_HAL_ERROR;}
#else
#define DEBUG_MSG(str)
#define DEBUG_PRINTF(fmt, args...)
#define DEBUG_ARRAY(a,b,c)            for(a=0;a!=0;){}
#define CHECK_NULL(a)                 if(a==NULL){return LGW_HAL_ERROR;}
#endif

void Error_Handler(void)
{
    DEBUG_MSG("error\n");
}

int main(void)
{
    uint16_t size;
    uint16_t sizet;
    uint8_t dataflash;

    dataflash = *(uint8_t *)DATA_EEPROM_BASE;
    if (dataflash == GOTODFU)
    {
        FLASH_Prog(DATA_EEPROM_BASE, 0);
        ((void (*)(void)) * ((uint32_t*) 0x1FFF0004))();
    }

    /************ start init  *************/
    Sx1308.init();
    Sx1308.SelectPage(0);
    Usbmanager.init();
    Usbmanager.initBuffromhost();
    wait_ms(1000);
    /************ end init  *************/
    Usbmanager.count = 1; // wait for an 64 bytes transfer
    Usbmanager.ReceiveCmd();
    while (1) {
        while (Usbmanager.count > 0) {// wait until it usbcmd rx
        }
        Usbmanager.count = 1;
        Usbmanager.initBuftohost();
        if (Usbmanager.DecodeCmd()) {    // decode cmd from Host
            size = (uint16_t)((Usbmanager.BufToHost[1] << 8) + Usbmanager.BufToHost[2] + 3);
            if ((size % 64) == 0) // for ZLP
            {
                sizet = size + 1;
            }
            else {
                sizet = size;
            }
            while (CDC_Transmit_FS(Usbmanager.BufToHost, sizet) != USBD_OK) // transmit answer to Host
            {
            }
        }
        Usbmanager.initBuffromhost(); // clean buffer
        Usbmanager.ReceiveCmd();
    }
}
