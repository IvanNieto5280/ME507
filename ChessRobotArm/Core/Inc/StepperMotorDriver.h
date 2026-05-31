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

void TMC_SPI_Send40(uint64_t datagram, stepper_motor_driver_t *motor);
void TMC_Init(stepper_motor_driver_t *motor);
void TMC_MoveOneRotationLeft(stepper_motor_driver_t *motor);
#endif /* INC_STEPPERMOTORDRIVER_H_ */
