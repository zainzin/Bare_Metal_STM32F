/*
* Created by Zain
*/


#include<stdio.h>
#include<string.h>
#include "stm32f407xx.h"

extern void initialise_monitor_handles();


uint8_t rxComplt = RESET;

#define MY_ADDR 0x61;

#define SLAVE_ADDR  0x68

void delay(void) {
    for (uint32_t i = 0; i < 500000 / 2; i++);
}

I2C_Handle_t I2C1Handle;


uint8_t rcv_buf[32];


void I2C1_GPIOInits(void) {
    GPIO_Handle_t I2CPins;

    I2CPins.pGPIOx = GPIOB;
    I2CPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    I2CPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
    I2CPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
    I2CPins.GPIO_PinConfig.GPIO_PinAltFunMode = 4;
    I2CPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;


    I2CPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6;
    GPIO_Init(&I2CPins);


    I2CPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_7;
    GPIO_Init(&I2CPins);
}

void I2C1_Inits(void) {
    I2C1Handle.pI2Cx = I2C1;
    I2C1Handle.I2C_Config.I2C_AckControl = I2C_ACK_ENABLE;
    I2C1Handle.I2C_Config.I2C_DeviceAddress = MY_ADDR;
    I2C1Handle.I2C_Config.I2C_FMDutyCycle = I2C_FM_DUTY_2;
    I2C1Handle.I2C_Config.I2C_SCLSpeed = I2C_SCL_SPEED_SM;

    I2C_Init(&I2C1Handle);
}

void GPIO_ButtonInit(void) {
    GPIO_Handle_t GPIOBtn, GpioLed;


    GPIOBtn.pGPIOx = GPIOA;
    GPIOBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
    GPIOBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
    GPIOBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    GPIOBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    GPIO_Init(&GPIOBtn);


    GpioLed.pGPIOx = GPIOD;
    GpioLed.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
    GpioLed.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    GpioLed.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    GpioLed.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
    GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    GPIO_PeriClockControl(GPIOD, ENABLE);

    GPIO_Init(&GpioLed);
}


int main(void) {
    uint8_t commandcode;

    uint8_t len;

    initialise_monitor_handles();

    printf("Application is running\n");

    GPIO_ButtonInit();


    I2C1_GPIOInits();


    I2C1_Inits();


    I2C_IRQInterruptConfig(IRQ_NO_I2C1_EV, ENABLE);
    I2C_IRQInterruptConfig(IRQ_NO_I2C1_ER, ENABLE);


    I2C_PeripheralControl(I2C1, ENABLE);


    I2C_ManageAcking(I2C1, I2C_ACK_ENABLE);

    while (1) {
        while (!GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0));


        delay();

        commandcode = 0x51;

        while (I2C_MasterSendDataIT(&I2C1Handle, &commandcode, 1,SLAVE_ADDR, I2C_ENABLE_SR) != I2C_READY);
        while (I2C_MasterReceiveDataIT(&I2C1Handle, &len, 1,SLAVE_ADDR, I2C_ENABLE_SR) != I2C_READY);

        commandcode = 0x52;
        while (I2C_MasterSendDataIT(&I2C1Handle, &commandcode, 1,SLAVE_ADDR, I2C_ENABLE_SR) != I2C_READY);
        while (I2C_MasterReceiveDataIT(&I2C1Handle, rcv_buf, len,SLAVE_ADDR, I2C_DISABLE_SR) != I2C_READY);

        rxComplt = RESET;

        while (rxComplt != SET) {
        }

        rcv_buf[len + 1] = '\0';

        printf("Data : %s", rcv_buf);

        rxComplt = RESET;
    }
}


void I2C1_EV_IRQHandler(void) {
    I2C_EV_IRQHandling(&I2C1Handle);
}


void I2C1_ER_IRQHandler(void) {
    I2C_ER_IRQHandling(&I2C1Handle);
}


void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle, uint8_t AppEv) {
    if (AppEv == I2C_EV_TX_CMPLT) {
        printf("Tx is completed\n");
    } else if (AppEv == I2C_EV_RX_CMPLT) {
        printf("Rx is completed\n");
        rxComplt = SET;
    } else if (AppEv == I2C_ERROR_AF) {
        printf("Error : Ack failure\n");

        I2C_CloseSendData(pI2CHandle);
        I2C_GenerateStopCondition(I2C1);

        while (1);
    }
}
