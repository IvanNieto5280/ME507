/*******************************************************************************
* @file           : Estop.c
* @brief          : Handles any errors from button press by user
* @project        : ME 507 S'26 Lab 3
* @author         : Ivan Nieto (IN) - inieto@calpoly.edu
* @version        : 0.1
* @date           : 2026-05-31
* @compiler       : STM32CubeIDE v.2.1.1 Build: 28236_20260312_0043 (UTC)
* @target         : STM32F411CEU6
* @attention      : (c) 2023 STMicroelectronics.  All rights reserved.
*
* @par Revision History
* 0.1 260530 IN  Created Module
*******************************************************************************/
#include "main.h"
#include "StepperMotorDriver.h"
#include "Estop.h"

/**
 * @brief Detect button press and set DRV_EN High.
 *
 * @param datagram 40-bit datagram to transmit.
 *
 * @return None.
 */
void Emergency_Stop_All(stepper_motor_driver_t *motor1,
								stepper_motor_driver_t *motor2,
								stepper_motor_driver_t *motor3)
{
    TMC_Stop(motor1);
    TMC_Stop(motor2);
    TMC_Stop(motor3);

    HAL_GPIO_WritePin(DRV_ENN_GPIO_Port, DRV_ENN_Pin, GPIO_PIN_SET);
}

void Release_From_Emergency(stepper_motor_driver_t *motor1,
								stepper_motor_driver_t *motor2,
								stepper_motor_driver_t *motor3)
{
    HAL_GPIO_WritePin(DRV_ENN_GPIO_Port, DRV_ENN_Pin, GPIO_PIN_RESET);
    TMC_SPI_Send40(packet(RAMPMODEW, RAMPMODE_POSITION), motor2);
}
