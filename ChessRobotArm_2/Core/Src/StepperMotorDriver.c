/*******************************************************************************
* @file           : StepperMotorDriver.c
* @brief          : Command control of Stepper Motors
* @project        : ME 507 S'26 Lab 3
* @author         : Ivan Nieto (IN) - inieto@calpoly.edu
* @version        : 0.1
* @date           : 2026-04-22
* @compiler       : STM32CubeIDE v.1.12.0 Build: 14980_20230301_1550 (UTC)
* @target         : STM32F411CEU6
* @attention      : (c) 2023 STMicroelectronics.  All rights reserved.
*
* @par Revision History
* 0.1 260530 IN  Created Module
* 0.2 260531 IN  implemented helper for increase readability of register
* 					  maninpulation
*******************************************************************************/
#include "main.h"
#include "StepperMotorDriver.h"
#include "Joystick.h"

/**
 * @brief Sends a 40-bit datagram over SPI to a TMC stepper driver.
 *
 * @param datagram 40-bit SPI datagram to transmit.
 * @param motor Pointer to the stepper motor driver structure.
 */
void TMC_SPI_Send40(uint64_t datagram, stepper_motor_driver_t *motor)
{
    uint8_t tx[5];

    tx[0] = (datagram >> 32) & 0xFF;
    tx[1] = (datagram >> 24) & 0xFF;
    tx[2] = (datagram >> 16) & 0xFF;
    tx[3] = (datagram >> 8)  & 0xFF;
    tx[4] = datagram & 0xFF;

    HAL_GPIO_WritePin(motor->NCS_Port, motor->NCS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(motor->hspi, tx, 5, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(motor->NCS_Port, motor->NCS_Pin, GPIO_PIN_SET);
}

/**
 * @brief Initializes the TMC stepper motor driver.
 *
 * @param motor Pointer to the stepper motor driver structure.
 */
void TMC_Init(stepper_motor_driver_t *motor)
{
    // Chopper / driver setup
    TMC_SPI_Send40(0xEC000100C3, motor); // CHOPCONF
    TMC_SPI_Send40(0x9000061F0A, motor); // IHOLD_IRUN
    TMC_SPI_Send40(0x910000000A, motor); // TPOWERDOWN
    TMC_SPI_Send40(0x8000000004, motor); // GCONF: EN_PWM_MODE
    TMC_SPI_Send40(0x93000001F4, motor); // TPWM_THRS

    // Ramp generator setup
    TMC_SPI_Send40(packet(A1W, 1000), motor); // A1
    TMC_SPI_Send40(packet(V1W, 50000), motor); // V1
	 TMC_SPI_Send40(packet(AMAXW, 500), motor); // AMAX
	 TMC_SPI_Send40(packet(VMAXW, 200000), motor); // VMAX
	 TMC_SPI_Send40(packet(DMAXW, 700), motor); // DMAX
	 TMC_SPI_Send40(packet(D1W, 1400), motor); // D1
	 TMC_SPI_Send40(packet(VSTARTW, 0), motor); // VSTART
	 TMC_SPI_Send40(packet(VSTOPW, 10), motor); // VSTOP
	 TMC_SPI_Send40(packet(RAMPMODEW, 0), motor); // RAMPMODE = 0

}

/**
 * @brief Commands the stepper motor to move one rotation left.
 *
 * @param motor Pointer to the stepper motor driver structure.
 */
void TMC_MoveOneRotationLeft(stepper_motor_driver_t *motor)
{
    TMC_SPI_Send40(packet(XTARGETW, -51200), motor); // XTARGET = -51200
}

/**
 * @brief Commands the stepper motor to move one rotation left.
 *
 * @param motor Pointer to the stepper motor driver structure.
 */
void TMC_Spin1ForwardSpin1Backward(stepper_motor_driver_t *motor)
{
	  TMC_MoveOneRotationLeft(motor);
	  HAL_Delay(2500);
	  TMC_SPI_Send40(packet(XTARGETW, 0), motor);
	  HAL_Delay(2500);
}

/**
 * @brief Runs a slow-motion test command on the stepper motor.
 *
 * @param motor Pointer to the stepper motor driver structure.
 */
void TMC_RunSlowMotionTest(stepper_motor_driver_t *motor)
{
    TMC_SPI_Send40(0xADFFFF3800, motor); // XTARGET = -51200
}

void TMC_Stop(stepper_motor_driver_t *motor)
{
	TMC_SPI_Send40(packet(RAMPMODEW, RAMPMODE_HOLD), motor);
}

void TMC_JoystickCommand(stepper_motor_driver_t *motor)
{
	joy_x1 = Read_ADC_Channel(&hadc1, JOYSTICK1X);
	if (joy_x1 > 2010 + 1000)
	{
		TMC_SPI_Send40(packet(RAMPMODEW, RAMPMODE_VEL_NEG), motor);
		TMC_SPI_Send40(packet(VMAXW, (joy_x1 - 2048)*40), motor);
	}
	else if (joy_x1 < 2010 - 1000)
	{
		TMC_SPI_Send40(packet(RAMPMODEW, RAMPMODE_VEL_POS), motor);
		TMC_SPI_Send40(packet(VMAXW, -1*(joy_x1 - 2048)*40), motor);
	}
	else
	{
		TMC_SPI_Send40(packet(VMAXW, 0), motor);
		//TMC_SPI_Send40(packet(RAMPMODEW, RAMPMODE_HOLD), motor);
	}

}
