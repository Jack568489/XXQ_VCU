# XXQ_VCU - 整车控制器

基于 STM32F105VC 的整车控制器 (VCU) 固件项目，实现档位管理、制动灯控制、传感器采集及 CAN 总线通信等功能。

## 硬件平台

- **MCU**: STM32F105VC (Cortex-M3, 72MHz)
- **开发环境**: STM32CubeMX + MDK-ARM (Keil)
- **HAL 库**: STM32F1xx HAL Driver

## 外设资源

| 外设 | 用途 |
|------|------|
| ADC1 (DMA) | 传感器模拟信号连续采样 |
| CAN1 / CAN2 | CAN 总线通信（当前为环回测试模式） |
| USART1 | 遥测数据上报 (1Hz) |
| TIM3 | 1ms 系统时基 |
| GPIO | LED 指示、蜂鸣器/鸣笛、制动灯、档位开关 |

## 功能特性

- **档位管理**: P/D 档切换检测，进入 D 档需同时满足档位开关闭合 + 刹车值 > 10
- **制动灯控制**: D 档下根据刹车状态控制制动灯
- **蜂鸣器/鸣笛**: 档位切换时发出确认音 (持续 3 秒)
- **传感器采集**: ADC DMA 连续采样，主循环中转换为物理量
- **CAN 通信**: 双 CAN 总线 (CAN1/CAN2)，环回测试模式每秒发送一帧并自收验证
- **遥测上报**: USART1 每秒打印传感器数据
- **LED 指示**: LED1 以 1Hz 频率闪烁，指示系统运行状态

## 目录结构

```
.
├── Core/
│   ├── Inc/                 # 头文件
│   │   ├── vehicle.h        # 整车控制
│   │   ├── can.h            # CAN 总线
│   │   ├── adc.h            # ADC 采样
│   │   └── ...
│   └── Src/                 # 源文件
│       ├── main.c           # 主程序入口
│       ├── vehicle.c        # 整车控制逻辑
│       ├── can.c            # CAN 驱动及环回测试
│       ├── adc.c            # ADC DMA 采样
│       └── ...
├── Drivers/                 # HAL 驱动库
├── MDK-ARM/                 # Keil MDK 工程文件
├── XXQ_VCU.ioc              # STM32CubeMX 配置文件
└── README.md
```

## 主循环调度

| 周期 | 任务 |
|------|------|
| 每次循环 | ADC 传感器数据处理、档位切换检测 |
| 500ms | LED1 闪烁 |
| 1000ms | 档位状态机计时、UART 遥测上报、CAN 环回测试 |

## 编译与烧录

1. 使用 STM32CubeMX 打开 `XXQ_VCU.ioc`，生成 MDK-ARM 工程
2. 使用 Keil MDK 打开 `MDK-ARM/XXQ_VCU.uvprojx`
3. 编译后通过 ST-Link 或其他调试器烧录至目标板
