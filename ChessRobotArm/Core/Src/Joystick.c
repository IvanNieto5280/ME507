/*******************************************************************************
* @file           : Joystick.c
* @brief          : sets up Joystick to output ADC value
* @project         : EE 507 S'26 Chess Robot Final Project
* @authors         : Ivan Nieto (IN) - inieto@calpoly.edu
* @version         : 0.1
* @date            : 260513
* @compiler        : STM32CubeIDE v.1.12.0 Build: 14980_20230301_1550 (UTC)
* @target          : STM32F411CEU6
* @attention      : (c) 2023 STMicroelectronics.  All rights reserved.
*
* @par Hardware Connections
* PA4 - VLX
* PA5 - VLY
* PA6 - VRX
* PA7 - VRY
* 5V - 5V
* GND - GND
*
* @par Revision History
* 0.1 260513 IN  Created Module
*******************************************************************************/

#include "Joystick.h"

/**
 * @brief Reads a joystick channel and sends an ADC value
 *
 * @param *hadc pointer to adc structure
 * @param channel describes which adc channel is being asked for
 *
 * @return uint16_t ADC value
 */
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
