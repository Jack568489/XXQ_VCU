/**
  ******************************************************************************
  * @file    dmcu.c
  * @brief   双电机控制模块实现 (电流控制, CAN2↔MCU, CAN1→ICU)
  ******************************************************************************
  */

#include "dmcu.h"
#include "adc.h"
#include "can.h"
#include "vehicle.h"

static dmcu_motor_t left_motor;
static dmcu_motor_t right_motor;
static int16_t       cached_target_current = 0;

static int16_t DMCU_ThrottleToCurrent(float t)
{ if (t <= 0.0f) return 0; if (t >= 100.0f) return 500; return (int16_t)(t * 5.0f); }

void DMCU_Init(void)
{
    left_motor.target_current = 0; left_motor.actual_rpm = 0; left_motor.online = 0;
    right_motor.target_current = 0; right_motor.actual_rpm = 0; right_motor.online = 0;
    cached_target_current = 0;
}

/* 油门信号处理: 200ms周期, 读取ADC油门值并转换为目标电流 */
void DMCU_ThrottleProcess(void)
{
    const vehicle_sensors_t *s = ADC_GetSensorHandle();
    vcu_state_t v = Vehicle_GetVCUState();
    gear_state_t g = Vehicle_GetGearState();
    int16_t tc = 0;

    if (v == VCU_DRIVING && g == GEAR_STATE_D)
        tc = DMCU_ThrottleToCurrent(s->throttle);

    cached_target_current = tc;
    left_motor.target_current = tc;
    right_motor.target_current = -tc;
}

/* 电机控制指令发送: 10ms周期, 通过CAN发送目标电流到左右电机 */
void DMCU_SendCurrentCmd(void)
{
    if (Vehicle_GetGearState() != GEAR_STATE_D) return;
    int16_t tc = cached_target_current;
    CAN_TxHeaderTypeDef h; uint8_t d[3]; uint32_t m;

    h.ExtId = 0; h.IDE = CAN_ID_STD; h.RTR = CAN_RTR_DATA; h.DLC = 3;
    h.TransmitGlobalTime = DISABLE;

    d[0] = DMCU_CTRL_MODE_CURRENT;
    d[1] = (uint8_t)((tc >> 8) & 0xFF); d[2] = (uint8_t)(tc & 0xFF);
    h.StdId = DMCU_LEFT_MOTOR_ID;
    if (HAL_CAN_AddTxMessage(&hcan2, &h, d, &m) != HAL_OK)
        HAL_CAN_AddTxMessage(&hcan2, &h, d, &m);

    int16_t rc = -tc;
    d[1] = (uint8_t)((rc >> 8) & 0xFF); d[2] = (uint8_t)(rc & 0xFF);
    h.StdId = DMCU_RIGHT_MOTOR_ID;
    if (HAL_CAN_AddTxMessage(&hcan2, &h, d, &m) != HAL_OK)
        HAL_CAN_AddTxMessage(&hcan2, &h, d, &m);
}

/* 保留旧接口: 兼容调用, 内部串联油门处理+指令发送 */
void DMCU_Process(void)
{
    DMCU_ThrottleProcess();
    DMCU_SendCurrentCmd();
}

void DMCU_Heartbeat(void)
{
    CAN_TxHeaderTypeDef h; uint8_t d[1] = {DMCU_CMD_ENABLE}; uint32_t m;
    h.ExtId = 0; h.IDE = CAN_ID_STD; h.RTR = CAN_RTR_DATA; h.DLC = 1;
    h.TransmitGlobalTime = DISABLE;
    h.StdId = DMCU_LEFT_MOTOR_ID;
    if (HAL_CAN_AddTxMessage(&hcan2, &h, d, &m) != HAL_OK) HAL_CAN_AddTxMessage(&hcan2, &h, d, &m);
    h.StdId = DMCU_RIGHT_MOTOR_ID;
    if (HAL_CAN_AddTxMessage(&hcan2, &h, d, &m) != HAL_OK) HAL_CAN_AddTxMessage(&hcan2, &h, d, &m);
}

void DMCU_QuerySpeed(void)
{
    CAN_TxHeaderTypeDef h; uint8_t d[2] = {DMCU_CMD_PARAM_READ, DMCU_PARAM_SPEED}; uint32_t m;
    h.ExtId = 0; h.IDE = CAN_ID_STD; h.RTR = CAN_RTR_DATA; h.DLC = 2;
    h.TransmitGlobalTime = DISABLE;
    h.StdId = DMCU_LEFT_MOTOR_ID;
    if (HAL_CAN_AddTxMessage(&hcan2, &h, d, &m) != HAL_OK) HAL_CAN_AddTxMessage(&hcan2, &h, d, &m);
    h.StdId = DMCU_RIGHT_MOTOR_ID;
    if (HAL_CAN_AddTxMessage(&hcan2, &h, d, &m) != HAL_OK) HAL_CAN_AddTxMessage(&hcan2, &h, d, &m);
}

const dmcu_motor_t * DMCU_GetLeftMotor(void) { return &left_motor; }
const dmcu_motor_t * DMCU_GetRightMotor(void) { return &right_motor; }

void DMCU_UpdateRPM(uint32_t id, int16_t rpm)
{ if (id == DMCU_LEFT_RPM_ID) { left_motor.actual_rpm = rpm; left_motor.online = 1; }
  else if (id == DMCU_RIGHT_RPM_ID) { right_motor.actual_rpm = rpm; right_motor.online = 1; } }

uint8_t DMCU_GetVehicleSpeed(void)
{
    int16_t l = left_motor.actual_rpm, r = right_motor.actual_rpm;
    if (l < 0) l = -l; if (r < 0) r = -r;
    float s = (float)((l + r) / 2) * DMCU_RPM_TO_KMH_FACTOR;
    if (s < 0.0f) s = 0.0f; if (s > 255.0f) s = 255.0f;
    return (uint8_t)s;
}

void DMCU_SendRPM(void)
{
    CAN_TxHeaderTypeDef h; uint8_t d[8] = {0}; uint32_t m;
    h.ExtId = 0; h.IDE = CAN_ID_STD; h.RTR = CAN_RTR_DATA; h.DLC = 8;
    h.TransmitGlobalTime = DISABLE;

    d[6] = (uint8_t)(left_motor.actual_rpm & 0xFF);
    d[7] = (uint8_t)((left_motor.actual_rpm >> 8) & 0xFF);
    h.StdId = DMCU_LEFT_RPM_ID;
    if (HAL_CAN_AddTxMessage(&hcan1, &h, d, &m) != HAL_OK) HAL_CAN_AddTxMessage(&hcan1, &h, d, &m);

    d[6] = (uint8_t)(right_motor.actual_rpm & 0xFF);
    d[7] = (uint8_t)((right_motor.actual_rpm >> 8) & 0xFF);
    h.StdId = DMCU_RIGHT_RPM_ID;
    if (HAL_CAN_AddTxMessage(&hcan1, &h, d, &m) != HAL_OK) HAL_CAN_AddTxMessage(&hcan1, &h, d, &m);
}
