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
* 0.3 260609 IN  Added debugging capability and tuning functions
* 0.4 260610 RF  Added Calibration code for X axis
*******************************************************************************/
#include "main.h"
#include "Servo.h"
#include "Estop.h"
#include "StepperMotorDriver.h"
#include "Joystick.h"

/**
 * @brief Queries TMC to get read data from addresses
 *
 * @param addr 8-bit addr to query
 * @param motor Pointer to the stepper motor driver structure.
 *
 * @return uint32_t data from queried address.
 */
uint32_t TMC_SPI_Read32(uint8_t addr, stepper_motor_driver_t *motor)
{
    uint8_t tx[5] = {0};
    uint8_t rx[5] = {0};

    // First transfer: send read address
    tx[0] = addr & 0x7F;
    tx[1] = 0;
    tx[2] = 0;
    tx[3] = 0;
    tx[4] = 0;

    HAL_GPIO_WritePin(motor->NCS_Port, motor->NCS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(motor->hspi, tx, rx, 5, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(motor->NCS_Port, motor->NCS_Pin, GPIO_PIN_SET);

    // Second transfer: clock out response
    memset(rx, 0, sizeof(rx));

    HAL_GPIO_WritePin(motor->NCS_Port, motor->NCS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(motor->hspi, tx, rx, 5, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(motor->NCS_Port, motor->NCS_Pin, GPIO_PIN_SET);

    return ((uint32_t)rx[1] << 24) |
           ((uint32_t)rx[2] << 16) |
           ((uint32_t)rx[3] << 8)  |
           ((uint32_t)rx[4]);
}

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
 * @brief Initializes UART for viewing Registers in the TMC Motor Driver
 *
 * @param motor Pointer to the stepper motor driver structure.
 */
void TMC_PrintDebug(stepper_motor_driver_t *motor)
{
    uint32_t pwm_scale  = TMC_SPI_Read32(PWM_SCALE, motor);
    uint32_t pwm_auto   = TMC_SPI_Read32(PWM_AUTO, motor);
    uint32_t drv_status = TMC_SPI_Read32(DRV_STATUS, motor);

    uint8_t pwm_ofs_auto   =  pwm_auto        & 0xFF;
    uint8_t pwm_grad_auto  = (pwm_auto >> 16) & 0xFF;

    uint8_t pwm_scale_sum  =  pwm_scale       & 0xFF;
    int16_t pwm_scale_auto = (int16_t)((pwm_scale >> 16) & 0x1FF);

    char msg[200];

    snprintf(msg, sizeof(msg),
        "PWM_AUTO=0x%08lX OFS=%u GRAD=%u\r\n"
        "PWM_SCALE=0x%08lX SUM=%u AUTO=%d\r\n"
        "DRV_STATUS=0x%08lX\r\n\r\n",
        pwm_auto,
        pwm_ofs_auto,
        pwm_grad_auto,
        pwm_scale,
        pwm_scale_sum,
        pwm_scale_auto,
        drv_status
    );

    HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

/**
 * @brief Initializes the TMC stepper motor driver. Will update
 * finalize parameters once tuning is finished.
 *
 * @param motor Pointer to the stepper motor driver structure.
 */
void TMC_Init(stepper_motor_driver_t *motor)
{
	TMC_SPI_Send40(0xEC000100C3, motor);  // CHOPCONF: TOFF=3, HSTRT=4, HEND=1, TBL=2, CHM=0 (SpreadCycle)
	TMC_SPI_Send40(0x9000061F0A, motor); // IHOLD_IRUN: IHOLD=10, IRUN=31 (max. current), IHOLDDELAY=6
	TMC_SPI_Send40(0x910000000A, motor); // TPOWERDOWN=10: Delay before power down in stand still
	TMC_SPI_Send40(0x8000000004, motor); // EN_PWM_MODE=1 enables StealthChop (with default PWMCONF)
	TMC_SPI_Send40(0x93000001F4, motor); // TPWM_THRS=500 yields a switching velocity about 35000 = ca. 30RPM

    // TPWM_THRS=500 yields a switching velocity about 35000 = ca. 30RPM
    //TMC_SPI_Send40(0x93000001F4, motor);

    // Ramp generator setup
    TMC_SPI_Send40(packet(A1W, 1000), motor); // A1 1000
    TMC_SPI_Send40(packet(V1W, 0), motor); // V1
	 TMC_SPI_Send40(packet(AMAXW, 500), motor); // AMAX 500
	 TMC_SPI_Send40(packet(VMAXW, 0), motor); // VMAX
	 TMC_SPI_Send40(packet(DMAXW, 10000), motor); // DMAX 700
	 TMC_SPI_Send40(packet(D1W, 1400), motor); // D1 1400
	 TMC_SPI_Send40(packet(VSTARTW, 0), motor); // VSTART
	 TMC_SPI_Send40(packet(VSTOPW, 10), motor); // VSTOP
	 TMC_SPI_Send40(packet(RAMPMODEW, RAMPMODE_POSITION), motor); // Pos mode

}

/**
 * @brief Once in Joystick mode, sets each velocity and accelleration
 *
 * @param motor Pointer to the stepper motor driver structure.
 */
void Set_Motor_Params(stepper_motor_driver_t *motor)
{

    // Ramp generator setup
    TMC_SPI_Send40(packet(A1W, 1000), motor); // A1 1000
    TMC_SPI_Send40(packet(V1W, 0), motor); // V1
	 TMC_SPI_Send40(packet(AMAXW, 2000), motor); // AMAX 500
	 TMC_SPI_Send40(packet(VMAXW, 0), motor); // VMAX
	 TMC_SPI_Send40(packet(DMAXW, 20000), motor); // DMAX 700
	 TMC_SPI_Send40(packet(D1W, 0), motor); // D1 1400
	 TMC_SPI_Send40(packet(VSTARTW, 0), motor); // VSTART
	 TMC_SPI_Send40(packet(VSTOPW, 10), motor); // VSTOP
	 TMC_SPI_Send40(packet(RAMPMODEW, RAMPMODE_POSITION), motor); // Pos mode

}

/**
 * @brief Tunes a motor according to AT1 and AT2 procedures
 *
 * @param motor Pointer to the stepper motor driver structure.
 */
void TMC_Tune(stepper_motor_driver_t *motor)
{
    // IHOLD_IRUN: IHOLD=10, IRUN=31 (max. current), IHOLDDELAY=6
    TMC_SPI_Send40(0x9000061F0A, motor);
    // TPOWERDOWN=10: Delay before power down in stand still
    TMC_SPI_Send40(0x910000000A, motor);
    //EN_PWM_MODE=1 enables StealthChop (with default PWMCONF)
    TMC_SPI_Send40(0x8000000004, motor);
    //pwm_autoscale = 1, pwm_autograd = 1, pwm_freq = 35 kHz
    TMC_SPI_Send40(packet(PWMCONFW, (1u << 16) | (1u<<18) | (1u<<19)), motor);
    //CHOPCONF TOFF = 5, TBL = 2, HSTART = 4, HEND = 0
    TMC_SPI_Send40(packet(CHOPCONFW, 65605), motor);

    //Configure AT Tuning
    TMC_SPI_Send40(packet(VSTARTW, 10), motor);
    TMC_SPI_Send40(packet(V1W, 0), motor);
    TMC_SPI_Send40(packet(VMAXW, 71600), motor);


    // TPWM_THRS=500 yields a switching velocity about 35000 = ca. 30RPM
    //TMC_SPI_Send40(0x93000001F4, motor);

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
 * @brief Translates steps to rotations/100.
 *
 * @param steps amount of rotations desired
 *
 * @return uint32_t steps where 100 steps = 1 rotation
 */
uint32_t TMC_Pos(uint32_t steps)
{
    return steps*512;
}

/**
 * @brief Runs a slow-motion test command on the stepper motor to test what
 * settings are the best for the motors according to the TMC5160 datasheet
 *
 * @param motor Pointer to the stepper motor driver structure.
 * @param A1 First accel between Vstart and V1.
 * @param Amax Second Accel between Vstart and V1.
 * @param V1 First de/accel phase threshold velocity.
 * @param Vmax Motion Ramp target velocity.
 * @param D1 Deceleration between V1 and Vstop.
 * @param Dmax Deceleration between Vmax and V1.
 */
void TMC_RunSlowMotionTest(stepper_motor_driver_t *motor, uint32_t A1,
									uint32_t Amax, uint32_t V1, uint32_t Vmax,
									uint32_t D1, uint32_t Dmax)
{
	TMC_SPI_Send40(packet(RAMPMODEW, RAMPMODE_POSITION), motor); // Pos mode
	TMC_SPI_Send40(packet(TZEROWAITW, 1u<<15), motor); // Wait 1s between moves

	TMC_SPI_Send40(packet(XTARGETW,TMC_Pos(100)), motor); // XTARGET = -51200
}

/**
 * @brief Stop the Motor Driver
 *
 * @param motor Pointer to the stepper motor driver structure.
 */
void TMC_Stop(stepper_motor_driver_t *motor)
{
	 TMC_SPI_Send40(packet(VMAXW, 0), motor);
    //TMC_SPI_Send40(packet(RAMPMODEW, RAMPMODE_HOLD), motor);
}

/**
 * @brief Clear Ramp Status
 *
 * @param motor Pointer to the stepper motor driver structure.
 */
void TMC_ClearRampStatus(stepper_motor_driver_t *motor)
{
    /*
     * RAMP_STAT contains clear-on-write event bits.
     * Writing 1s clears the latched status flags.
     */

    TMC_SPI_Send40(packet(RAMP_STATW, 0x000000FF), motor);
}

/**
 * @brief Reset Position
 *
 * @param motor Pointer to the stepper motor driver structure.
 */
void TMC_ResetPosition(stepper_motor_driver_t *motor)
{
	TMC_SPI_Send40(packet(RAMPMODEW, RAMPMODE_HOLD), motor);
	TMC_SPI_Send40(packet(XACTUALW, 0), motor);
	TMC_SPI_Send40(packet(XTARGETW, 0), motor);
}

/**
 * @brief Moves to the right while Rampmode is in velocity mode
 *
 * @param motor Pointer to the stepper motor driver structure.
 *
 * @return int32_t position of the TMC
 */
int32_t TMC_GetPosition(stepper_motor_driver_t *motor)
{
    return (int32_t)TMC_SPI_Read32(XACTUALW, motor);
}

/**
 * @brief Moves to the right while Rampmode is in velocity mode
 *
 * @param motor Pointer to the stepper motor driver structure.
 */
void TMC_MoveVelocityRight(stepper_motor_driver_t *motor)
{
    TMC_ClearRampStatus(motor);
    TMC_SPI_Send40(packet(VMAXW, CALIBRATION_SPEED), motor); // VMAX
    TMC_SPI_Send40(packet(RAMPMODEW, RAMPMODE_VEL_POS), motor);
}

/**
 * @brief Moves to the left
 *
 * @param motor Pointer to the stepper motor driver structure.
 */
void TMC_MoveVelocityLeft(stepper_motor_driver_t *motor)
{
    TMC_ClearRampStatus(motor);
    TMC_SPI_Send40(packet(VMAXW, CALIBRATION_SPEED), motor); // VMAX
    TMC_SPI_Send40(packet(RAMPMODEW, RAMPMODE_VEL_NEG), motor);
}

/**
 * @brief Activates Right switch
 *
 * @param motor Pointer to the stepper motor driver structure.
 *
 * @return bool
 */

bool TMC_RightSwitchActive(stepper_motor_driver_t *motor)
{
    uint32_t rampStat;

    rampStat = TMC_SPI_Read32(RAMP_STATW, motor);

    return (rampStat & (1 << 0)) != 0;
}

/**
 * @brief Activates Left switch
 *
 * @param motor Pointer to the stepper motor driver structure.
 *
 * @return bool
 */
bool TMC_LeftSwitchActive(stepper_motor_driver_t *motor)
{
    uint32_t rampStat;

    rampStat = TMC_SPI_Read32(RAMP_STATW, motor);

    return (rampStat & (1 << 1)) != 0;
}

bool TMC_WaitForLeftSwitch(stepper_motor_driver_t *motor,
                           uint32_t timeout_ms)
{
    uint32_t startTick;

    startTick = HAL_GetTick();

    while (!TMC_LeftSwitchActive(motor))
    {
        if ((HAL_GetTick() - startTick) > timeout_ms)
        {
            TMC_Stop(motor);
            return false;
        }

        if (eStopPressed == 1)
        {
       	 Emergency_Stop_One(motor);
        }
    }

    TMC_Stop(motor);
    return true;
}

bool TMC_WaitForRightSwitch(stepper_motor_driver_t *motor,
                            uint32_t timeout_ms)
{
    uint32_t startTick;

    startTick = HAL_GetTick();

    while (!TMC_RightSwitchActive(motor))
    {
        if ((HAL_GetTick() - startTick) > timeout_ms)
        {
            TMC_Stop(motor);
            return false;
        }

        if (eStopPressed == 1)
        {
       	 Emergency_Stop_One(motor);
        }
    }

    TMC_Stop(motor);
    return true;
}

/**
 * @brief Control the robot using the Joysticks
 *
 * @param motor Pointer to the stepper motor driver structure.
 * @param timeout_ms time elapsed - timeout if too long
 *
 * @return bool
 */
bool TMC_WaitForPositionReached(stepper_motor_driver_t *motor,
                                uint32_t timeout_ms)
{
    uint32_t startTick;

    startTick = HAL_GetTick();

    while (!TMC_PositionReached(motor))
    {
        if ((HAL_GetTick() - startTick) > timeout_ms)
        {
            TMC_Stop(motor);
            return false;
        }
    }

    TMC_Stop(motor);
    return true;
}

bool TMC_PositionReached(stepper_motor_driver_t *motor)
{
    uint32_t rampStat;

    rampStat = TMC_SPI_Read32(RAMP_STATW, motor);

    return (rampStat & (1 << 9)) != 0;
}

/**
 * @brief Control the robot using the Joysticks
 *
 * @param motor Pointer to the stepper motor driver structure.
 * @param targetPosition desired X-Coordinate
 */
void TMC_MoveToPosition(stepper_motor_driver_t *motor,
                        int32_t targetPosition)
{
    TMC_ClearRampStatus(motor);
    TMC_SPI_Send40(packet(RAMPMODEW,RAMPMODE_POSITION), motor);
    TMC_SPI_Send40(packet(VMAXW, CALIBRATION_SPEED), motor);
    TMC_SPI_Send40(packet(XTARGETW, (uint32_t)targetPosition), motor);
}

/**
 * @brief Control the robot using the Joysticks
 *
 * @param motor Pointer to the stepper motor driver structure.
 * @param joy_axis ADC value from joystick.
 */
void TMC_JoystickCommand(stepper_motor_driver_t *motor, uint16_t joy_axis)
{
	if (joy_axis > 2010 + 1000)
	{
		TMC_SPI_Send40(packet(RAMPMODEW, RAMPMODE_VEL_NEG), motor);
		TMC_SPI_Send40(packet(VMAXW, 15000), motor); //prev (joy_axis - 2048)*100
	}
	else if (joy_axis < 2010 - 1000)
	{
		TMC_SPI_Send40(packet(RAMPMODEW, RAMPMODE_VEL_POS), motor);
		TMC_SPI_Send40(packet(VMAXW, 15000), motor); //prev -1*(joy_axis - 2048)*100), motor
	}
	else
	{
		TMC_SPI_Send40(packet(VMAXW, 0), motor);
	}

}

/**
 * @brief Set the robot to a known center position
 *
 * @param motor Pointer to the stepper motor driver structure.
 */
void X_Homing(stepper_motor_driver_t *motor)
{
    char msg[120];
    int32_t rightPosition;
    int32_t centerPosition;
    int32_t finalPosition;

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)"Starting base homing test\r\n",
                      strlen("Starting base homing test\r\n"),
                      HAL_MAX_DELAY);

    TMC_Stop(motor);
    HAL_Delay(100);

    TMC_ClearRampStatus(motor);
    HAL_Delay(20);

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)"Moving toward left switch REFL...\r\n",
                      strlen("Moving toward left switch REFL...\r\n"),
                      HAL_MAX_DELAY);

    TMC_MoveVelocityLeft(motor);

    if (!TMC_WaitForLeftSwitch(motor, 25000))
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t*)"ERROR: Left switch timeout\r\n",
                          strlen("ERROR: Left switch timeout\r\n"),
                          HAL_MAX_DELAY);
        Emergency_Stop_One(motor);
        return;
    }

    TMC_Stop(motor);

    HAL_Delay(200);

    TMC_ResetPosition(motor);
    HAL_Delay(100);

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)"Left switch found. Position reset to 0.\r\n",
                      strlen("Left switch found. Position reset to 0.\r\n"),
                      HAL_MAX_DELAY);

    TMC_ClearRampStatus(motor);
    HAL_Delay(200);

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)"Moving toward right switch REFR...\r\n",
                      strlen("Moving toward right switch REFR...\r\n"),
                      HAL_MAX_DELAY);

    TMC_MoveVelocityRight(motor);

    if (!TMC_WaitForRightSwitch(motor, 25000))
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t*)"ERROR: Right switch timeout\r\n",
                          strlen("ERROR: Right switch timeout\r\n"),
                          HAL_MAX_DELAY);
        Emergency_Stop_One(motor);
        return;
    }

    TMC_Stop(motor);
    HAL_Delay(200);

    rightPosition = TMC_GetPosition(motor);
    centerPosition = rightPosition / 2;

    sprintf(msg,
            "Right switch found. XACTUAL: %ld\r\nCenter target: %ld\r\n",
            (long)rightPosition,
            (long)centerPosition);

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)msg,
                      strlen(msg),
                      HAL_MAX_DELAY);

    TMC_ClearRampStatus(motor);
    HAL_Delay(200);

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)"Moving to center/home position...\r\n",
                      strlen("Moving to center/home position...\r\n"),
                      HAL_MAX_DELAY);

    TMC_MoveToPosition(motor, centerPosition);

    if (!TMC_WaitForPositionReached(motor, 20000))
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t*)"ERROR: Center move timeout\r\n",
                          strlen("ERROR: Center move timeout\r\n"),
                          HAL_MAX_DELAY);
        Emergency_Stop_One(motor);
        return;
    }

    TMC_Stop(motor);
    HAL_Delay(200);

    finalPosition = TMC_GetPosition(motor);

    sprintf(msg,
            "Reached center. XACTUAL before reset: %ld\r\n",
            (long)finalPosition);

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)msg,
                      strlen(msg),
                      HAL_MAX_DELAY);

    TMC_ResetPosition(motor);

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)"Base homing complete. Center is now home zero.\r\n",
                      strlen("Base homing complete. Center is now home zero.\r\n"),
                      HAL_MAX_DELAY);
}

/**
 * @brief Set the robot to a known extension position using 1 limit switch.
 *
 * @param motor Pointer to the stepper motor driver structure.
 * @param HomeOffsetSteps Number of steps from limit switch to call home
 */
void Y_Homing(stepper_motor_driver_t *motor, int32_t HomeOffsetSteps)
{
    char msg[120];
    int32_t switchPosition;
    int32_t homeTarget;
    int32_t finalPosition;

    //Change this value to adjust where the height home position is
    //after the rear limit switch is found by putty.*/
    //int32_t heightHomeOffsetSteps = 31902;

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)"Starting height homing test\r\n",
                      strlen("Starting height homing test\r\n"),
                      HAL_MAX_DELAY);

    TMC_Stop(motor);
    HAL_Delay(100);

    TMC_ClearRampStatus(motor);
    HAL_Delay(20);

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)"Moving height toward rear switch REFR...\r\n",
                      strlen("Moving height toward rear switch REFR...\r\n"),
                      HAL_MAX_DELAY);

  TMC_MoveVelocityRight(motor);

    if (!TMC_WaitForRightSwitch(motor, 10000))
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t*)"ERROR: Height rear switch timeout\r\n",
                          strlen("ERROR: Height rear switch timeout\r\n"),
                          HAL_MAX_DELAY);
        Emergency_Stop_One(motor);
        return;
    }

    TMC_Stop(motor);
    HAL_Delay(200);

    switchPosition = TMC_GetPosition(motor);

    sprintf(msg,
            "Height rear switch found. XACTUAL: %ld\r\n",
            (long)switchPosition);

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)msg,
                      strlen(msg),
                      HAL_MAX_DELAY);



//Set the rear switch position as zero.
  TMC_ResetPosition(motor);
  HAL_Delay(100);

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)"Height rear switch position reset to 0.\r\n",
                      strlen("Height rear switch position reset to 0.\r\n"),
                      HAL_MAX_DELAY);

    TMC_ClearRampStatus(motor);
    HAL_Delay(200);



//Move to adjustable home position
  homeTarget = HomeOffsetSteps;

    sprintf(msg,
            "Moving height to home offset: %ld steps\r\n",
            (long)homeTarget);

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)msg,
                      strlen(msg),
                      HAL_MAX_DELAY);

    TMC_MoveToPosition(motor, homeTarget);

    if (!TMC_WaitForPositionReached(motor, 10000))
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t*)"ERROR: Height home offset timeout\r\n",
                          strlen("ERROR: Height home offset timeout\r\n"),
                          HAL_MAX_DELAY);
        Emergency_Stop_One(motor);
        return;
    }

    TMC_Stop(motor);
    HAL_Delay(200);

    finalPosition = TMC_GetPosition(motor);

    sprintf(msg,
            "Height home reached. XACTUAL before reset: %ld\r\n",
            (long)finalPosition);

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)msg,
                      strlen(msg),
                      HAL_MAX_DELAY);

    //Current height is now home zero.
    TMC_ResetPosition(motor);

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)"Height homing complete. Current height is now home zero.\r\n",
                      strlen("Height homing complete. Current height is now home zero.\r\n"),
                      HAL_MAX_DELAY);
}

/**
 * @brief Set the robot to a known height position using 1 limit switch.
 *
 * @param motor Pointer to the stepper motor driver structure.
 * @param HomeOffsetSteps Number of steps from limit switch to call home
 */
void Z_Homing(stepper_motor_driver_t *motor, int32_t HomeOffsetSteps)
{
    char msg[120];
    int32_t switchPosition;
    int32_t homeTarget;
    int32_t finalPosition;

    //Change this value to adjust where the height home position is
    //after the rear limit switch is found by putty.*/
    //int32_t heightHomeOffsetSteps = 31902;

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)"Starting height homing test\r\n",
                      strlen("Starting height homing test\r\n"),
                      HAL_MAX_DELAY);

    TMC_Stop(motor);
    HAL_Delay(100);

    TMC_ClearRampStatus(motor);
    HAL_Delay(20);

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)"Moving height toward rear switch REFR...\r\n",
                      strlen("Moving height toward rear switch REFR...\r\n"),
                      HAL_MAX_DELAY);

  TMC_MoveVelocityLeft(motor);

    if (!TMC_WaitForRightSwitch(motor, 10000))
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t*)"ERROR: Height rear switch timeout\r\n",
                          strlen("ERROR: Height rear switch timeout\r\n"),
                          HAL_MAX_DELAY);
        Emergency_Stop_One(motor);
        return;
    }

    TMC_Stop(motor);
    HAL_Delay(200);

    switchPosition = TMC_GetPosition(motor);

    sprintf(msg,
            "Height rear switch found. XACTUAL: %ld\r\n",
            (long)switchPosition);

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)msg,
                      strlen(msg),
                      HAL_MAX_DELAY);



//Set the rear switch position as zero.
  TMC_ResetPosition(motor);
  HAL_Delay(100);

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)"Height rear switch position reset to 0.\r\n",
                      strlen("Height rear switch position reset to 0.\r\n"),
                      HAL_MAX_DELAY);

    TMC_ClearRampStatus(motor);
    HAL_Delay(200);



//Move to adjustable home position
  homeTarget = HomeOffsetSteps;

    sprintf(msg,
            "Moving height to home offset: %ld steps\r\n",
            (long)homeTarget);

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)msg,
                      strlen(msg),
                      HAL_MAX_DELAY);

    TMC_MoveToPosition(motor, homeTarget);

    if (!TMC_WaitForPositionReached(motor, 10000))
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t*)"ERROR: Height home offset timeout\r\n",
                          strlen("ERROR: Height home offset timeout\r\n"),
                          HAL_MAX_DELAY);
        Emergency_Stop_One(motor);
        return;
    }

    TMC_Stop(motor);
    HAL_Delay(200);

    finalPosition = TMC_GetPosition(motor);

    sprintf(msg,
            "Height home reached. XACTUAL before reset: %ld\r\n",
            (long)finalPosition);

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)msg,
                      strlen(msg),
                      HAL_MAX_DELAY);

    //Current height is now home zero.
    TMC_ResetPosition(motor);

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)"Height homing complete. Current height is now home zero.\r\n",
                      strlen("Height homing complete. Current height is now home zero.\r\n"),
                      HAL_MAX_DELAY);
}
