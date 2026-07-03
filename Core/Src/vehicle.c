/**
  ******************************************************************************
  * @file    vehicle.c
  * @brief   整车控制函数实现
  * @note    包含LED闪烁、档位切换(P→D)、制动灯/蜂鸣器控制、UART遥测上报
  *          各任务函数为纯动作函数, 由main.c负责节拍调度
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "vehicle.h"
#include "adc.h"
#include "usart.h"
#include <stdio.h>

/*------------------------------------------------------------------------------
 * 常量定义
 *------------------------------------------------------------------------------*/
#define BUZZER_DURATION_SEC     (3U)    /* 进入D档蜂鸣器确认音持续时长 (秒) */
#define BRAKE_THRESHOLD_D       10.0f   /* P→D切换所需刹车开度阈值 */

/*------------------------------------------------------------------------------
 * 私有变量
 *------------------------------------------------------------------------------*/

/* LED1闪烁: 当前状态 (0=灭, 1=亮) */
static uint8_t led1_is_on = 0;

/* 档位状态机 */
static gear_state_t gear_state = GEAR_STATE_P;  /* 当前档位 */
static uint8_t     buzzer_timer = 0;              /* 蜂鸣器已持续秒数 */

/*------------------------------------------------------------------------------
 * 函数实现
 *------------------------------------------------------------------------------*/

/**
  * @brief  整车控制初始化
  */
void Vehicle_Init(void)
{
    LED1_OFF();
    BRAKE_LIGHT_OFF();
    BUZZER_OFF();

    led1_is_on   = 0;
    gear_state   = GEAR_STATE_P;
    buzzer_timer = 0;
}

/**
  * @brief  LED1闪烁翻转 (每500ms调用一次)
  */
void Vehicle_LED1_Blink(void)
{
    if (led1_is_on)
    {
        LED1_OFF();
        led1_is_on = 0;
    }
    else
    {
        LED1_ON();
        led1_is_on = 1;
    }
}

/**
  * @brief  档位切换检测 (主循环每次迭代调用)
  * @note   P→D条件: 档位开关闭合(TS_ACT=低电平) 且 刹车值>10
  *         仅检测P→D的上升沿, 进入D后不会重复触发
  */
void Vehicle_GearShiftCheck(void)
{
    if (gear_state != GEAR_STATE_P)
    {
        return;  /* 已在D档, 不重复处理 */
    }

    uint8_t gear_switch_closed = GET_GEAR_SWITCH();  /* 1=闭合(低电平) */
    const vehicle_sensors_t *s = ADC_GetSensorHandle();

    if (gear_switch_closed && (s->brake > BRAKE_THRESHOLD_D))
    {
        /* P→D 切换 */
        gear_state   = GEAR_STATE_D;
        buzzer_timer = 0;
        BRAKE_LIGHT_ON();    /* LAMP点亮, 指示D档 */
        BUZZER_ON();         /* 蜂鸣器确认音 */
    }
}

/**
  * @brief  档位状态机计时 (每秒调用一次)
  * @note   进入D档后蜂鸣器持续BUZZER_DURATION_SEC秒后自动关闭,
  *         LAMP保持点亮以指示当前为D档
  */
void Vehicle_GearTick(void)
{
    if (gear_state == GEAR_STATE_D)
    {
        buzzer_timer++;

        if (buzzer_timer >= BUZZER_DURATION_SEC)
        {
            BUZZER_OFF();  /* 确认音结束, LAMP保持点亮指示D档 */
        }
    }
}

/**
  * @brief  获取当前档位状态
  */
gear_state_t Vehicle_GetGearState(void)
{
    return gear_state;
}

/**
  * @brief  UART传感器遥测上报 (每秒调用一次)
  */
void Vehicle_SensorReport(void)
{
    const vehicle_sensors_t *s = ADC_GetSensorHandle();

    char uart_buf[128];
    int  len = snprintf(uart_buf, sizeof(uart_buf),
                "Vbat:%.1fV UV:%d Thr:%.1f%% Brk:%.1f%% Str:%.1fdeg Gear:%c\r\n",
                s->battery_voltage,
                s->undervoltage,
                s->throttle,
                s->brake,
                s->steering_angle,
                (gear_state == GEAR_STATE_D) ? 'D' : 'P');

    if (len > 0)
    {
        UART1_Send((uint8_t *)uart_buf, (uint16_t)len);
    }
}
