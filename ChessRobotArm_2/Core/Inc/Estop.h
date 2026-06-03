/*******************************************************************************
* @file           : Estop.h
* @brief          : Header file for Estop.c
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
#include "StepperMotorDriver.h"

#ifndef INC_ESTOP_H_
#define INC_ESTOP_H_

void Emergency_Stop_All(stepper_motor_driver_t *motor1,
								stepper_motor_driver_t *motor2,
								stepper_motor_driver_t *motor3);
void Release_From_Emergency(stepper_motor_driver_t *motor1,
								stepper_motor_driver_t *motor2,
								stepper_motor_driver_t *motor3);


#endif /* INC_ESTOP_H_ */
