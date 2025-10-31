#ifndef STM32F4XX_FLASH_H
#define STM32F4XX_FLASH_H

#include "hw/sysbus.h"

#define STM_FLASH_ACR      0x00
#define STM_FLASH_KEYR     0x04
#define STM_FLASH_OPTKEYR  0x08
#define STM_FLASH_SR       0x0c
#define STM_FLASH_CR       0x10
#define STM_FLASH_AR       0x14
#define STM_FLASH_OBR      0x18
#define STM_FLASH_WRPR     0x20

#define PWR_CR_DBP      (1 << 8)

#define TYPE_STM32F4XX_FLASH "stm32f4xx-flash"
#define STM32F4XX_FLASH(obj) \
    OBJECT_CHECK(STM32F4XXFlashState, (obj), TYPE_STM32F4XX_FLASH)

typedef struct {
    /* <private> */
    SysBusDevice parent_obj;

    /* <public> */
    MemoryRegion mmio;

    uint32_t flash_acr;
    uint32_t flash_keyr;
    uint32_t flash_optkeyr;
    uint32_t flash_sr;
    uint32_t flash_cr;
    uint32_t flash_ar;
    uint32_t flash_obr;
    uint32_t flash_wrpr;

} STM32F4XXFlashState;

#endif
