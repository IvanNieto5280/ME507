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

/* TMC5160 Register Definitions */
#define TMC_RAMPMODE        0x20
#define TMC_XACTUAL         0x21
#define TMC_XTARGET         0x2D
#define TMC_SW_MODE         0x34
#define TMC_RAMP_STAT       0x35

// speed conversion defines
#define TMC_A1              0x24
#define TMC_V1              0x25
#define TMC_AMAX            0x26
#define TMC_VMAX            0x27
#define TMC_DMAX            0x28
#define TMC_D1              0x2A

#define TMC_WRITE_BIT       0x80u
#define TMC_WRITE_ADDR(addr) ((uint8_t)((addr) | TMC_WRITE_BIT))

#define TMC_PACKET(addr, value) \
    ((((uint64_t)TMC_WRITE_ADDR(addr)) << 32) | ((uint32_t)(value)))

/* TMC5160 RAMPMODE Values */
#define TMC_RAMPMODE_POSITION      0
#define TMC_RAMPMODE_VEL_POS       1
#define TMC_RAMPMODE_VEL_NEG       2
#define TMC_RAMPMODE_HOLD          3

void TMC_SetMaxVelocity(stepper_motor_driver_t *motor, uint32_t velocity)
	{
	    TMC_SPI_Send40(TMC_PACKET(TMC_VMAX, velocity), motor);
	}

void TMC_SetMaxAcceleration(stepper_motor_driver_t *motor, uint32_t acceleration)
	{
	    TMC_SPI_Send40(TMC_PACKET(TMC_AMAX, acceleration), motor);
	}

void TMC_SetMaxDeceleration(stepper_motor_driver_t *motor, uint32_t deceleration)
	{
	    TMC_SPI_Send40(TMC_PACKET(TMC_DMAX, deceleration), motor);
	}

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
* TMC_WriteRegister - Writes a 32bit value to a TMC5160 register
* Inputs: register address, data, and motor pointer
* Output: none
* To do:  None :)
* -------------------------------------------------------------------------- */
void TMC_WriteRegister(uint8_t address,
                       uint32_t data,
                       stepper_motor_driver_t *motor)
{
    uint64_t datagram;

    datagram = 0;
    datagram |= ((uint64_t)(address | 0x80) << 32);
    datagram |= data;

    TMC_SPI_Send40(datagram, motor);
}

/* ---------------------------- ( NOTE: 80 chars long ) ------------------------
* TMC_ReadRegister - Reads a 32bit value from a TMC5160 register
* Inputs: register address and motor pointer
* Output: register value
* To do:  None :)
* -------------------------------------------------------------------------- */
uint32_t TMC_ReadRegister(uint8_t address,
                          stepper_motor_driver_t *motor)
{
    uint8_t tx[5] = {0};
    uint8_t rx[5] = {0};

    tx[0] = address & 0x7F;

    /*
     * The TMC5160 returns read data on the following SPI transfer.
     * Therefore, the first transfer sends the read request, and the
     * second transfer receives the requested data.
     */

    HAL_GPIO_WritePin(motor->NCS_Port,
                      motor->NCS_Pin,
                      GPIO_PIN_RESET);

    HAL_SPI_TransmitReceive(motor->hspi,
                            tx,
                            rx,
                            5,
                            HAL_MAX_DELAY);

    HAL_GPIO_WritePin(motor->NCS_Port,
                      motor->NCS_Pin,
                      GPIO_PIN_SET);

    HAL_GPIO_WritePin(motor->NCS_Port,
                      motor->NCS_Pin,
                      GPIO_PIN_RESET);

    HAL_SPI_TransmitReceive(motor->hspi,
                            tx,
                            rx,
                            5,
                            HAL_MAX_DELAY);

    HAL_GPIO_WritePin(motor->NCS_Port,
                      motor->NCS_Pin,
                      GPIO_PIN_SET);

    return ((uint32_t)rx[1] << 24) |
           ((uint32_t)rx[2] << 16) |
           ((uint32_t)rx[3] << 8)  |
           ((uint32_t)rx[4]);
}

/* ---------------------------- ( NOTE: 80 chars long ) ------------------------
* TMC_init - Initializes the motor as stated in page 116
* Inputs: none
* Output: none
* To do:  None :)
* -------------------------------------------------------------------------- */
void TMC_Init(stepper_motor_driver_t *motor)
{
    /* Chopper / driver setup */
    TMC_SPI_Send40(0xEC000100C3, motor); // CHOPCONF
    TMC_SPI_Send40(0x9000061F0A, motor); // IHOLD_IRUN
    TMC_SPI_Send40(0x910000000A, motor); // TPOWERDOWN
    TMC_SPI_Send40(0x8000000004, motor); // GCONF: EN_PWM_MODE
    TMC_SPI_Send40(0x93000001F4, motor); // TPWM_THRS

    /* Ramp generator setup */
    TMC_SPI_Send40(TMC_PACKET(TMC_A1,    1000), motor);  // A1
    TMC_SPI_Send40(TMC_PACKET(TMC_V1,   10000), motor);  // V1
    TMC_SPI_Send40(TMC_PACKET(TMC_AMAX,  1000), motor);  // AMAX
    TMC_SPI_Send40(TMC_PACKET(TMC_VMAX, 10000), motor);  // VMAX
    TMC_SPI_Send40(TMC_PACKET(TMC_DMAX,  1000), motor);  // DMAX
    TMC_SPI_Send40(TMC_PACKET(TMC_D1,    1000), motor);  // D1
    TMC_SPI_Send40(0xAB0000000A, motor);                 // VSTOP

    /* Enable REFL and REFR as reference switch inputs */
    TMC_EnableReferenceSwitches(motor);

    /* Clear any stored switch or ramp status flags */
    TMC_ClearRampStatus(motor);

    TMC_WriteRegister(TMC_XACTUAL, 0x00000000, motor);
    TMC_WriteRegister(TMC_XTARGET, 0x00000000, motor);
    TMC_WriteRegister(TMC_RAMPMODE, TMC_RAMPMODE_HOLD, motor);
}

/* ---------------------------- ( NOTE: 80 chars long ) ------------------------
* TMC_EnableReferenceSwitches - Enables REFL and REFR switch monitoring
* Inputs: motor pointer
* Output: none
* To do:  None :)
* -------------------------------------------------------------------------- */
void TMC_EnableReferenceSwitches(stepper_motor_driver_t *motor)
{
    /*
     * SW_MODE = 0xA3
     *
     * bit 0 = stop_l_enable
     * bit 1 = stop_r_enable
     * bit 5 = latch_l_active
     * bit 7 = latch_r_active
     * pol_stop_l = 0
     * pol_stop_r = 0
     *
     * use this for normally closed switches where:
     * normal state = LOW
     * triggered/open state = HIGH
     */

    TMC_WriteRegister(TMC_SW_MODE, 0x000000A3, motor);
}

/* ---------------------------- ( NOTE: 80 chars long ) ------------------------
* TMC_ClearRampStatus - Clears latched ramp/reference switch events
* Inputs: motor pointer
* Output: none
* To do:  None :)
* -------------------------------------------------------------------------- */
void TMC_ClearRampStatus(stepper_motor_driver_t *motor)
{
    /*
     * RAMP_STAT contains clear-on-write event bits.
     * Writing 1s clears the latched status flags.
     */

    TMC_WriteRegister(TMC_RAMP_STAT, 0x000000FF, motor);
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

/* ---------------------------- ( NOTE: 80 chars long ) ------------------------
* TMC_MoveOneRotationRight - test command to move motor in other direction
* Inputs: none
* Output: none
* To do:  None :)
* -------------------------------------------------------------------------- */
void TMC_MoveOneRotationRight(stepper_motor_driver_t *motor)
{
    TMC_SPI_Send40(0xAD0000C800, motor); // XTARGET = 51200
}

/* ---------------------------- ( NOTE: 80 chars long ) ------------------------
* TMC_MoveVelocityLeft - Moves motor continuously in negative direction
* Inputs: motor pointer
* Output: none
* To do:  None :)
* -------------------------------------------------------------------------- */
void TMC_MoveVelocityLeft(stepper_motor_driver_t *motor)
{
    TMC_ClearRampStatus(motor);
    TMC_WriteRegister(TMC_RAMPMODE, TMC_RAMPMODE_VEL_NEG, motor);
}

/* ---------------------------- ( NOTE: 80 chars long ) ------------------------
* TMC_MoveVelocityRight - Moves motor continuously in positive direction
* Inputs: motor pointer
* Output: none
* To do:  None :)
* -------------------------------------------------------------------------- */
void TMC_MoveVelocityRight(stepper_motor_driver_t *motor)
{
    TMC_ClearRampStatus(motor);
    TMC_WriteRegister(TMC_RAMPMODE, TMC_RAMPMODE_VEL_POS, motor);
}

/* ---------------------------- ( NOTE: 80 chars long ) ------------------------
* TMC_Stop - Stops motor by placing ramp generator in hold mode
* Inputs: motor pointer
* Output: none
* To do:  None :)
* -------------------------------------------------------------------------- */
void TMC_Stop(stepper_motor_driver_t *motor)
{
    TMC_WriteRegister(TMC_RAMPMODE, TMC_RAMPMODE_HOLD, motor);
}

/* ---------------------------- ( NOTE: 80 chars long ) ------------------------
* TMC_MoveToPosition - Moves motor to target position using ramp generator
* Inputs: target position and motor pointer
* Output: none
* To do:  None :)
* -------------------------------------------------------------------------- */
void TMC_MoveToPosition(stepper_motor_driver_t *motor,
                        int32_t targetPosition)
{
    TMC_ClearRampStatus(motor);
    TMC_WriteRegister(TMC_RAMPMODE, TMC_RAMPMODE_POSITION, motor);
    TMC_WriteRegister(TMC_XTARGET, (uint32_t)targetPosition, motor);
}

/* ---------------------------- ( NOTE: 80 chars long ) ------------------------
* TMC_ResetPosition - Sets XACTUAL to zero
* Inputs: motor pointer
* Output: none
* To do:  None :)
* -------------------------------------------------------------------------- */
void TMC_ResetPosition(stepper_motor_driver_t *motor)
{
    TMC_WriteRegister(TMC_RAMPMODE, TMC_RAMPMODE_HOLD, motor);
    TMC_WriteRegister(TMC_XACTUAL, 0x00000000, motor);
    TMC_WriteRegister(TMC_XTARGET, 0x00000000, motor);
}

/* ---------------------------- ( NOTE: 80 chars long ) ------------------------
* TMC_GetPosition - Reads XACTUAL position from TMC5160
* Inputs: motor pointer
* Output: motor position
* To do:  None :)
* -------------------------------------------------------------------------- */
int32_t TMC_GetPosition(stepper_motor_driver_t *motor)
{
    return (int32_t)TMC_ReadRegister(TMC_XACTUAL, motor);
}

/* ---------------------------- ( NOTE: 80 chars long ) ------------------------
* TMC_LeftSwitchActive - Checks REFL switch status
* Inputs: motor pointer
* Output: true if left reference switch is active
* To do:  None :)
* -------------------------------------------------------------------------- */
bool TMC_LeftSwitchActive(stepper_motor_driver_t *motor)
{
    uint32_t rampStat;

    rampStat = TMC_ReadRegister(TMC_RAMP_STAT, motor);

    return (rampStat & (1 << 0)) != 0;
}

/* ---------------------------- ( NOTE: 80 chars long ) ------------------------
* TMC_RightSwitchActive - Checks REFR switch status
* Inputs: motor pointer
* Output: true if right reference switch is active
* To do:  None :)
* -------------------------------------------------------------------------- */
bool TMC_RightSwitchActive(stepper_motor_driver_t *motor)
{
    uint32_t rampStat;

    rampStat = TMC_ReadRegister(TMC_RAMP_STAT, motor);

    return (rampStat & (1 << 1)) != 0;
}

/* ---------------------------- ( NOTE: 80 chars long ) ------------------------
* TMC_PositionReached - Checks if the target position has been reached
* Inputs: motor pointer
* Output: true if position reached
* To do:  None :)
* -------------------------------------------------------------------------- */
bool TMC_PositionReached(stepper_motor_driver_t *motor)
{
    uint32_t rampStat;

    rampStat = TMC_ReadRegister(TMC_RAMP_STAT, motor);

    return (rampStat & (1 << 9)) != 0;
}

/* ---------------------------- ( NOTE: 80 chars long ) ------------------------
* TMC_WaitForLeftSwitch - Waits until REFL switch is triggered
* Inputs: motor pointer and timeout in ms
* Output: true if switch was reached, false if timeout occurred
* To do:  None :)
* -------------------------------------------------------------------------- */
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
    }

    TMC_Stop(motor);
    return true;
}

/* ---------------------------- ( NOTE: 80 chars long ) ------------------------
* TMC_WaitForRightSwitch - Waits until REFR switch is triggered
* Inputs: motor pointer and timeout in ms
* Output: true if switch was reached, false if timeout occurred
* To do:  None :)
* -------------------------------------------------------------------------- */
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
    }

    TMC_Stop(motor);
    return true;
}

/* ---------------------------- ( NOTE: 80 chars long ) ------------------------
* TMC_WaitForPositionReached - Waits until target position is reached
* Inputs: motor pointer and timeout in ms
* Output: true if position was reached, false if timeout occurred
* To do:  None :)
* -------------------------------------------------------------------------- */
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
