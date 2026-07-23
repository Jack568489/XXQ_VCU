/**
  ******************************************************************************
  * @file    vehicle.c
  * @brief   整车控制函数实现
  ******************************************************************************
  */

#include "vehicle.h"
#include "adc.h"
#include "usart.h"
#include <stdio.h>

#define BUZZER_DURATION_SEC        (3U)
#define BRAKE_THRESHOLD_D          0.1f
#define BRAKE_LIGHT_ON_THRESHOLD   0.2f
#define BRAKE_LIGHT_OFF_THRESHOLD  0.1f
#define SYS_READY_DELAY_SEC        (8U)

static uint8_t      led1_is_on = 0;
static gear_state_t gear_state = GEAR_STATE_P;
static uint8_t      buzzer_timer = 0;
static uint8_t      brake_ready  = 0;
static vcu_state_t  vcu_state = VCU_POWERON_MODE;

/**
  * @brief  整车控制初始化
  * @note   所有执行器置安全状态, 状态变量复位
  */
void Vehicle_Init(void)
{
    LED1_OFF(); BRAKE_LIGHT_OFF(); BUZZER_OFF();
    led1_is_on = 0; gear_state = GEAR_STATE_P;
    buzzer_timer = 0; brake_ready = 0;
    vcu_state = VCU_POWERON_MODE;
}

/**
  * @brief  LED1状态翻转 (500ms周期调用, 实现1Hz闪烁)
  */
void Vehicle_LED1_Blink(void)
{ if (led1_is_on) { LED1_OFF(); led1_is_on = 0; } else { LED1_ON(); led1_is_on = 1; } }

/**
  * @brief  档位切换检测
  * @note   P->D条件: 踩刹车+档位开关闭合+外部TS激活
  *         D->P条件: 档位开关断开即切回P档
  */
void Vehicle_GearShiftCheck(void)
{
    const vehicle_sensors_t *s = ADC_GetSensorHandle();
    uint8_t sw = GET_GEAR_SWITCH();
    uint8_t ext = GET_EXT_TS_SWITCH();

    if (gear_state == GEAR_STATE_P)
    {
        if ((s->brake > BRAKE_THRESHOLD_D) && !sw) brake_ready = 1;
        if (brake_ready && sw && ext)
        { gear_state = GEAR_STATE_D; buzzer_timer = 0; brake_ready = 0; BUZZER_ON(); return; }
        if (s->brake < BRAKE_LIGHT_OFF_THRESHOLD) brake_ready = 0;
    }
    else
    { if (!sw) { gear_state = GEAR_STATE_P; buzzer_timer = 0; BUZZER_OFF(); } }
}

/**
  * @brief  制动灯控制
  * @note   刹车开度 > 20% 点亮, < 10% 熄灭, 中间区间保持原状态(滞回)
  */
void Vehicle_BrakeLightCheck(void)
{
    const vehicle_sensors_t *s = ADC_GetSensorHandle();
    if (s->brake > BRAKE_LIGHT_ON_THRESHOLD) BRAKE_LIGHT_ON();
    else if (s->brake < BRAKE_LIGHT_OFF_THRESHOLD) BRAKE_LIGHT_OFF();
}

/**
  * @brief  档位计时 (1s周期)
  * @note   D档下累计3秒后自动关闭蜂鸣器
  */
void Vehicle_GearTick(void)
{
    if (gear_state == GEAR_STATE_D) { buzzer_timer++; if (buzzer_timer >= BUZZER_DURATION_SEC) BUZZER_OFF(); }
}

/* 获取当前档位状态 */
gear_state_t Vehicle_GetGearState(void) { return gear_state; }

/**
  * @brief  VCU状态机 (100ms周期)
  * @note   上电->低压自检->高压自检->就绪->行驶, 各状态按条件跳转
  */
void Vehicle_StateMachine(void)
{
    const vehicle_sensors_t *s = ADC_GetSensorHandle();
    switch (vcu_state) {
    case VCU_POWERON_MODE:  vcu_state = VCU_LV_SELF_CHECK;
    case VCU_LV_SELF_CHECK: if (s->undervoltage == 0U) { vcu_state = VCU_HV_SELF_CHECK; } else break;
    case VCU_HV_SELF_CHECK: vcu_state = VCU_READY;
    case VCU_READY: if (gear_state == GEAR_STATE_D) vcu_state = VCU_DRIVING; break;
    case VCU_DRIVING: if (gear_state == GEAR_STATE_P) vcu_state = VCU_READY; break;
    default: vcu_state = VCU_POWERON_MODE; break;
    }
}

/* 获取当前VCU状态 */
vcu_state_t Vehicle_GetVCUState(void) { return vcu_state; }

/**
  * @brief  传感器数据串口打印 (1s周期)
  * @note   格式: Vbat:xx.xV UV:x Thr:xx.x% Brk:xx.x% Str:xx.xdeg Gear:x VCU:x
  */
void Vehicle_SensorReport(void)
{
    const vehicle_sensors_t *s = ADC_GetSensorHandle();
    char buf[128];
    int len = snprintf(buf, sizeof(buf),
        "Vbat:%.1fV UV:%d Thr:%.1f%% Brk:%.1f%% Str:%.1fdeg Gear:%c VCU:%d\r\n",
        s->battery_voltage, s->undervoltage, s->throttle, s->brake,
        s->steering_angle, (gear_state == GEAR_STATE_D) ? 'D' : 'P', (int)vcu_state);
    if (len > 0) UART1_Send((uint8_t *)buf, (uint16_t)len);
}
