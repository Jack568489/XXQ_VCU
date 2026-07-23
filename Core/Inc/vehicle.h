/**
  ******************************************************************************
  * @file    vehicle.h
  * @brief   整车控制函数头文件
  ******************************************************************************
  */
#ifndef __VEHICLE_H__
#define __VEHICLE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* LED1控制: 低电平有效 */
#define LED1_ON()               HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET)
#define LED1_OFF()              HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET)

/* 蜂鸣器控制: 高电平有效 */
#define BUZZER_ON()             HAL_GPIO_WritePin(HORN_CTL_GPIO_Port, HORN_CTL_Pin, GPIO_PIN_SET)
#define BUZZER_OFF()            HAL_GPIO_WritePin(HORN_CTL_GPIO_Port, HORN_CTL_Pin, GPIO_PIN_RESET)

/* 制动灯控制: 低电平有效 */
#define BRAKE_LIGHT_ON()        HAL_GPIO_WritePin(LAMP_GPIO_Port, LAMP_Pin, GPIO_PIN_RESET)
#define BRAKE_LIGHT_OFF()       HAL_GPIO_WritePin(LAMP_GPIO_Port, LAMP_Pin, GPIO_PIN_SET)

/* 档位开关读取: 闭合(低电平)返回1 */
#define GET_GEAR_SWITCH()       ((HAL_GPIO_ReadPin(TS_ACT_DET_GPIO_Port, \
                                    TS_ACT_DET_Pin) == GPIO_PIN_RESET) ? 1U : 0U)
/* 外部TS激活开关: PE2高电平=激活 */
#define GET_EXT_TS_SWITCH()    ((HAL_GPIO_ReadPin(TS_ACT_EXT_DET_GPIO_Port, \
                                    TS_ACT_EXT_DET_Pin) == GPIO_PIN_SET) ? 1U : 0U)

typedef enum { GEAR_STATE_P = 0, GEAR_STATE_D = 1 } gear_state_t;
typedef enum {
    VCU_POWERON_MODE = 0, VCU_LV_SELF_CHECK = 1, VCU_HV_SELF_CHECK = 2,
    VCU_READY = 3, VCU_DRIVING = 4
} vcu_state_t;

void Vehicle_Init(void);
void Vehicle_LED1_Blink(void);
void Vehicle_GearShiftCheck(void);
void Vehicle_BrakeLightCheck(void);
void Vehicle_GearTick(void);
gear_state_t Vehicle_GetGearState(void);
void Vehicle_StateMachine(void);
vcu_state_t Vehicle_GetVCUState(void);
void Vehicle_SensorReport(void);

#ifdef __cplusplus
}
#endif

#endif /* __VEHICLE_H__ */
