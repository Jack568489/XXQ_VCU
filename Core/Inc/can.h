/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.h
  * @brief   This file contains all the function prototypes for
  *          the can.c file
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
#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "adc.h"
#include "vehicle.h"
/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan1;

extern CAN_HandleTypeDef hcan2;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_CAN1_Init(void);
void MX_CAN2_Init(void);

/* USER CODE BEGIN Prototypes */

/* CAN总线初始化: 配置滤波器、启动CAN1/CAN2、使能接收中断 */
void CAN_Init(void);
/* CAN报文发送: hcan-CAN句柄, StdId-标准帧ID, pData-数据指针, DLC-数据长度 */
HAL_StatusTypeDef CAN_SendMessage(CAN_HandleTypeDef *hcan, uint32_t StdId, uint8_t *pData, uint8_t DLC);
/* VCU整车信息上报: 通过CAN1发送VCU状态、档位、车速、油门/刹车/转向数据 */
void CAN_SendVCUInfo(const vehicle_sensors_t *sensors, vcu_state_t vcu_state, gear_state_t gear, uint8_t speed_kmh);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

