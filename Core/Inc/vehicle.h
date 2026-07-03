/**
  ******************************************************************************
  * @file    vehicle.h
  * @brief   整车控制函数头文件
  * @note    包含LED指示灯、档位切换、制动灯/蜂鸣器、UART遥测上报等任务
  ******************************************************************************
  */
#ifndef __VEHICLE_H__
#define __VEHICLE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/*------------------------------------------------------------------------------
 * 硬件操作宏定义
 *------------------------------------------------------------------------------*/
/* LED1控制: 低电平有效 */
#define LED1_ON()               HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET)
#define LED1_OFF()              HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET)

/* 蜂鸣器控制: 高电平有效 */
#define BUZZER_ON()             HAL_GPIO_WritePin(HORN_CTL_GPIO_Port, HORN_CTL_Pin, GPIO_PIN_SET)
#define BUZZER_OFF()            HAL_GPIO_WritePin(HORN_CTL_GPIO_Port, HORN_CTL_Pin, GPIO_PIN_RESET)

/* 制动灯控制: 低电平有效 */
#define BRAKE_LIGHT_ON()        HAL_GPIO_WritePin(LAMP_GPIO_Port, LAMP_Pin, GPIO_PIN_RESET)
#define BRAKE_LIGHT_OFF()       HAL_GPIO_WritePin(LAMP_GPIO_Port, LAMP_Pin, GPIO_PIN_SET)

/* 档位开关读取: 闭合(低电平)返回1, 断开(高电平)返回0 */
#define GET_GEAR_SWITCH()       ((HAL_GPIO_ReadPin(TS_ACT_DET_GPIO_Port, \
                                    TS_ACT_DET_Pin) == GPIO_PIN_RESET) ? 1U : 0U)

/*------------------------------------------------------------------------------
 * 档位状态枚举
 *------------------------------------------------------------------------------*/
typedef enum {
    GEAR_STATE_P = 0,   /* P档: 驻车状态                         */
    GEAR_STATE_D = 1    /* D档: 驾驶状态 (LAMP点亮, 蜂鸣器确认音) */
} gear_state_t;

/*------------------------------------------------------------------------------
 * 函数原型
 *------------------------------------------------------------------------------*/

void Vehicle_Init(void);
void Vehicle_LED1_Blink(void);

/**
  * @brief  档位切换检测 (主循环每次迭代调用)
  * @note   P→D条件: 档位开关闭合(TS_ACT=低) 且 刹车值>10
  *         进入D档时点亮LAMP并驱动蜂鸣器
  */
void Vehicle_GearShiftCheck(void);

/**
  * @brief  档位状态机计时 (每秒调用一次)
  * @note   进入D档后蜂鸣器持续3秒自动关闭, LAMP保持点亮
  */
void Vehicle_GearTick(void);

/**
  * @brief  获取当前档位状态
  * @retval gear_state_t: GEAR_STATE_P 或 GEAR_STATE_D
  */
gear_state_t Vehicle_GetGearState(void);

void Vehicle_SensorReport(void);

#ifdef __cplusplus
}
#endif

#endif /* __VEHICLE_H__ */
