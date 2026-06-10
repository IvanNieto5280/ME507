/*******************************************************************************
* @file           : StepperMotorDriver.h
* @brief          : header for StepperMotorDriver.c
* project         : Chess Robot Arm
* authors         : Ivan Nieto (IN) - inieto@calpoly.edu
* version         : 0.1
* date            : 260530
* compiler        : STM32CubeIDE v.1.18.0 Build: 14980_20230301_1550 (UTC)
* target          : STM32F411CEU6
* @attention      : (c) 2023 STMicroelectronics.  All rights reserved.
*******************************************************************************
* REVISION HISTORY
* 0.1 260530 IN  Created Module
 *******************************************************************************
*/

#ifndef INC_STEPPERMOTORDRIVER_H_
#define INC_STEPPERMOTORDRIVER_H_

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* ---------------------------- ( NOTE: 80 chars long ) ------------------------
* typedef struct - defines the struct to handle the motor commands
* Inputs: none
* Output: none
* To do:  None :)
* -------------------------------------------------------------------------- */
typedef struct {
	// TIM_HandleTypeDef *htim;
	//	uint32_t uInChannel1;
	//	uint32_t uInChannel2;
	//	uint32_t uiPwmPulseWidth_Channel1;
	//	uint32_t uiPwmPulseWidth_Channel2;
	GPIO_TypeDef *NCS_Port;
	uint32_t NCS_Pin;
	GPIO_TypeDef *Diag0_Port;
	uint32_t Diag0_Pin;
	GPIO_TypeDef *Diag1_Port;
	uint32_t Diag1_Pin;
	SPI_HandleTypeDef *hspi;
} stepper_motor_driver_t;

// Define registers for readability

#define TMC_WRITE_BIT 0x80u

#define TMC_WRITE_ADDR(addr) ((uint8_t)((addr) | TMC_WRITE_BIT))

#define TMC_PACKET(addr, value) \
	((((uint64_t)TMC_WRITE_ADDR(addr)) << 32) | ((uint32_t)(value)))

void TMC_SetMaxVelocity(stepper_motor_driver_t *motor, uint32_t velocity);
void TMC_SetMaxAcceleration(stepper_motor_driver_t *motor, uint32_t acceleration);
void TMC_SetMaxDeceleration(stepper_motor_driver_t *motor, uint32_t deceleration);

void TMC_SPI_Send40(uint64_t datagram, stepper_motor_driver_t *motor);
void TMC_WriteRegister(uint8_t address, uint32_t data, stepper_motor_driver_t *motor);
uint32_t TMC_ReadRegister(uint8_t address, stepper_motor_driver_t *motor);

void TMC_Init(stepper_motor_driver_t *motor);

void TMC_EnableReferenceSwitches(stepper_motor_driver_t *motor);
void TMC_ClearRampStatus(stepper_motor_driver_t *motor);

void TMC_MoveOneRotationLeft(stepper_motor_driver_t *motor);
void TMC_MoveOneRotationRight(stepper_motor_driver_t *motor);

void TMC_MoveVelocityLeft(stepper_motor_driver_t *motor);
void TMC_MoveVelocityRight(stepper_motor_driver_t *motor);
void TMC_Stop(stepper_motor_driver_t *motor);

void TMC_MoveToPosition(stepper_motor_driver_t *motor, int32_t targetPosition);
void TMC_ResetPosition(stepper_motor_driver_t *motor);
int32_t TMC_GetPosition(stepper_motor_driver_t *motor);

bool TMC_LeftSwitchActive(stepper_motor_driver_t *motor);
bool TMC_RightSwitchActive(stepper_motor_driver_t *motor);
bool TMC_PositionReached(stepper_motor_driver_t *motor);

bool TMC_WaitForLeftSwitch(stepper_motor_driver_t *motor, uint32_t timeout_ms);
bool TMC_WaitForRightSwitch(stepper_motor_driver_t *motor, uint32_t timeout_ms);
bool TMC_WaitForPositionReached(stepper_motor_driver_t *motor, uint32_t timeout_ms);

// Speed acceleration helper functions
void TMC_SetMaxVelocity(stepper_motor_driver_t *motor, uint32_t velocity);
void TMC_SetMaxAcceleration(stepper_motor_driver_t *motor, uint32_t acceleration);
void TMC_SetMaxDeceleration(stepper_motor_driver_t *motor, uint32_t deceleration);

#endif /* INC_STEPPERMOTORDRIVER_H_ */
