/**
  ******************************************************************************
  * @file    dmcu.h
  * @brief   双电机控制模块头文件
  ******************************************************************************
  */
#ifndef __DMCU_H__
#define __DMCU_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define DMCU_LEFT_MOTOR_ID      0x01U
#define DMCU_RIGHT_MOTOR_ID     0x02U
#define DMCU_LEFT_RPM_ID        0x203U
#define DMCU_RIGHT_RPM_ID       0x204U
#define DMCU_CTRL_MODE_CURRENT  0x01U
#define DMCU_CMD_ENABLE         0x00U
#define DMCU_CMD_PARAM_READ     0x0FU
#define DMCU_PARAM_SPEED        0x01U
#define DMCU_MOTOR_POLE_PAIRS   4
#define DMCU_SPEED_MAX_RPM      3000
#define DMCU_RPM_TO_KMH_FACTOR  0.003f

typedef struct {
    int16_t  target_current;
    int16_t  actual_rpm;
    uint8_t  online;
} dmcu_motor_t;

void DMCU_Init(void);
void DMCU_ThrottleProcess(void);
void DMCU_SendCurrentCmd(void);
void DMCU_Process(void);
void DMCU_Heartbeat(void);
void DMCU_QuerySpeed(void);
const dmcu_motor_t * DMCU_GetLeftMotor(void);
const dmcu_motor_t * DMCU_GetRightMotor(void);
void DMCU_UpdateRPM(uint32_t motor_id, int16_t rpm);
uint8_t DMCU_GetVehicleSpeed(void);
void DMCU_SendRPM(void);

#ifdef __cplusplus
}
#endif
#endif /* __DMCU_H__ */
