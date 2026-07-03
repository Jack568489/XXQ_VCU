/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
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
#include "can.h"

/* USER CODE BEGIN 0 */
#include "usart.h"
#include <stdio.h>

/* CAN测试用变量 */
static uint8_t can_test_tx_counter = 0;   /* 发送计数器, 每次发送自增, 用于观察数据变化 */

/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef hcan2;

/* CAN1 init function */
void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 6;
  hcan1.Init.Mode = CAN_MODE_LOOPBACK;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_8TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_3TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = ENABLE;
  hcan1.Init.AutoWakeUp = ENABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}
/* CAN2 init function */
void MX_CAN2_Init(void)
{

  /* USER CODE BEGIN CAN2_Init 0 */

  /* USER CODE END CAN2_Init 0 */

  /* USER CODE BEGIN CAN2_Init 1 */

  /* USER CODE END CAN2_Init 1 */
  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 6;
  hcan2.Init.Mode = CAN_MODE_LOOPBACK;
  hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan2.Init.TimeSeg1 = CAN_BS1_8TQ;
  hcan2.Init.TimeSeg2 = CAN_BS2_3TQ;
  hcan2.Init.TimeTriggeredMode = DISABLE;
  hcan2.Init.AutoBusOff = ENABLE;
  hcan2.Init.AutoWakeUp = ENABLE;
  hcan2.Init.AutoRetransmission = DISABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN2_Init 2 */

  /* USER CODE END CAN2_Init 2 */

}

static uint32_t HAL_RCC_CAN1_CLK_ENABLED=0;

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    HAL_RCC_CAN1_CLK_ENABLED++;
    if(HAL_RCC_CAN1_CLK_ENABLED==1){
      __HAL_RCC_CAN1_CLK_ENABLE();
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN1 GPIO Configuration
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(CAN1_TX_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
  else if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspInit 0 */

  /* USER CODE END CAN2_MspInit 0 */
    /* CAN2 clock enable */
    __HAL_RCC_CAN2_CLK_ENABLE();
    HAL_RCC_CAN1_CLK_ENABLED++;
    if(HAL_RCC_CAN1_CLK_ENABLED==1){
      __HAL_RCC_CAN1_CLK_ENABLE();
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN2 GPIO Configuration
    PB12     ------> CAN2_RX
    PB13     ------> CAN2_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* CAN2 interrupt Init */
    HAL_NVIC_SetPriority(CAN2_TX_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN2_TX_IRQn);
    HAL_NVIC_SetPriority(CAN2_RX0_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(CAN2_RX0_IRQn);
  /* USER CODE BEGIN CAN2_MspInit 1 */

  /* USER CODE END CAN2_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    HAL_RCC_CAN1_CLK_ENABLED--;
    if(HAL_RCC_CAN1_CLK_ENABLED==0){
      __HAL_RCC_CAN1_CLK_DISABLE();
    }

    /**CAN1 GPIO Configuration
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN1_TX_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
  else if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspDeInit 0 */

  /* USER CODE END CAN2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN2_CLK_DISABLE();
    HAL_RCC_CAN1_CLK_ENABLED--;
    if(HAL_RCC_CAN1_CLK_ENABLED==0){
      __HAL_RCC_CAN1_CLK_DISABLE();
    }

    /**CAN2 GPIO Configuration
    PB12     ------> CAN2_RX
    PB13     ------> CAN2_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12|GPIO_PIN_13);

    /* CAN2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN2_TX_IRQn);
    HAL_NVIC_DisableIRQ(CAN2_RX0_IRQn);
  /* USER CODE BEGIN CAN2_MspDeInit 1 */

  /* USER CODE END CAN2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
  * @brief  CAN环回测试初始化
  *         配置滤波器接收所有报文, 启动CAN1/CAN2, 使能RX FIFO0中断通知
  * @retval None
  */
void CAN_TestInit(void)
{
    CAN_FilterTypeDef sFilterConfig;

    /* ==================== CAN1 滤波器配置 (Bank 0) ==================== */
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;       /* 掩码模式 */
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;      /* 32位滤波 */
    sFilterConfig.FilterIdHigh = 0x0000;                    /* 不过滤任何ID */
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;                /* 掩码全0, 所有ID都通过 */
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;  /* 匹配的消息存入FIFO0 */
    sFilterConfig.FilterActivation = CAN_FILTER_ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14;                /* Bank 0-13归CAN1, 14-27归CAN2 */

    if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /* 启动CAN1 */
    if (HAL_CAN_Start(&hcan1) != HAL_OK)
    {
        Error_Handler();
    }

    /* 使能CAN1 RX FIFO0 消息挂起中断 + TX邮箱空中断通知 */
    if (HAL_CAN_ActivateNotification(&hcan1,
            CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK)
    {
        Error_Handler();
    }

    /* ==================== CAN2 滤波器配置 (Bank 14) ==================== */
    sFilterConfig.FilterBank = 14;
    /* SlaveStartFilterBank 保持不变: 14 */
    if (HAL_CAN_ConfigFilter(&hcan2, &sFilterConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /* 启动CAN2 */
    if (HAL_CAN_Start(&hcan2) != HAL_OK)
    {
        Error_Handler();
    }

    /* 使能CAN2 RX FIFO0 消息挂起中断 + TX邮箱空中断通知 */
    if (HAL_CAN_ActivateNotification(&hcan2,
            CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK)
    {
        Error_Handler();
    }

    /* 打印初始化完成信息 */
    char msg[] = "\r\n"
                 "========== CAN Loopback Test Init OK ==========\r\n"
                 "  CAN1: Loopback | 500kbps | StdId=0x123 | Bank 0\r\n"
                 "  CAN2: Loopback | 500kbps | StdId=0x456 | Bank 14\r\n"
                 "  Test sends CAN1 + CAN2 every 1000ms...\r\n"
                 "==============================================\r\n\r\n";
    UART1_Send((uint8_t *)msg, sizeof(msg) - 1);
}

/**
  * @brief  发送CAN测试数据 (CAN1 + CAN2 各发一帧)
  *         环回模式下每帧都会被自身立即接收, 通过串口观察收发是否一致
  * @retval None
  */
void CAN_TestSendMessage(void)
{
    CAN_TxHeaderTypeDef   TxHeader;
    uint8_t               txData[8];
    uint32_t              txMailbox;
    char                  uartBuf[128];
    int                   len;

    /* ==================== CAN1 发送 ==================== */
    TxHeader.StdId = 0x123;                 /* CAN1 标准ID: 0x123 */
    TxHeader.ExtId = 0;
    TxHeader.IDE = CAN_ID_STD;              /* 标准帧 */
    TxHeader.RTR = CAN_RTR_DATA;            /* 数据帧 */
    TxHeader.DLC = 8;                       /* 数据长度: 8字节 */
    TxHeader.TransmitGlobalTime = DISABLE;

    /* CAN1 数据: [counter, counter+1, ... counter+7] */
    for (uint8_t i = 0; i < 8; i++)
    {
        txData[i] = can_test_tx_counter + i;
    }

    /* 打印 CAN1 即将发送的数据 (主循环上下文) */
    len = snprintf(uartBuf, sizeof(uartBuf),
                   "--- CAN1 TX --- StdId=0x%03X DLC=%d Data: %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
                   TxHeader.StdId, TxHeader.DLC,
                   txData[0], txData[1], txData[2], txData[3],
                   txData[4], txData[5], txData[6], txData[7]);
    if (len > 0)
    {
        UART1_Send((uint8_t *)uartBuf, (uint16_t)len);
    }

    /* CAN1: 投入发送邮箱 */
    if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, txData, &txMailbox) != HAL_OK)
    {
        char errMsg[] = "--- CAN1 TX --- ERROR: AddTxMessage failed!\r\n";
        UART1_Send((uint8_t *)errMsg, sizeof(errMsg) - 1);
    }

    /* ==================== CAN2 发送 ==================== */
    TxHeader.StdId = 0x456;                 /* CAN2 标准ID: 0x456 (与CAN1不同,便于区分) */
    /* IDE, RTR, DLC, TransmitGlobalTime 不变 */

    /* CAN2 数据: [counter+0x80, counter+0x81, ... counter+0x87] (高位置位,便于区分) */
    for (uint8_t i = 0; i < 8; i++)
    {
        txData[i] = can_test_tx_counter + 0x80U + i;
    }

    /* 打印 CAN2 即将发送的数据 (主循环上下文) */
    len = snprintf(uartBuf, sizeof(uartBuf),
                   "--- CAN2 TX --- StdId=0x%03X DLC=%d Data: %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
                   TxHeader.StdId, TxHeader.DLC,
                   txData[0], txData[1], txData[2], txData[3],
                   txData[4], txData[5], txData[6], txData[7]);
    if (len > 0)
    {
        UART1_Send((uint8_t *)uartBuf, (uint16_t)len);
    }

    /* CAN2: 投入发送邮箱 */
    if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, txData, &txMailbox) != HAL_OK)
    {
        char errMsg[] = "--- CAN2 TX --- ERROR: AddTxMessage failed!\r\n";
        UART1_Send((uint8_t *)errMsg, sizeof(errMsg) - 1);
    }

    /* 发送计数器自增 (CAN1和CAN2共用, 便于对比) */
    can_test_tx_counter++;
}

/* ========================== HAL CAN 回调函数 ========================== */

/**
  * @brief  CAN RX FIFO0 消息挂起回调 (中断上下文)
  *         当FIFO0中有新消息时由 HAL_CAN_IRQHandler 调用
  * @param  hcan: CAN句柄指针
  * @retval None
  */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t             rxData[8];
    char                uartBuf[128];
    int                 len;
    const char         *canName;

    /* 从FIFO0读取接收到的报文 */
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, rxData) != HAL_OK)
    {
        return;
    }

    /* 区分CAN1 / CAN2 */
    canName = (hcan->Instance == CAN1) ? "CAN1" : "CAN2";

    /* 格式化并打印接收到的数据 (中断上下文) */
    len = snprintf(uartBuf, sizeof(uartBuf),
                   "--- %s RX --- StdId=0x%03X DLC=%d Data: %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
                   canName, RxHeader.StdId, RxHeader.DLC,
                   rxData[0], rxData[1], rxData[2], rxData[3],
                   rxData[4], rxData[5], rxData[6], rxData[7]);
    if (len > 0)
    {
        UART1_Send((uint8_t *)uartBuf, (uint16_t)len);
    }
}

/**
  * @brief  CAN RX FIFO1 消息挂起回调 (预留)
  * @param  hcan: CAN句柄指针
  * @retval None
  */
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t             rxData[8];
    char                uartBuf[128];
    int                 len;
    const char         *canName;

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &RxHeader, rxData) != HAL_OK)
    {
        return;
    }

    canName = (hcan->Instance == CAN1) ? "CAN1" : "CAN2";

    len = snprintf(uartBuf, sizeof(uartBuf),
                   "--- %s RX-FIFO1 --- StdId=0x%03X DLC=%d Data: %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
                   canName, RxHeader.StdId, RxHeader.DLC,
                   rxData[0], rxData[1], rxData[2], rxData[3],
                   rxData[4], rxData[5], rxData[6], rxData[7]);
    if (len > 0)
    {
        UART1_Send((uint8_t *)uartBuf, (uint16_t)len);
    }
}

/**
  * @brief  CAN 发送邮箱0完成回调 (中断上下文)
  * @param  hcan: CAN句柄指针
  * @retval None
  */
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
{
    const char *canName = (hcan->Instance == CAN1) ? "CAN1" : "CAN2";
    char msg[64];
    int len = snprintf(msg, sizeof(msg), "--- %s TX --- Mailbox0 complete\r\n", canName);
    if (len > 0)
    {
        UART1_Send((uint8_t *)msg, (uint16_t)len);
    }
}

/**
  * @brief  CAN 发送邮箱1完成回调
  * @param  hcan: CAN句柄指针
  * @retval None
  */
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan)
{
    const char *canName = (hcan->Instance == CAN1) ? "CAN1" : "CAN2";
    char msg[64];
    int len = snprintf(msg, sizeof(msg), "--- %s TX --- Mailbox1 complete\r\n", canName);
    if (len > 0)
    {
        UART1_Send((uint8_t *)msg, (uint16_t)len);
    }
}

/**
  * @brief  CAN 发送邮箱2完成回调
  * @param  hcan: CAN句柄指针
  * @retval None
  */
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan)
{
    const char *canName = (hcan->Instance == CAN1) ? "CAN1" : "CAN2";
    char msg[64];
    int len = snprintf(msg, sizeof(msg), "--- %s TX --- Mailbox2 complete\r\n", canName);
    if (len > 0)
    {
        UART1_Send((uint8_t *)msg, (uint16_t)len);
    }
}

/**
  * @brief  CAN 错误回调 (中断上下文)
  * @param  hcan: CAN句柄指针
  * @retval None
  */
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
    const char *canName = (hcan->Instance == CAN1) ? "CAN1" : "CAN2";
    uint32_t error = HAL_CAN_GetError(hcan);
    char msg[80];
    int len = snprintf(msg, sizeof(msg), "--- %s ERR --- Error Code: 0x%08lX\r\n", canName, error);
    if (len > 0)
    {
        UART1_Send((uint8_t *)msg, (uint16_t)len);
    }
    HAL_CAN_ResetError(hcan);
}

/* USER CODE END 1 */
