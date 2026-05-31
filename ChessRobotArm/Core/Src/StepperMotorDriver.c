/*******************************************************************************
* @file           : StepperMotorDriver.c
* @brief          : Command control of Stepper Motors
* project         : ME 507 S'26 Lab 3
* authors         : Ivan Nieto (IN) - inieto@calpoly.edu
* version         : 0.1
* date            : 260422
* compiler        : STM32CubeIDE v.1.12.0 Build: 14980_20230301_1550 (UTC)
* target          : STM32F411CEU6
* @attention      : (c) 2023 STMicroelectronics.  All rights reserved.
*******************************************************************************
* REVISION HISTORY
* 0.1 260530 IN  Created Module
 *******************************************************************************
*/
#include "main.h"
#include "StepperMotorDriver.h"

/* ---------------------------- ( NOTE: 80 chars long ) ------------------------
* TMC_SPI_Send40 - SPI Code to send 40bit datagram
* Inputs: datagram, CSN port, CSN pin, and SPI pointer
* Output: none
* To do:  None :)
* -------------------------------------------------------------------------- */
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

/* ---------------------------- ( NOTE: 80 chars long ) ------------------------
* TMC_init - Initializes the motor as stated in page 116
* Inputs: none
* Output: none
* To do:  None :)
* -------------------------------------------------------------------------- */
void TMC_Init(stepper_motor_driver_t *motor)
{
    // Chopper / driver setup
    TMC_SPI_Send40(0xEC000100C3, motor); // CHOPCONF
    TMC_SPI_Send40(0x9000061F0A, motor); // IHOLD_IRUN
    TMC_SPI_Send40(0x910000000A, motor); // TPOWERDOWN
    TMC_SPI_Send40(0x8000000004, motor); // GCONF: EN_PWM_MODE
    TMC_SPI_Send40(0x93000001F4, motor); // TPWM_THRS

    // Ramp generator setup
    TMC_SPI_Send40(0xA4000003E8, motor); // A1
    TMC_SPI_Send40(0xA50000C350, motor); // V1
    TMC_SPI_Send40(0xA6000001F4, motor); // AMAX
    TMC_SPI_Send40(0xA700030D40, motor); // VMAX
    TMC_SPI_Send40(0xA8000002BC, motor); // DMAX
    TMC_SPI_Send40(0xAA00000578, motor); // D1
    TMC_SPI_Send40(0xAB0000000A, motor); // VSTOP
    TMC_SPI_Send40(0xA000000000, motor); // RAMPMODE = 0
}

/* ---------------------------- ( NOTE: 80 chars long ) ------------------------
* TMC_MoveOneRotationLeft - test command to move stepper motor in one direction
* Inputs: none
* Output: none
* To do:  None :)
* -------------------------------------------------------------------------- */
void TMC_MoveOneRotationLeft(stepper_motor_driver_t *motor)
{
    TMC_SPI_Send40(0xADFFFF3800, motor); // XTARGET = -51200
}
