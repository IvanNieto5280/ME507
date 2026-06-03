/**
*******************************************************************************
* @file           : Joystick.h
* @brief          : Header for Joystick.c
* project         : EE 507 S'26 Chess Robot Final Project
* authors         : Ivan Nieto (IN) - inieto@calpoly.edu
* version         : 0.1
* date            : 260513
* compiler        : STM32CubeIDE v.2.1.1 Build: 28236_20260312_0043 (UTC)
* target          : STM32F411CEU6
* @attention      : (c) 2023 STMicroelectronics.  All rights reserved.
*******************************************************************************
* REVISION HISTORY
* 0.1 260513 IN  Created Module
*******************************************************************************
**/

#include "main.h"

#ifndef INC_JOYSTICK_H_
#define INC_JOYSTICK_H_

#define JOYSTICK1X ADC_CHANNEL_4 // PA4
#define JOYSTICK1Y ADC_CHANNEL_5 // PA5
#define JOYSTICK2X ADC_CHANNEL_6 // PA4
#define JOYSTICK2Y ADC_CHANNEL_7 // PA5


uint16_t Read_ADC_Channel(ADC_HandleTypeDef *hadc, uint32_t channel);



#endif /* INC_JOYSTICK_H_ */
