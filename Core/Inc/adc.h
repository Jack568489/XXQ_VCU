/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.h
  * @brief   This file contains all the function prototypes for
  *          the adc.c file
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
#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdint.h>
/* USER CODE END Includes */

extern ADC_HandleTypeDef hadc1;

/* USER CODE BEGIN Private defines */

/*------------------------------------------------------------------------------
 * ADC通道索引定义 (与DMA缓冲区下标一一对应)
 *------------------------------------------------------------------------------*/
#define ADC_CH_INDEX_VCC       0   /* 电池电压    PC0, ADC1_IN10 */
#define ADC_CH_INDEX_THROTTLE  1   /* 油门信号    PC2, ADC1_IN12 */
#define ADC_CH_INDEX_BRAKE     2   /* 制动传感器  PC3, ADC1_IN13 */
#define ADC_CH_INDEX_STEERING  3   /* 转向传感器  PC4, ADC1_IN14 */
#define ADC_CH_NUM             4   /* ADC通道总数 */

/*------------------------------------------------------------------------------
 * 电池电压转换参数
 *------------------------------------------------------------------------------
 * 低压电池额定12V, 经电阻分压后送入ADC.
 * 标定点: 电池电压10V时ADC原始值=1128, 低于此值触发欠压.
 * 转换公式: V_bat = raw * 10.0 / 1128.0
 */
#define BATTERY_UNDERVOLTAGE_RAW    1128U   /* 10V对应的ADC原始值 */
#define BATTERY_UNDERVOLTAGE_V      10.0f   /* 欠压阈值电压 */

/*------------------------------------------------------------------------------
 * 油门转换参数
 *------------------------------------------------------------------------------
 * 有效范围: ADC原始值 200~850 → 0~100 (百分比)
 */
#define THROTTLE_RAW_MIN    200U
#define THROTTLE_RAW_MAX    850U

/*------------------------------------------------------------------------------
 * 刹车转换参数
 *------------------------------------------------------------------------------
 * 有效范围: ADC原始值 200~400 → 0~100 (百分比)
 */
#define BRAKE_RAW_MIN       200U
#define BRAKE_RAW_MAX       400U

/*------------------------------------------------------------------------------
 * 转向传感器转换参数
 *------------------------------------------------------------------------------
 * 传感器输出0~5V, 经电阻分压(5:3.3)后送入ADC(0~3.3V).
 * 换算: ADC原始值 = 传感器电压 / 5.0 * 4095
 *
 *   传感器电压        ADC原始值        转角
 *   0V       →        0        →      -30° (左转极限)
 *   2.4V     →        1966     →        ~0° (左转边界)
 *   2.4~2.6V →   1966~2130    →        0°  (中位校准窗口)
 *   2.6V     →        2130     →        ~0° (右转边界)
 *   5.0V     →        4095     →      +30° (右转极限)
 */
#define STEER_LEFT_MAX_RAW      1966U   /* 传感器2.4V对应ADC值, 左转区上界 */
#define STEER_RIGHT_MIN_RAW     2130U   /* 传感器2.6V对应ADC值, 右转区下界 */
#define STEER_RAW_MAX           4095U   /* ADC满量程, 对应传感器5V */
#define STEER_ANGLE_MAX         30.0f   /* 最大转向角度 */

/*------------------------------------------------------------------------------
 * 传感器数据结构体 (句柄)
 *------------------------------------------------------------------------------
 * 其他模块通过访问此结构体获取所有传感器物理值.
 */
typedef struct {
    float    battery_voltage;   /* 电池电压 (V)              */
    uint8_t  undervoltage;      /* 欠压标志: 0=正常, 1=欠压 */
    float    throttle;          /* 油门开度 (0~100)          */
    float    brake;             /* 刹车开度 (0~100)          */
    float    steering_angle;    /* 转向角度 (-30°~+30°)      */
    uint16_t raw[ADC_CH_NUM];   /* 最近一次ADC原始值         */
} vehicle_sensors_t;

/* USER CODE END Private defines */

void MX_ADC1_Init(void);

/* USER CODE BEGIN Prototypes */

/**
  * @brief  启动ADC1 DMA连续采样
  * @param  None
  * @retval None
  */
void ADC_StartDMA(void);

/**
  * @brief  获取ADC采样原始值 (从DMA安全副本读取, 非阻塞)
  * @param  raw_out: 输出缓冲区, 长度至少为ADC_CH_NUM
  * @retval None
  */
void ADC_GetRawValues(uint16_t *raw_out);

/**
  * @brief  处理ADC原始值, 转换为传感器物理量
  * @param  sensors: 传感器数据结构体指针 (输出句柄)
  * @retval None
  * @note   调用此函数前应先调用ADC_GetRawValues获取最新原始值,
  *         或直接传入已知的原始值数组.
  */
void ADC_ProcessSensorData(vehicle_sensors_t *sensors);

/**
  * @brief  获取传感器数据句柄 (包含最新转换结果)
  * @param  None
  * @retval 指向全局传感器数据句柄的指针
  * @note   每次调用ADC_ProcessSensorData后会更新此句柄内容
  */
const vehicle_sensors_t * ADC_GetSensorHandle(void);

/**
  * @brief  获取欠压状态
  * @param  None
  * @retval 0=正常, 1=欠压
  */
uint8_t ADC_IsUndervoltage(void);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H__ */
