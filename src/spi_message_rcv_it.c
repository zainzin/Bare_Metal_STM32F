/*
* Created by Zain
*/


#include<stdio.h>
#include<string.h>
#include "stm32f407xx.h"


SPI_Handle_t SPI2handle;

#define MAX_LEN 500

char RcvBuff[MAX_LEN];

volatile char ReadByte;


volatile uint8_t rcvStop = 0;


volatile uint8_t dataAvailable = 0;

void delay(void) {
    for (uint32_t i = 0; i < 500000 / 2; i++);
}


void SPI2_GPIOInits(void) {
    GPIO_Handle_t SPIPins;

    SPIPins.pGPIOx = GPIOB;
    SPIPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    SPIPins.GPIO_PinConfig.GPIO_PinAltFunMode = 5;
    SPIPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    SPIPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;


    SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
    GPIO_Init(&SPIPins);


    SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_15;
    GPIO_Init(&SPIPins);


    SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_14;
    GPIO_Init(&SPIPins);


    SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
    GPIO_Init(&SPIPins);
}

void SPI2_Inits(void) {
    SPI2handle.pSPIx = SPI2;
    SPI2handle.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;
    SPI2handle.SPIConfig.SPI_DeviceMode = SPI_DEVICE_MODE_MASTER;
    SPI2handle.SPIConfig.SPI_SclkSpeed = SPI_SCLK_SPEED_DIV32;
    SPI2handle.SPIConfig.SPI_DFF = SPI_DFF_8BITS;
    SPI2handle.SPIConfig.SPI_CPOL = SPI_CPOL_LOW;
    SPI2handle.SPIConfig.SPI_CPHA = SPI_CPHA_LOW;
    SPI2handle.SPIConfig.SPI_SSM = SPI_SSM_DI;

    SPI_Init(&SPI2handle);
}


void Slave_GPIO_InterruptPinInit(void) {
    GPIO_Handle_t spiIntPin;
    memset(&spiIntPin, 0, sizeof(spiIntPin));


    spiIntPin.pGPIOx = GPIOD;
    spiIntPin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6;
    spiIntPin.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IT_FT;
    spiIntPin.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;
    spiIntPin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;

    GPIO_Init(&spiIntPin);

    GPIO_IRQPriorityConfig(IRQ_NO_EXTI9_5, NVIC_IRQ_PRI15);
    GPIO_IRQInterruptConfig(IRQ_NO_EXTI9_5, ENABLE);
}


int main(void) {
    uint8_t dummy = 0xff;

    Slave_GPIO_InterruptPinInit();
    SPI2_GPIOInits();
    SPI2_Inits();

    SPI_SSOEConfig(SPI2, ENABLE);

    SPI_IRQInterruptConfig(IRQ_NO_SPI2, ENABLE);

    while (1) {
        rcvStop = 0;

        while (!dataAvailable);

        GPIO_IRQInterruptConfig(IRQ_NO_EXTI9_5, DISABLE);


        SPI_PeripheralControl(SPI2, ENABLE);


        while (!rcvStop) {
            while (SPI_SendDataIT(&SPI2handle, &dummy, 1) == SPI_BUSY_IN_TX);
            while (SPI_ReceiveDataIT(&SPI2handle, &ReadByte, 1) == SPI_BUSY_IN_RX);
        }


        while (SPI_GetFlagStatus(SPI2, SPI_BUSY_FLAG));


        SPI_PeripheralControl(SPI2, DISABLE);

        printf("Rcvd data = %s\n", RcvBuff);

        dataAvailable = 0;

        GPIO_IRQInterruptConfig(IRQ_NO_EXTI9_5, ENABLE);
    }

    return 0;
}


void SPI2_IRQHandler(void) {
    SPI_IRQHandling(&SPI2handle);
}


void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle, uint8_t AppEv) {
    static uint32_t i = 0;

    if (AppEv == SPI_EVENT_RX_CMPLT) {
        RcvBuff[i++] = ReadByte;
        if (ReadByte == '\0' || (i == MAX_LEN)) {
            rcvStop = 1;
            RcvBuff[i - 1] = '\0';
            i = 0;
        }
    }
}


void EXTI9_5_IRQHandler(void) {
    GPIO_IRQHandling(GPIO_PIN_NO_6);
    dataAvailable = 1;
}
