/*******************************************************************************
* @file           : Servo.c
* @brief          : Handles overall flow of the program
* @project        : ME 507 S'26 Lab 3
* @author         : Ivan Nieto (IN) - inieto@calpoly.edu
* @version        : 0.1
* @date           : 2026-05-31
* @compiler       : STM32CubeIDE v.2.1.1 Build: 28236_20260312_0043 (UTC)
* @target         : STM32F411CEU6
* @attention      : (c) 2023 STMicroelectronics.  All rights reserved.
*
* @par Revision History
* 0.1 260531 IN  Created Module
*******************************************************************************/
#include "main.h"
#include "servo.h"

void servoOpen()
{
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET); //LED for Debug
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, SERVO_OPEN_VAL);
}

void servoClosed()
{
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET); //LED for Debug
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, SERVO_CLOSED_VAL);
}
