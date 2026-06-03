/*******************************************************************************
* @file           : fsm.c
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
#include "fsm.h"
#include "Servo.h"
#include "Estop.h"
#include "StepperMotorDriver.h"

FSM_t fsm = {
    .systemState = SYS_IDLE,
    .servoState = SERVO_OPEN,
	 .motorState = MOTOR_IDLE
};

void FSM_Update(FSM_t *fsm)
{
	 switch(fsm->systemState)
	 {
	 	 case SYS_IDLE:
	 		 //On power up, turn off motor power, wait for user button click
	 		 if (userButtonPressed == 1)
	 		 {
	 			 userButtonPressed = 0;
	 			 fsm->systemState = SYS_CALIBRATE;
	 		 }

	 		 break;

	 	 case SYS_CALIBRATE:
	 		 //Hit the limit switches
	 		 for (int i = 1; i < 10; i++) // This is just dummy calibration for now
	 		 {
	 			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET); //LED for Debug
	 			HAL_Delay(100);
	 			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET); //LED for Debug
	 			HAL_Delay(100);
	 		 }
	 		 fsm->systemState = SYS_JOYSTICK_CONTROL;
	 		 break;

	 	 case SYS_JOYSTICK_CONTROL:
	 		if (eStopPressed == 1)
	 		{
				 eStopPressed = 0;
				 Emergency_Stop_All(&motor1, &motor2, &motor3);

				 fsm->systemState = SYS_FAULT;
				 break;
			}

	 		TMC_JoystickCommand(&motor2);
//	 		switch(fsm->motorState)
//	 		{
//	 		case MOTOR_IDLE:
//	 			break;
//	 		case MOTOR_MOVE_RIGHT:
//
//	 			break;
//	 		case MOTOR_MOVE_LEFT:
//	  			break;
//	 		case MOTOR_WAIT_LEFT:
//	 			break;
//	 		case MOTOR_WAIT_RIGHT:
//	 			break;
//	 		}
	 		break;


	 	 case SYS_FAULT:
			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET); //LED for Debug
	 		if (userButtonPressed == 1)
	 		{
	 			userButtonPressed = 0;
	 			Release_From_Emergency(&motor1, &motor2, &motor3);
	 			fsm->systemState = SYS_CALIBRATE;
	 		}
	 		break;
	 }
}

void Servo_Update(FSM_t *fsm)
{
	 switch(fsm->servoState)
	 {
		  case SERVO_CLOSED:
			  // Close the Claw by sending certain PWM
			  servoOpen();
			  fsm->servoState = SERVO_OPEN;
				break;

	     case SERVO_OPEN:
	   	  // Open the Claw by sending certain PWM
	   	  servoClose();
	   	  fsm->servoState = SERVO_CLOSED;
	         break;

	 }
}
