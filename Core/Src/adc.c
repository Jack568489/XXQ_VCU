/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.c
  * @brief   This file provides code for the configuration
  *          of the ADC instances.
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
/* Includes ------------------------------------------------------------------*/
#include "adc.h"

/* USER CODE BEGIN 0 */

/*------------------------------------------------------------------------------
 * ADC DMA 私有变量
 *------------------------------------------------------------------------------*/
static uint16_t adc_dma_buffer[ADC_CH_NUM] = {0};  /* DMA循环缓冲�? (中断填充)     */
static uint16_t adc_values[ADC_CH_NUM]     = {0};  /* 安全副本 (DMA回调中同步更�?) */

/* 全局传感器数据句�? (外部通过ADC_GetSensorHandle()只读访问) */
static vehicle_sensors_t g_vehicle_sensors = {0};

static float ADC_ConvertToBatteryVoltage(uint16_t raw)
{
    return (float)raw * BATTERY_UNDERVOLTAGE_V / (float)BATTERY_UNDERVOLTAGE_RAW;
}

/*------------------------------------------------------------------------------
 * 内部辅助: 将原始�?�转换为油门�?�? (0~100)
 *------------------------------------------------------------------------------
 * 有效范围: raw 200~850 �? 0~100%
 * 超出范围做钳位处�?
 */
static float ADC_ConvertToThrottle(uint16_t raw)
{
    if (raw <= THROTTLE_RAW_MIN) return 0.0f;
    if (raw >= THROTTLE_RAW_MAX) return 100.0f;
    return (float)(raw - THROTTLE_RAW_MIN) * 100.0f
           / (float)(THROTTLE_RAW_MAX - THROTTLE_RAW_MIN);
}

/*------------------------------------------------------------------------------
 * 内部辅助: 将原始�?�转换为刹车�?�? (0~100)
 *------------------------------------------------------------------------------
 * 有效范围: raw 200~400 �? 0~100%
 * 超出范围做钳位处�?
 */
static float ADC_ConvertToBrake(uint16_t raw)
{
    if (raw <= BRAKE_RAW_MIN) return 0.0f;
    if (raw >= BRAKE_RAW_MAX) return 100.0f;
    return (float)(raw - BRAKE_RAW_MIN) * 100.0f
           / (float)(BRAKE_RAW_MAX - BRAKE_RAW_MIN);
}

/*------------------------------------------------------------------------------
 * 内部辅助: 将原始�?�转换为转向角度 (-30° ~ +30°)
 *------------------------------------------------------------------------------
 * 传感器输�?0~5V, 经电阻分压后ADC�?0~3.3V (4095).
 *
 *   区间划分:
 *     [0,       STEER_LEFT_MAX_RAW)  �? 左转�?, 线�?�映射到 -30° ~ 0°
 *     [STEER_LEFT_MAX_RAW, STEER_RIGHT_MIN_RAW] �? 中位校准窗口, 0°
 *     (STEER_RIGHT_MIN_RAW, STEER_RAW_MAX]      �? 右转�?, 线�?�映射到 0° ~ +30°
 */
static float ADC_ConvertToSteeringAngle(uint16_t raw)
{
    if (raw < STEER_LEFT_MAX_RAW)
    {
        /* 左转�?: raw 0 �? -30°, raw STEER_LEFT_MAX_RAW �? 0° */
        return -STEER_ANGLE_MAX * (float)(STEER_LEFT_MAX_RAW - raw)
               / (float)STEER_LEFT_MAX_RAW;
    }
    else if (raw <= STEER_RIGHT_MIN_RAW)
    {
        /* 中位校准窗口: 方向盘居�?, 角度�?0° */
        return 0.0f;
    }
    else
    {
        /* 右转�?: raw STEER_RIGHT_MIN_RAW �? 0°, raw STEER_RAW_MAX �? +30° */
        float angle = STEER_ANGLE_MAX * (float)(raw - STEER_RIGHT_MIN_RAW)
                      / (float)(STEER_RAW_MAX - STEER_RIGHT_MIN_RAW);
        if (angle > STEER_ANGLE_MAX) angle = STEER_ANGLE_MAX;
        return angle;
    }
}

/* USER CODE END 0 */

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

/* ADC1 init function */
void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 4;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_10;   /* PC0: 电池电压检测 */
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_12;   /* PC2: 油门踏板信号 */
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_13;   /* PC3: 刹车踏板信号 */
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_14;   /* PC4: 转向角度传感器 */
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* ADC1 clock enable */
    __HAL_RCC_ADC1_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PC0     ------> ADC1_IN10
    PC2     ------> ADC1_IN12
    PC3     ------> ADC1_IN13
    PC4     ------> ADC1_IN14
    */
    GPIO_InitStruct.Pin = VCC_AD_Pin|OIL_PRE_F_AD_Pin|OIL_PRE_R_AD_Pin|ADC14_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(adcHandle,DMA_Handle,hdma_adc1);

  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC1_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PC0     ------> ADC1_IN10
    PC2     ------> ADC1_IN12
    PC3     ------> ADC1_IN13
    PC4     ------> ADC1_IN14
    */
    HAL_GPIO_DeInit(GPIOC, VCC_AD_Pin|OIL_PRE_F_AD_Pin|OIL_PRE_R_AD_Pin|ADC14_Pin);

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);
  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
  * @brief  ADC DMA转换完成回调 (每轮4通道扫描完成时由HAL库调�?)
  * @param  hadc: ADC句柄指针
  * @retval None
  * @note   在中断上下文中执�?, 仅做快�?�内存拷�?, 不做浮点运算
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        /* 从DMA循环缓冲区复制到安全副本 */
        adc_values[ADC_CH_INDEX_VCC]      = adc_dma_buffer[ADC_CH_INDEX_VCC];
        adc_values[ADC_CH_INDEX_THROTTLE] = adc_dma_buffer[ADC_CH_INDEX_THROTTLE];
        adc_values[ADC_CH_INDEX_BRAKE]    = adc_dma_buffer[ADC_CH_INDEX_BRAKE];
        adc_values[ADC_CH_INDEX_STEERING] = adc_dma_buffer[ADC_CH_INDEX_STEERING];
    }
}

/**
  * @brief  启动ADC1 DMA连续采样
  * @param  None
  * @retval None
  * @note   调用此函数后ADC将连续扫�?4通道, 通过DMA循环填充缓冲�?
  */
void ADC_StartDMA(void)
{
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_dma_buffer, ADC_CH_NUM);
}

/**
  * @brief  获取ADC采样原始�? (从安全副本读�?, 非阻�?)
  * @param  raw_out: 输出缓冲�?, 长度至少为ADC_CH_NUM
  * @retval None
  */
void ADC_GetRawValues(uint16_t *raw_out)
{
    if (raw_out == NULL) return;
    raw_out[ADC_CH_INDEX_VCC]      = adc_values[ADC_CH_INDEX_VCC];
    raw_out[ADC_CH_INDEX_THROTTLE] = adc_values[ADC_CH_INDEX_THROTTLE];
    raw_out[ADC_CH_INDEX_BRAKE]    = adc_values[ADC_CH_INDEX_BRAKE];
    raw_out[ADC_CH_INDEX_STEERING] = adc_values[ADC_CH_INDEX_STEERING];
}

/**
  * @brief  处理ADC原始�?, 转换为传感器物理�?
  * @param  sensors: 传感器数据结构体指针 (输出句柄, 为NULL时更新全�?句柄)
  * @retval None
  * @note   在主循环中调�?, 执行浮点转换运算
  */
void ADC_ProcessSensorData(vehicle_sensors_t *sensors)
{
    if (sensors == NULL)
    {
        sensors = &g_vehicle_sensors;
    }

    /* 同步�?新原始�?? */
    ADC_GetRawValues(sensors->raw);

    /* 通道0: 电池电压 */
    sensors->battery_voltage = ADC_ConvertToBatteryVoltage(
                                   sensors->raw[ADC_CH_INDEX_VCC]);

    /* 欠压判断: ADC原始�? <= 1128 (对应10V) */
    sensors->undervoltage = (sensors->raw[ADC_CH_INDEX_VCC]
                             <= BATTERY_UNDERVOLTAGE_RAW) ? 1U : 0U;

    /* 通道1: 油门�?�? (0~100) */
    sensors->throttle = ADC_ConvertToThrottle(
                            sensors->raw[ADC_CH_INDEX_THROTTLE]);

    /* 通道2: 刹车�?�? (0~100) */
    sensors->brake = ADC_ConvertToBrake(
                         sensors->raw[ADC_CH_INDEX_BRAKE]);

    /* 通道3: 转向角度 (-30° ~ +30°) */
    sensors->steering_angle = ADC_ConvertToSteeringAngle(
                                  sensors->raw[ADC_CH_INDEX_STEERING]);
}

/**
  * @brief  获取传感器数据句�? (只读)
  * @param  None
  * @retval 指向全局传感器数据句柄的常量指针
  */
const vehicle_sensors_t * ADC_GetSensorHandle(void)
{
    return &g_vehicle_sensors;
}

/**
  * @brief  获取欠压状�??
  * @param  None
  * @retval 0=正常, 1=欠压
  */
uint8_t ADC_IsUndervoltage(void)
{
    return (adc_values[ADC_CH_INDEX_VCC] <= BATTERY_UNDERVOLTAGE_RAW) ? 1U : 0U;
}

/* USER CODE END 1 */
