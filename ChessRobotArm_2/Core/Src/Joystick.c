/**
*******************************************************************************
* @file           : Joystick.c
* @brief          : sets up Joystick to output ADC value
* project         : EE 507 S'26 Chess Robot Final Project
* authors         : Ivan Nieto (IN) - inieto@calpoly.edu
* version         : 0.1
* date            : 260513
* compiler        : STM32CubeIDE v.1.12.0 Build: 14980_20230301_1550 (UTC)
* target          : STM32F411CEU6
* @attention      : (c) 2023 STMicroelectronics.  All rights reserved.
*******************************************************************************
* STM32F411 to Joystick wiring
* PA4 - VRX
* PA5 - VRY
* 5V - 5V
* GND - GND
*******************************************************************************
* REVISION HISTORY
* 0.1 260513 IN  Created Module
*******************************************************************************
**/

#include "Joystick.h"

uint16_t Read_ADC_Channel(ADC_HandleTypeDef *hadc, uint32_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel = channel;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;

    HAL_ADC_ConfigChannel(hadc, &sConfig);

    HAL_ADC_Start(hadc);
    HAL_ADC_PollForConversion(hadc, 100);

    uint16_t value = HAL_ADC_GetValue(hadc);

    HAL_ADC_Stop(hadc);

    return value;
}
