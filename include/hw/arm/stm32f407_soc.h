/*
 * STM32F407 SoC
 *
 * Copyright (c) 2025 zhouyuming <1006129869@qq.com>
 */

#ifndef HW_ARM_STM32F407_SOC_H
#define HW_ARM_STM32F407_SOC_H
 
#include "hw/or-irq.h"
#include "hw/arm/armv7m.h"
#include "hw/misc/stm32_rcc.h"
#include "hw/misc/stm32f4xx_syscfg.h"
#include "hw/char/stm32f4xx_usart.h"
#include "hw/misc/stm32f4xx_exti.h"
#include "hw/misc/stm32f4xx_pwr.h"
// #include "hw/timer/stm32f4xx_timer.h"
// #include "hw/gpio/stm32f4xx_gpio.h"
// #include "hw/misc/stm32f4xx_flash.h"
 
#define TYPE_STM32F407_SOC "stm32f407-soc"
OBJECT_DECLARE_SIMPLE_TYPE(STM32F407State, STM32F407_SOC)

#define RCC_BASE_ADDR       0x40023800

#define SYSCFG_BASE_ADDR    0x40013800
#define SYSCFG_IRQ  71

#define EXIT_BASE_ADDR      0x40013C00

#define STM_NUM_USARTS      4
#define STM32F407_USART1    0x40011000
#define STM32F407_USART2    0x40004400
#define STM32F407_USART3    0x40004800
#define STM32F407_USART6    0x40011400

// #define STM_NUM_TIMERS      4
// #define STM32F407_TIM1      0x40010000
// #define STM32F407_TIM2      0x40000000
// #define STM32F407_TIM3      0x40000400
// #define STM32F407_TIM4      0x40000800
// #define STM32F407_TIM5      0x40000c00
// #define STM32F407_TIM6      0x40001000
// #define STM32F407_TIM7      0x40001400
// #define STM32F407_TIM8      0x40010400
// #define STM32F407_TIM9      0x40014000
// #define STM32F407_TIM10     0x40014400
// #define STM32F407_TIM11     0x40014800
// #define STM32F407_TIM12     0x40001800
// #define STM32F407_TIM13     0x40001c00
// #define STM32F407_TIM14     0x40002000

// #define POWER_BASE_ADDR     0x40007000
// #define FLASH_BASE_ADDR     0x40003C00

// /* PortA ~ PortK */
// #define STM_NUM_GPIOS       11
// #define STM_GPIO_PORTA      0x40020000
// #define STM_GPIO_PORTB      0x40020400
// #define STM_GPIO_PORTC      0x40020800
// #define STM_GPIO_PORTD      0x40020c00
// #define STM_GPIO_PORTE      0x40021000
// #define STM_GPIO_PORTF      0x40021400
// #define STM_GPIO_PORTG      0x40021800
// #define STM_GPIO_PORTH      0x40021c00
// #define STM_GPIO_PORTI      0x40022000
// #define STM_GPIO_PORTJ      0x40022400
// #define STM_GPIO_PORTK      0x40022800

#define FLASH_BASE_ADDRESS  0x8000000
#define FLASH_SIZE          0x100000
#define SRAM_BASE_ADDRESS   0x20000000
#define SRAM_SIZE 0x1000000
#define CCM_BASE_ADDRESS 0x10000000
#define CCM_SIZE 0x10000
 
typedef struct STM32F407State {
    SysBusDevice parent_obj;

    ARMv7MState armv7m;

    STM32RccState rcc;
    STM32F4xxSyscfgState syscfg;
    STM32F4xxExtiState exti;
    STM32F4XXUsartState usart[STM_NUM_USARTS];

    MemoryRegion ccm;
    MemoryRegion sram;
    MemoryRegion flash;
    MemoryRegion flash_alias;

    Clock *sysclk;
    Clock *refclk;
} STM32F407State;
 
#endif
