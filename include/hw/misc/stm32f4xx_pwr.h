#ifndef HW_STM32F4XX_PWR_H
#define HW_STM32F4XX_PWR_H

#include "hw/sysbus.h"
#include "qom/object.h"

#define STM_PWR_CR      0x00
#define STM_PWR_CSR     0x04

#define PWR_CR_DBP      (1 << 8)
#define PWR_CR_ODEN     (1 << 16)
#define PWR_CR_ODSWEN   (1 << 17)

#define PWR_CSR_ODRDY   (1 << 16)
#define PWR_CSR_ODSWRDY (1 << 17)

#define TYPE_STM32F4XX_POWER "stm32f4xx-pwr"
OBJECT_DECLARE_SIMPLE_TYPE(STM32F4XXPowerState, STM32F4XX_POWER)

struct STM32F4XXPowerState {
    /* <private> */
    SysBusDevice parent_obj;

    /* <public> */
    MemoryRegion mmio;

    uint32_t pwr_cr;
    uint32_t pwr_csr;
};

#endif
