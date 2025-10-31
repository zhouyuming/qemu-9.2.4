#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/log.h"
#include "hw/misc/stm32f4xx_flash.h"
#include "hw/irq.h"
#include "hw/qdev-properties.h"
#include "qemu/module.h"
#include "migration/vmstate.h"
#include "qemu/module.h"

#ifndef STM_FLASH_ERR_DEBUG
#define STM_FLASH_ERR_DEBUG 0
#endif

#define DB_PRINT_L(lvl, fmt, args...) do { \
    if (STM_FLASH_ERR_DEBUG >= lvl) { \
        qemu_log("%s: " fmt, __func__, ## args); \
    } \
} while (0);

#define DB_PRINT(fmt, args...) DB_PRINT_L(1, fmt, ## args)

static void stm32f4xx_flash_reset(DeviceState *dev)
{
    STM32F4XXFlashState *s = STM32F4XX_FLASH(dev);

    s->flash_acr        = 0x00000030;
    s->flash_keyr       = 0x00000000;
    s->flash_optkeyr    = 0x00000000;
    s->flash_sr         = 0x00000000;
    s->flash_cr         = 0x00000000;
    s->flash_ar      = 0x0fffaaed;
    s->flash_obr     = 0x0fff0000;
    s->flash_wrpr     = 0x0fff0000;
}

static uint64_t stm32f4xx_flash_read(void *opaque, hwaddr addr,
                                     unsigned int size)
{
    STM32F4XXFlashState *s = opaque;
    uint64_t retvalue = 0;

    DB_PRINT("Address: 0x%" HWADDR_PRIx "\n", addr);
    switch(addr) {
    case STM_FLASH_ACR:
        retvalue = s->flash_acr;
        break;
    case STM_FLASH_KEYR:
        retvalue = s->flash_keyr;
        break;
    case STM_FLASH_OPTKEYR:
        retvalue = s->flash_optkeyr;
        break;
    case STM_FLASH_SR:
        retvalue = s->flash_sr;
        break;
    case STM_FLASH_CR:
        retvalue = s->flash_cr;
        break;
    case STM_FLASH_AR:
        retvalue = s->flash_ar;
        break;
    case STM_FLASH_OBR:
        retvalue = s->flash_obr;
        break;
    case STM_FLASH_WRPR:
        retvalue = s->flash_wrpr;
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset 0x%" HWADDR_PRIx "\n",
            __func__, addr);
        retvalue = 0;
        break;
    }
    return retvalue;
}

static void stm32f4xx_flash_write(void *opaque, hwaddr addr,
                                uint64_t val64, unsigned int size)
{
    STM32F4XXFlashState *s = opaque;
    uint32_t value = val64;

    DB_PRINT("Address: 0x%" HWADDR_PRIx ", Value: 0x%x\n", addr, value);
    switch(addr) {
     case STM_FLASH_ACR:
        s->flash_acr |= value;
        break;
    case STM_FLASH_KEYR:
        s->flash_keyr |= value;
        break;
    case STM_FLASH_OPTKEYR:
        s->flash_optkeyr |= value;
        break;
    case STM_FLASH_SR:
        s->flash_sr |= value;
        break;
    case STM_FLASH_CR:
        s->flash_cr |= value;
        break;
    case STM_FLASH_AR:
        s->flash_ar |= value;
        break;
    case STM_FLASH_OBR:
        s->flash_obr |= value;
        break;
    case STM_FLASH_WRPR:
        s->flash_wrpr |= value;
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset 0x%" HWADDR_PRIx "\n",
            __func__, addr);
    }
}

static const MemoryRegionOps stm32f4xx_flash_ops = {
    .read = stm32f4xx_flash_read,
    .write = stm32f4xx_flash_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static const VMStateDescription vmstate_stm32f4xx_flash = {
    .name = TYPE_STM32F4XX_FLASH,
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32(flash_acr, STM32F4XXFlashState),
        VMSTATE_UINT32(flash_keyr, STM32F4XXFlashState),
        VMSTATE_UINT32(flash_optkeyr, STM32F4XXFlashState),
        VMSTATE_UINT32(flash_sr, STM32F4XXFlashState),
        VMSTATE_UINT32(flash_cr, STM32F4XXFlashState),
        VMSTATE_UINT32(flash_obr, STM32F4XXFlashState),
        VMSTATE_UINT32(flash_obr, STM32F4XXFlashState),
        VMSTATE_UINT32(flash_wrpr, STM32F4XXFlashState),
        VMSTATE_END_OF_LIST()
    }
};

static void stm32f4xx_flash_init(Object *obj)
{
    STM32F4XXFlashState *s = STM32F4XX_FLASH(obj);

    memory_region_init_io(&s->mmio, obj, &stm32f4xx_flash_ops, s,
                          TYPE_STM32F4XX_FLASH, 0x400);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->mmio);
}

static void stm32f4xx_flash_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    // dc->reset = stm32f4xx_flash_reset;
    dc->vmsd = &vmstate_stm32f4xx_flash;
}

static const TypeInfo stm32f4xx_flash_info = {
    .name          = TYPE_STM32F4XX_FLASH,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(STM32F4XXFlashState),
    .instance_init = stm32f4xx_flash_init,
    .class_init    = stm32f4xx_flash_class_init,
};

static void stm32f4xx_flash_register_types(void)
{
    type_register_static(&stm32f4xx_flash_info);
}

type_init(stm32f4xx_flash_register_types)
