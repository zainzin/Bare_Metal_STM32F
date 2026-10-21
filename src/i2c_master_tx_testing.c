/*
* Created by Zain
*/


#include<stdio.h>
#include<string.h>
#include "stm32f407xx.h"

#define MY_ADDR 0x61;

#define SLAVE_ADDR  0x68

void delay(void) {
    for (uint32_t i = 0; i < 500000 / 2; i++);
}

I2C_Handle_t I2C1Handle;


uint8_t some_data[] = "We are testing I2C master Tx\n";


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


    I2CPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_9;

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
    GPIO_Handle_t GPIOBtn;


    GPIOBtn.pGPIOx = GPIOA;
    GPIOBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
    GPIOBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
    GPIOBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    GPIOBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    GPIO_Init(&GPIOBtn);
}


int main(void) {
    GPIO_ButtonInit();
    I2C1_GPIOInits();
    I2C1_Inits();

    I2C_PeripheralControl(I2C1, ENABLE);

    while (1) {
        while (!GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0));

        delay();
        I2C_MasterSendData(&I2C1Handle, some_data, strlen((char *) some_data),SLAVE_ADDR, 0);
    }
}
