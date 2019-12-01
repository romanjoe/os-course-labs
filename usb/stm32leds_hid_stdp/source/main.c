/*###############################################################################
* main.c
*
* This file is a part of demo application of STM32 hid device
*
* Copyright (C) 2017 Roman Okhrimenko <mrromanjoe@gmail.com>
* 
* Description of this application, Linux driver to support it
* and user space application for demonstration can be found here
* <https://github.com/romanjoe/os-course-labs/usb>
*
*###############################################################################
* Copyright 2018-2019 Cypress Semiconductor Corporation
* SPDX-License-Identifier: Apache-2.0
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*############################################################################### */

#include "usbd_hid_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "core_cm4.h"
#include "stm32f4xx.h"

#define BUF_SIZE 11

/* input/output buffer in relation to host */
uint8_t InBuffer[BUF_SIZE];
uint8_t OutBuffer[BUF_SIZE];

USB_OTG_CORE_HANDLE USB_OTG_dev;

uint8_t push_button_cnt = 0;

 void DelayQ(uint32_t nTime) {
     while ( --nTime)  {   }
 }

int main(void)
{
    SystemInit();

    STM32F4_Discovery_LEDInit(LED3);
    STM32F4_Discovery_LEDInit(LED4);
    STM32F4_Discovery_LEDInit(LED5);
    STM32F4_Discovery_LEDInit(LED6);

    STM32F4_Discovery_PBInit(BUTTON_USER, BUTTON_MODE_GPIO);

    USBD_Init(&USB_OTG_dev,  USB_OTG_FS_CORE_ID,  &USR_desc,  &USBD_HID_cb,  &USR_cb);

    InBuffer[0]=2; // REPORT_ID value
    USBD_HID_SendReport (&USB_OTG_dev, InBuffer, 11);

    while (1)
    {
        if (OutBuffer[9] != 0xFF) // Check PC->MCU buffer
        {
            if((OutBuffer[9])  & (1  << LED3))
            {
                STM32F4_Discovery_LEDOn(LED3);
            } else STM32F4_Discovery_LEDOff(LED3);
            if(OutBuffer[9] & (1  << LED4))
            {
                STM32F4_Discovery_LEDOn(LED4);
            } else STM32F4_Discovery_LEDOff(LED4);
            if(OutBuffer[9] & (1  << LED5))
            {
                STM32F4_Discovery_LEDOn(LED5);
            } else STM32F4_Discovery_LEDOff(LED5);
            if(OutBuffer[9] & (1  << LED6))
            {
                STM32F4_Discovery_LEDOn(LED6);
            } else STM32F4_Discovery_LEDOff(LED6);
        }
        if(OutBuffer[9] == 0)
        {
                STM32F4_Discovery_LEDOff(LED6);
                STM32F4_Discovery_LEDOff(LED5);
                STM32F4_Discovery_LEDOff(LED4);
                STM32F4_Discovery_LEDOff(LED3);
        }

        if(OutBuffer[8] == 1) // Check PC->MCU buffer
        {
            OutBuffer[8] = 0;
            InBuffer[0] = 2; // Fill REPORT_ID for MCU->PC buffer
            InBuffer[1] = push_button_cnt;
        for(uint32_t i = 2; i < 10; i++)
        {
            InBuffer[i] = 0xFF;
        }

        USBD_HID_SendReport (&USB_OTG_dev, InBuffer, 10); // ready to send MCU->PC
        }

    }
}

void OTG_FS_IRQHandler(void)
{
    USBD_OTG_ISR_Handler(&USB_OTG_dev);
}









