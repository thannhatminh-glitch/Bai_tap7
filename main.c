#include "stm32f10x.h"
#include <stdio.h>

void UART1_Init(void);
void UART1_SendStr(char *s);
void SPI1_Init(void);
uint8_t SPI1_Transfer(uint8_t data);
void delay_ms(uint32_t ms);

int main(void)
{
    uint8_t rx;
    char buf[30];

    SystemInit();
    UART1_Init();
    SPI1_Init();

    UART1_SendStr("SPI Master Test\r\n");

    while (1)
    {
        // Kéo NSS xuong (chan Slave)
        GPIO_ResetBits(GPIOA, GPIO_Pin_4);

        // Gui 1 byte (vi du 0x55) và nhan lai phan hoi
        rx = SPI1_Transfer(0x55);

        // Nha NSS
        GPIO_SetBits(GPIOA, GPIO_Pin_4);

        sprintf(buf, "Received: 0x%02X\r\n", rx);
        UART1_SendStr(buf);

        delay_ms(1000);
    }
}

/*================ UART1 ================*/
void UART1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitS;
    USART_InitTypeDef USART_InitS;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    // PA9 TX
    GPIO_InitS.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitS.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitS.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitS);

    // PA10 RX
    GPIO_InitS.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitS.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitS);

    USART_InitS.USART_BaudRate = 9600;
    USART_InitS.USART_WordLength = USART_WordLength_8b;
    USART_InitS.USART_StopBits = USART_StopBits_1;
    USART_InitS.USART_Parity = USART_Parity_No;
    USART_InitS.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitS.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART1, &USART_InitS);
    USART_Cmd(USART1, ENABLE);
}

void UART1_SendStr(char *s)
{
    while (*s)
    {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, *s++);
    }
}

/*================ SPI1 Master ================*/
void SPI1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitS;
    SPI_InitTypeDef SPI_InitS;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1, ENABLE);

    // PA5 SCK, PA7 MOSI, PA6 MISO, PA4 NSS
    // SCK & MOSI as Alternate Function Push Pull
    GPIO_InitS.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitS.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitS.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitS);

    // MISO as input floating
    GPIO_InitS.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitS.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitS);

    // NSS as GPIO output (chu dong dieu khien)
    GPIO_InitS.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitS.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitS);
    GPIO_SetBits(GPIOA, GPIO_Pin_4); // ban dau nha

    // Cau hinh SPI1 Master
    SPI_InitS.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitS.SPI_Mode = SPI_Mode_Master;
    SPI_InitS.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitS.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitS.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitS.SPI_NSS = SPI_NSS_Soft;
    SPI_InitS.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16; // tốc độ ~ 4.5 MHz nếu SYSCLK=72 MHz
    SPI_InitS.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitS.SPI_CRCPolynomial = 7;

    SPI_Init(SPI1, &SPI_InitS);
    SPI_Cmd(SPI1, ENABLE);
}

// Gửi 1 byte và nhận 1 byte
uint8_t SPI1_Transfer(uint8_t data)
{
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, data);
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_I2S_ReceiveData(SPI1);
}

/*================ Delay SysTick ================*/
void delay_ms(uint32_t ms)
{
    SysTick->LOAD  = (SystemCoreClock / 1000) - 1;
    SysTick->VAL   = 0;
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    while (ms--)
        while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));

    SysTick->CTRL = 0;
}

