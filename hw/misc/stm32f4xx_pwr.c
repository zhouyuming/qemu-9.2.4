#include "qemu/osdep.h"
#include "qemu/log.h"
#include "trace.h"
#include "hw/irq.h"
#include "migration/vmstate.h"
#include "hw/misc/stm32f4xx_pwr.h"

#ifndef STM_POWER_ERR_DEBUG
#define STM_POWER_ERR_DEBUG 0
#endif

#define DB_PRINT_L(lvl, fmt, args...) do { \
    if (STM_POWER_ERR_DEBUG >= lvl) { \
        qemu_log("%s: " fmt, __func__, ## args); \
    } \
} while (0);

#define DB_PRINT(fmt, args...) DB_PRINT_L(1, fmt, ## args)

static void stm32f4xx_power_reset(DeviceState *dev)
{
    STM32F4XXPowerState *s = STM32F4XX_POWER(dev);
    s->pwr_cr    = 0x0000c000;
    s->pwr_csr   = 0x00000000;
}

static uint64_t stm32f4xx_power_read(void *opaque, hwaddr addr,
                                     unsigned int size)
{
    STM32F4XXPowerState *s = opaque;
    uint64_t retvalue = 0;

    DB_PRINT("Address: 0x%" HWADDR_PRIx "\n", addr);
    switch(addr) {
    case STM_PWR_CR:
        retvalue = s->pwr_cr;
        break;
    case STM_PWR_CSR:
        retvalue = s->pwr_csr;
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset 0x%" HWADDR_PRIx "\n",
            __func__, addr);
        retvalue = 0;
        break;
    }
    return retvalue;
}

static void stm32f4xx_power_write(void *opaque, hwaddr addr,
                                uint64_t val64, unsigned int size)
{
    STM32F4XXPowerState *s = opaque;
    uint32_t value = val64;

    DB_PRINT("Address: 0x%" HWADDR_PRIx ", Value: 0x%x\n", addr, value);
    switch(addr) {
    case STM_PWR_CR:
        s->pwr_cr = value;
        if (value & PWR_CR_ODEN)
            s->pwr_csr |= PWR_CSR_ODRDY;
        if (value & PWR_CR_ODSWEN)
            s->pwr_csr |= PWR_CSR_ODSWRDY;
        break;
    case STM_PWR_CSR:
        s->pwr_csr = value;
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset 0x%" HWADDR_PRIx "\n",
            __func__, addr);
    }
}

static const MemoryRegionOps stm32f4xx_power_ops = {
    .read = stm32f4xx_power_read,
    .write = stm32f4xx_power_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void stm32f4xx_power_init(Object *obj)
{
    STM32F4XXPowerState *s = STM32F4XX_POWER(obj);

    memory_region_init_io(&s->mmio, obj, &stm32f4xx_power_ops, s,
                          TYPE_STM32F4XX_POWER, 0x400);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->mmio);
}

static const VMStateDescription vmstate_stm32f4xx_power = {
    .name = TYPE_STM32F4XX_POWER,
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32(pwr_cr, STM32F4XXPowerState),
        VMSTATE_UINT32(pwr_csr, STM32F4XXPowerState),
        VMSTATE_END_OF_LIST()
    }
};

static void stm32f4xx_power_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->vmsd = &vmstate_stm32f4xx_power;
    device_class_set_legacy_reset(dc, stm32f4xx_power_reset);
}

static const TypeInfo stm32f4xx_power_info = {
    .name          = TYPE_STM32F4XX_POWER,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(STM32F4XXPowerState),
    .instance_init = stm32f4xx_power_init,
    .class_init    = stm32f4xx_power_class_init,
};

static void stm32f4xx_power_register_types(void)
{
    type_register_static(&stm32f4xx_power_info);
}

type_init(stm32f4xx_power_register_types)
