/*******************************************************************************
* @file           : fsm.h
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

#ifndef INC_FSM_H_
#define INC_FSM_H_

// General System FSM
typedef enum
{
    SYS_IDLE,
	 SYS_CALIBRATE,
    SYS_JOYSTICK_CONTROL,
    SYS_FAULT
} SystemState_t;

// Servo FSM
typedef enum
{
	SERVO_CLOSED,
	SERVO_OPEN
} ServoState_t;

// Motor State FSM
typedef enum
{
	MOTOR_IDLE,
   MOTOR_MOVE_RIGHT,
	MOTOR_MOVE_LEFT,
   MOTOR_WAIT_RIGHT,
   MOTOR_WAIT_LEFT
} MotorState_t;

typedef struct
{
    SystemState_t systemState;
    ServoState_t servoState;
    MotorState_t motorState;
 	 uint32_t movementTimer;
} FSM_t;

extern FSM_t fsm;

void FSM_Update(FSM_t *fsm);
void Servo_Update(FSM_t *fsm);


#endif /* INC_FSM_H_ */
