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
#include "Joystick.h"
#include "fsm.h"
#include "Servo.h"
#include "Estop.h"
#include "StepperMotorDriver.h"

FSM_t fsm = {
    .systemState = SYS_IDLE,
    .servoState = SERVO_OPEN,
	 .motorState = MOTOR_IDLE
};

static uint8_t prevButton = 0;

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
	 		 if (GPIOB->IDR & PI_or_JS_Pin)
	 		 {
		 		 fsm->systemState = SYS_JOYSTICK_CONTROL;
	 		 }
	 		 else
	 		 {
	 			 fsm->systemState = SYS_PI_CONTROL;
	 		 }

	 		 break;

	 	 case SYS_PI_CONTROL:

	 		break;

	 	 case SYS_JOYSTICK_CONTROL:
	 		if (eStopPressed == 1)
	 		{
				 eStopPressed = 0;
				 Emergency_Stop_All(&motor1, &motor2, &motor3);
				 fsm->servoState = SERVO_OPEN;
				 fsm->systemState = SYS_FAULT;
				 break;
			}

	 		// Command control of the robot with the joysticks
		   uint16_t joy_x1 = Read_ADC_Channel(&hadc1, JOYSTICK1X);
		   uint16_t joy_y1 = Read_ADC_Channel(&hadc1, JOYSTICK1Y);
		   uint16_t joy_y2 = Read_ADC_Channel(&hadc1, JOYSTICK2Y);

	 		TMC_JoystickCommand(&motor1, joy_x1);
	 		TMC_JoystickCommand(&motor2, joy_y1);
	 		TMC_JoystickCommand(&motor3, joy_y2);

	 		// Command control of the servo with the button
	 		Servo_Update(fsm);

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

   uint8_t buttonEvent = (userButtonPressed && !prevButton);

   prevButton = userButtonPressed;

	 switch(fsm->servoState)
	 {
		  case SERVO_CLOSED:
			  // Close the Claw by sending certain PWM
			  servoClose();
			  if (buttonEvent)
			  {
				  fsm->servoState = SERVO_OPEN;
				  userButtonPressed = 0;

			  }
			  break;

	     case SERVO_OPEN:
	   	  // Open the Claw by sending certain PWM
	   	  servoOpen();
	   	  if (buttonEvent)
	   	  {
		   	  fsm->servoState = SERVO_CLOSED;
		   	  userButtonPressed = 0;

	   	  }
	   	  break;


	 }
}
