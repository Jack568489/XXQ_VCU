/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tick.h"
#include "led.h"
#include "beep.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/** @brief 制动任务状态枚举 */
typedef enum {
    BRAKE_STATE_IDLE   = 0,  /* 空闲状态：等待档位开关触发 */
    BRAKE_STATE_ACTIVE = 1   /* 激活状态：制动灯和蜂鸣器工作中 */
} brake_state_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/*------------------------------------------------------------------------------
 * 硬件操作宏定义 —— 桥接bsp层接口
 *------------------------------------------------------------------------------
 * tick.c 已提供多种时间粒度的标志位，此处直接按需选用:
 *   time_500ms_flag  → LED1闪烁（500ms亮/500ms灭 = 1Hz）
 *   time_1000ms_flag → 制动秒计数
 * 无需手动用 1ms_flag 累加计数。
 */
#define BRAKE_DURATION_SEC      (3U)      /* 制动持续时间: 3秒 */

/* LED1控制: 调用bsp层 set_led_status()，索引1对应LED1 */
#define LED1_ON()               set_led_status(1, 1)
#define LED1_OFF()              set_led_status(1, 0)

/* 蜂鸣器控制: 调用bsp层 BEEP_ON/BEEP_OFF */
#define BUZZER_ON()             BEEP_ON()
#define BUZZER_OFF()            BEEP_OFF()

/* 制动灯控制: LAMP_Pin(PD8)，低电平有效(输出低点亮) */
#define BRAKE_LIGHT_ON()        HAL_GPIO_WritePin(LAMP_GPIO_Port, LAMP_Pin, GPIO_PIN_RESET)
#define BRAKE_LIGHT_OFF()       HAL_GPIO_WritePin(LAMP_GPIO_Port, LAMP_Pin, GPIO_PIN_SET)

/* 档位开关读取: 按下返回1，松开返回0 (TS_ACT_DET_Pin, PE1) */
#define GET_GEAR_SWITCH()       ((HAL_GPIO_ReadPin(TS_ACT_DET_GPIO_Port, \
                                    TS_ACT_DET_Pin) == GPIO_PIN_RESET) ? 1U : 0U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* --- LED1 闪烁任务变量 --- */
static uint8_t  led1_is_on       = 0;   /* LED1当前状态: 0=灭, 1=亮 */

/* --- 制动灯 & 蜂鸣器 控制任务变量 --- */
static brake_state_t brake_state       = BRAKE_STATE_IDLE;  /* 制动任务状态机 */
static uint8_t       brake_timer       = 0;                  /* 制动激活计时器 (秒) */
static uint8_t       last_gear_switch  = 0;                  /* 上一周期档位开关状态(边沿检测) */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  /* 初始化tick定时器（启动TIM3中断） */
  tick_init();

  /* 确保所有外设处于已知初始状态 */
  LED1_OFF();
  BRAKE_LIGHT_OFF();
  BUZZER_OFF();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /*=================================================================
     * 第1步: 消费tick标志，驱动定时任务
     *=================================================================
     * tick.c 提供多种时间粒度标志位，按需各取所用:
     *   time_500ms_flag  → LED1闪烁（500ms亮/500ms灭 = 1Hz）
     *   time_1000ms_flag → 制动秒计数
     */
    if (time_flag.bit_field.time_500ms_flag)
    {
        time_flag.bit_field.time_500ms_flag = 0;  /* 清零标志 */

        /* ---- LED1 每500ms翻转一次（亮500ms + 灭500ms = 1Hz） ---- */
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

    if (time_flag.bit_field.time_1000ms_flag)
    {
        time_flag.bit_field.time_1000ms_flag = 0;  /* 清零标志 */

        /* ---- 制动任务秒计时（仅在激活期间） ---- */
        if (brake_state == BRAKE_STATE_ACTIVE)
        {
            brake_timer++;
        }
    }

    /*=================================================================
     * 第2步: 制动灯 & 蜂鸣器 控制任务
     *=================================================================
     * 触发条件: 档位开关的上升沿（0→1跳变），即驾驶员按下档位开关的瞬间。
     * 动作: 打开制动灯和蜂鸣器，启动3秒定时器。
     * 超时: 3秒后自动关闭制动灯和蜂鸣器，状态机回到空闲。
     * 可重入: 在3秒执行期间再次按下开关 → 重置定时器，重新计时3秒。
     */
    {
        uint8_t current_gear = GET_GEAR_SWITCH();  /* 读取当前档位开关状态 */

        /* ---- 边沿检测: 检测按下瞬间（0→1跳变） ---- */
        if ((last_gear_switch == 0U) && (current_gear == 1U))
        {
            /* 开关按下: 进入/重置制动激活状态 */
            brake_state  = BRAKE_STATE_ACTIVE;
            brake_timer  = 0;                    /* 重置秒计数器（支持可重入） */
            BRAKE_LIGHT_ON();
            BUZZER_ON();
        }

        /* ---- 制动超时检测: 3秒定时到达 ---- */
        if ((brake_state == BRAKE_STATE_ACTIVE) && (brake_timer >= BRAKE_DURATION_SEC))
        {
            /* 定时完成: 关闭所有输出，状态机复位 */
            BRAKE_LIGHT_OFF();
            BUZZER_OFF();
            brake_state = BRAKE_STATE_IDLE;
            brake_timer = 0;                     /* 清零计数器，准备下次触发 */
        }

        /* 保存本次开关状态，供下一周期边沿检测使用 */
        last_gear_switch = current_gear;
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.Prediv1Source = RCC_PREDIV1_SOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  RCC_OscInitStruct.PLL2.PLL2State = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the Systick interrupt time
  */
  __HAL_RCC_PLLI2S_ENABLE();
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
