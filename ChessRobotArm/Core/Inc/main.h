/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_1
#define LED_GPIO_Port GPIOB
#define EStop_Pin GPIO_PIN_2
#define EStop_GPIO_Port GPIOB
#define DRV_ENN_Pin GPIO_PIN_10
#define DRV_ENN_GPIO_Port GPIOB
#define Diag1_3_Pin GPIO_PIN_12
#define Diag1_3_GPIO_Port GPIOB
#define Diag0_3_Pin GPIO_PIN_8
#define Diag0_3_GPIO_Port GPIOA
#define Diag1_2_Pin GPIO_PIN_9
#define Diag1_2_GPIO_Port GPIOA
#define Diag0_2_Pin GPIO_PIN_10
#define Diag0_2_GPIO_Port GPIOA
#define Diag1_1_Pin GPIO_PIN_11
#define Diag1_1_GPIO_Port GPIOA
#define Diag0_1_Pin GPIO_PIN_12
#define Diag0_1_GPIO_Port GPIOA
#define NCS_3_Pin GPIO_PIN_15
#define NCS_3_GPIO_Port GPIOA
#define NCS_2_Pin GPIO_PIN_4
#define NCS_2_GPIO_Port GPIOB
#define NCS_1_Pin GPIO_PIN_5
#define NCS_1_GPIO_Port GPIOB
#define User_Button_Pin GPIO_PIN_8
#define User_Button_GPIO_Port GPIOB
#define User_Button_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */
#define SERVO_OPEN 25000
#define SERVO_CLOSED 13000

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
