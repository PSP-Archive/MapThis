#include <pspsdk.h>
#include <pspintrman_kernel.h>
#include <pspintrman.h>
#include <pspsyscon.h>
#include "siogps.h"

SceUID g_eventflag = -1;

void sioSetBaud(int baud)
{
        int div1, div2;

        div1 = PSP_UART_CLK / baud;
        div2 = div1 & 0x3F;
        div1 >>= 6;

        _sw(div1, PSP_UART4_DIV1);
        _sw(div2, PSP_UART4_DIV2);
        _sw(0x60, PSP_UART4_CTRL);
}

void _sioInit(void)
{
        sceHprmEnd();
        sceSysregUartIoEnable(4);
        sceSysconCtrlHRPower(1);
}

int intr_handler(void *arg)
{
        u32 stat;

        /* Read out the interrupt state and clear it */
        stat = _lw(0xBE500040);
        _sw(stat, 0xBE500044);

        sceKernelDisableIntr(PSP_HPREMOTE_INT);

        sceKernelSetEventFlag(g_eventflag, EVENT_SIO);

        return -1;
}

void sioInit(int baud, int kponly)
{
        _sioInit();
        if(!kponly)
        {
                g_eventflag = sceKernelCreateEventFlag("SioShellEvent", 0, 0, 0);
                void *func = (void *) ((unsigned int) intr_handler | 0x80000000);
                int st=sceKernelRegisterIntrHandler(PSP_HPREMOTE_INT, 1, func, NULL, NULL);
                st=sceKernelEnableIntr(PSP_HPREMOTE_INT);
                /* Delay thread for a bit */
                sceKernelDelayThread(2000000);
        }
        sioSetBaud(baud);
}

int sioReadChar(void)
{
        int ch;
        u32 result;
         SceUInt timeout;

        timeout = 100000;
        ch = pspDebugSioGetchar();
        if(ch == -1)
        {
                sceKernelEnableIntr(PSP_HPREMOTE_INT);
                sceKernelWaitEventFlag(g_eventflag, EVENT_SIO, 0x21, &result, &timeout);

                ch = pspDebugSioGetchar();
        }
        return ch;
}
