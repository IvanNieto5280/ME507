/*******************************************************************************
* @file           : StepperMotorDriver.h
* @brief          : Header file for StepperMotorDriver.c
* @project        : Chess Robot Arm
* @author         : Ivan Nieto (IN) - inieto@calpoly.edu
* @version        : 0.1
* @date           : 2026-05-30
* @compiler       : STM32CubeIDE v.2.1.1 Build: 28236_20260312_0043 (UTC)
* @target         : STM32F411CEU6
* @attention      : (c) 2023 STMicroelectronics. All rights reserved.
*
* @par Revision History
* 0.1 260530 IN Created Module
*******************************************************************************/

#include <stdbool.h>

#ifndef INC_STEPPERMOTORDRIVER_H_
#define INC_STEPPERMOTORDRIVER_H_

/**
 * @brief Structure containing hardware resources associated with a
 *        TMC stepper motor driver.
 */
typedef struct {
	GPIO_TypeDef *NCS_Port;      /**< Chip-select GPIO port */
	uint32_t NCS_Pin;            /**< Chip-select GPIO pin */

	GPIO_TypeDef *Diag0_Port;    /**< Diagnostic output 0 GPIO port */
	uint32_t Diag0_Pin;          /**< Diagnostic output 0 GPIO pin */

	GPIO_TypeDef *Diag1_Port;    /**< Diagnostic output 1 GPIO port */
	uint32_t Diag1_Pin;          /**< Diagnostic output 1 GPIO pin */

	SPI_HandleTypeDef *hspi;     /**< SPI peripheral handle */
} stepper_motor_driver_t;

// Helper Function to convert address to write address as specified in TMC
// Stepper Motor Driver Data sheet

#define TMC_WRITE_BIT   0x80u

#define TMC_WRITE_ADDR(addr) ((uint8_t)((addr) | TMC_WRITE_BIT))

#define packet(addr, value) \
    ((((uint64_t)TMC_WRITE_ADDR(addr)) << 32) | ((uint32_t)(value)))

// #define registers for readability

#define GCONF 0x00 			//RW

#define RECALIBRATE 0b1				//Bit 0 of CONF
#define FASTSTANDSTILL 0b10		//Bit 1 of CONF
#define EN_PWM_MODE 0b100			//Bit 2 of CONF
#define MULTISTEP_FILT 0b1000		//Bit 3 of CONF



#define GSTAT 					0x01 		//R+WC

#define GLOBALSCALERW 0x0B	//W

#define IHOLD_IRUNW 			0x10		//W
#define TPOWERDOWNW 			0x11		//W
#define TSTEP 					0x12		//R
#define TPWMTHRS 				0x13		//W
#define TCOOLTHRSW 			0x14		//W
#define THIGHW 				0x15		//W

//Ramp Mode and its definitions
#define RAMPMODEW 			0x20 		//RW
#define RAMPMODE_POSITION  0
#define RAMPMODE_VEL_POS   1
#define RAMPMODE_VEL_NEG   2
#define RAMPMODE_HOLD      3

#define XACTUALW 				0x21		//RW
#define VACTUAL 				0x22 		//R
#define VSTARTW 				0x23		//W
#define A1W 					0x24		//W
#define V1W 					0x25		//W
#define AMAXW 					0x26		//W
#define VMAXW 					0x27		//W
#define DMAXW 					0x28		//W
#define D1W 					0x2A		//W
#define VSTOPW 				0x2B		//W
#define TZEROWAITW 			0x2C		//W
#define XTARGETW 				0x2D		//RW


#define RAMP_STATW 			0x35		//RW+C

#define DRV_STATUS 			0x6F		//R

#define CHOPCONFW 			0x6C 		//RW

#define PWMCONFW				0x70		//W

#define PWM_SCALE    		0x71		//R
#define PWM_AUTO		      0x72		//R
#define LOST_STEPS 			0x73		//R


#define CALIBRATION_SPEED 10000

// External Variables

extern stepper_motor_driver_t motor1;
extern stepper_motor_driver_t motor2;
extern stepper_motor_driver_t motor3;

//Function Prototypes

uint32_t TMC_SPI_Read32(uint8_t addr, stepper_motor_driver_t *motor);
void TMC_SPI_Send40(uint64_t datagram, stepper_motor_driver_t *motor);
void TMC_PrintDebug(stepper_motor_driver_t *motor);
void TMC_Init(stepper_motor_driver_t *motor);
void Set_Motor_Params(stepper_motor_driver_t *motor);
void TMC_Tune(stepper_motor_driver_t *motor);
uint32_t TMC_Pos(uint32_t steps);
void TMC_RunSlowMotionTest(stepper_motor_driver_t *motor, uint32_t A1,
									uint32_t Amax, uint32_t V1, uint32_t Vmax,
									uint32_t D1, uint32_t Dmax);
void TMC_MoveOneRotationLeft(stepper_motor_driver_t *motor);
void TMC_Spin1ForwardSpin1Backward(stepper_motor_driver_t *motor);
void TMC_JoystickCommand(stepper_motor_driver_t *motor, uint16_t joy_axis);
void TMC_Stop(stepper_motor_driver_t *motor);
void TMC_ClearRampStatus(stepper_motor_driver_t *motor);
void TMC_ResetPosition(stepper_motor_driver_t *motor);
void TMC_MoveVelocityRight(stepper_motor_driver_t *motor);
void TMC_MoveVelocityLeft(stepper_motor_driver_t *motor);
int32_t TMC_GetPosition(stepper_motor_driver_t *motor);
bool TMC_WaitForPositionReached(stepper_motor_driver_t *motor,
                                uint32_t timeout_ms);
bool TMC_PositionReached(stepper_motor_driver_t *motor);

bool TMC_LeftSwitchActive(stepper_motor_driver_t *motor);
bool TMC_WaitForLeftSwitch(stepper_motor_driver_t *motor,
                           uint32_t timeout_ms);
bool TMC_RightSwitchActive(stepper_motor_driver_t *motor);
bool TMC_WaitForRightSwitch(stepper_motor_driver_t *motor,
                            uint32_t timeout_ms);
bool TMC_WaitForPositionReached(stepper_motor_driver_t *motor,
                                uint32_t timeout_ms);
void TMC_MoveToPosition(stepper_motor_driver_t *motor,
                        int32_t targetPosition);

//void TMC_Calibration(stepper_motor_driver_t *motor);
void X_Homing(stepper_motor_driver_t *motor);
void Y_Homing(stepper_motor_driver_t *motor, int32_t HomeOffsetSteps);
void Z_Homing(stepper_motor_driver_t *motor, int32_t HomeOffsetSteps);

#endif /* INC_STEPPERMOTORDRIVER_H_ */
