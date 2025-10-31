#ifndef STM32F4XX_GPIO_H
#define STM32F4XX_GPIO_H

#include "hw/sysbus.h"
#include "qom/object.h"

#define STM_GPIO_MODE           0x00
#define STM_GPIO_OTYPER         0x04
#define STM_GPIO_OSPEEDR        0x08
#define STM_GPIO_PUPDR          0x0C
#define STM_GPIO_IDR            0x10
#define STM_GPIO_ODR            0x14
#define STM_GPIO_BSRR           0x18
#define STM_GPIO_LCKR           0x1c
#define STM_GPIO_AFRL           0x20
#define STM_GPIO_AFRH           0x24

#define TYPE_STM32F4XX_GPIO "stm32f4xx-gpio"
OBJECT_DECLARE_SIMPLE_TYPE(STM32F4XXGPIOState, STM32F4XX_GPIO)

struct STM32F4XXGPIOState {
    /* <private> */
    SysBusDevice parent_obj;

    /* <public> */
    MemoryRegion mmio;

    uint32_t gpio_mode;
    uint32_t gpio_otyper;
    uint32_t gpio_ospeedr;
    uint32_t gpio_pupdr;
    uint32_t gpio_idr;
    uint32_t gpio_odr;
    uint32_t gpio_bsrr;
    uint32_t gpio_lckr;
    uint32_t gpio_afrl;
    uint32_t gpio_afrh;

    Clock *clk;
    qemu_irq irq;
};

#endif
