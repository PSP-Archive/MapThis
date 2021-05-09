#define PSP_UART4_DIV1 0xBE500024
#define PSP_UART4_DIV2 0xBE500028
#define PSP_UART4_CTRL 0xBE50002C
#define PSP_UART_CLK   96000000
#define EVENT_SIO       0x01
int sceHprmEnd(void);
int sceSysregUartIoEnable(int uart);
int sceSyscon_driver_44439604(int power);


void sioInit(int baud, int kponly);
int sioReadChar();
