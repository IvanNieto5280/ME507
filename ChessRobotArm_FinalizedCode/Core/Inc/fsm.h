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

/**
 * @brief System operating states.
 */
typedef enum
{
    SYS_IDLE,              /**< Waiting for startup command. */
    SYS_CALIBRATE,         /**< Home all axes and configure motion parameters. */
    SYS_PI_CONTROL,        /**< Autonomous PI control mode. */
    SYS_JOYSTICK_CONTROL,  /**< Manual joystick control mode. */
    SYS_FAULT              /**< Emergency stop / fault state. */
} SystemState_t;

// Servo FSM

/**
 * @brief Servo motor operating states.
 */
typedef enum
{
	SERVO_CLOSED,			/**< Servo is Closed. */
	SERVO_OPEN				/**< Servo is Open. */
} ServoState_t;


/**
 * @brief Main finite state machine structure.
 */
typedef struct
{
    SystemState_t systemState; /**< Current system operating state. */
    ServoState_t servoState;   /**< Current servo state. */
} FSM_t;

extern FSM_t fsm;

void FSM_Update(FSM_t *fsm);
void Servo_Update(FSM_t *fsm);


#endif /* INC_FSM_H_ */
