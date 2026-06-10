/*******************************************************************************
* @file           : Servo.h
* @brief          : Header file for Servo.c
* @project        : Chess Robot Arm
* @author         : Ivan Nieto (IN) - inieto@calpoly.edu
* @version        : 0.1
* @date           : 2026-05-31
* @compiler       : STM32CubeIDE v.2.1.1 Build: 28236_20260312_0043 (UTC)
* @target         : STM32F411CEU6
* @attention      : (c) 2023 STMicroelectronics. All rights reserved.
*
* @par Revision History
* 0.1 260530 IN Created Module
*******************************************************************************/

#include "main.h"

#ifndef INC_SERVO_H_
#define INC_SERVO_H_

void servoOpen();
void servoClose();

#define SERVO_OPEN_VAL 13000 //25000
#define SERVO_CLOSED_VAL 6000 //13000


#endif /* INC_SERVO_H_ */
