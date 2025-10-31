#include "qemu/osdep.h"
#include "qemu/log.h"
#include "hw/gpio/stm32f4xx_gpio.h"
#include "hw/irq.h"
#include "hw/clock.h"
#include "hw/qdev-clock.h"
#include "hw/qdev-properties.h"
#include "qapi/visitor.h"
#include "qapi/error.h"
#include "migration/vmstate.h"
#include "trace.h"

#ifndef STM_GPIO_ERR_DEBUG
#define STM_GPIO_ERR_DEBUG 0
#endif

#define DB_PRINT_L(lvl, fmt, args...) do { \
    if (STM_GPIO_ERR_DEBUG >= lvl) { \
        qemu_log("%s: " fmt, __func__, ## args); \
    } \
} while (0);

#define DB_PRINT(fmt, args...) DB_PRINT_L(1, fmt, ## args)

// static void stm32f4xx_gpio_reset(DeviceState *dev)
// {
//     STM32F4XXGPIOState *s = STM32F4XX_GPIO(dev);
//     s->gpio_mode     = 0x00000000;
//     s->gpio_otyper   = 0x00000000;
//     s->gpio_ospeedr  = 0x00000000;
//     s->gpio_otyper   = 0x00000000;
//     s->gpio_pupdr    = 0x00000000;
//     s->gpio_idr      = 0x00000000;
//     s->gpio_odr      = 0x00000000;
//     s->gpio_bsrr     = 0x00000000;
//     s->gpio_lckr     = 0x00000000;
//     s->gpio_afrl     = 0x00000000;
//     s->gpio_afrh     = 0x00000000;
// }

static uint64_t stm32f4xx_gpio_read(void *opaque, hwaddr addr,
                                     unsigned int size)
{
    STM32F4XXGPIOState *s = opaque;
    uint64_t retvalue = 0;

    DB_PRINT("Address: 0x%" HWADDR_PRIx "\n", addr);
    switch(addr) {
    case STM_GPIO_MODE:
        retvalue = s->gpio_mode;
        break;
    case STM_GPIO_OTYPER:
        retvalue = s->gpio_otyper;
        break;
    case STM_GPIO_OSPEEDR:
        retvalue = s->gpio_ospeedr;
        break;
    case STM_GPIO_PUPDR:
        retvalue = s->gpio_pupdr;
        break;
    case STM_GPIO_IDR:
        retvalue = s->gpio_idr;
        break;
    case STM_GPIO_ODR:
        retvalue = s->gpio_odr;
        break;
    case STM_GPIO_BSRR:
        retvalue = s->gpio_bsrr;
        break;
    case STM_GPIO_LCKR:
        retvalue = s->gpio_lckr;
        break;
    case STM_GPIO_AFRL:
        retvalue = s->gpio_afrl;
        break;
    case STM_GPIO_AFRH:
        retvalue = s->gpio_afrh;
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset 0x%" HWADDR_PRIx "\n",
            __func__, addr);
        retvalue = 0;
        break;
    }
    return retvalue;
}

static void stm32f4xx_gpio_write(void *opaque, hwaddr addr,
                                uint64_t val64, unsigned int size)
{
    STM32F4XXGPIOState *s = opaque;
    uint32_t value = val64;

    DB_PRINT("Address: 0x%" HWADDR_PRIx ", Value: 0x%x\n", addr, value);
    switch(addr) {
    case STM_GPIO_MODE:
        s->gpio_mode = value;
        break;
    case STM_GPIO_OTYPER:
        s->gpio_otyper = value;
        break;
    case STM_GPIO_OSPEEDR:
        s->gpio_ospeedr = value;
        break;
    case STM_GPIO_PUPDR:
        s->gpio_pupdr = value;
        break;
    case STM_GPIO_IDR:
        s->gpio_idr = value;
        break;
    case STM_GPIO_ODR:
        s->gpio_odr = value;
        break;
    case STM_GPIO_BSRR:
        s->gpio_bsrr = value;
        break;
    case STM_GPIO_LCKR:
        s->gpio_lckr = value;
        break;
    case STM_GPIO_AFRL:
        s->gpio_afrl = value;
        break;
    case STM_GPIO_AFRH:
        s->gpio_afrh = value;
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset 0x%" HWADDR_PRIx "\n",
            __func__, addr);
    }
}

static const MemoryRegionOps stm32f4xx_gpio_ops = {
    .read = stm32f4xx_gpio_read,
    .write = stm32f4xx_gpio_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void stm32f4xx_gpio_realize(DeviceState *dev, Error **errp)
{
    STM32F4XXGPIOState *s = STM32F4XX_GPIO(dev);
    if (!clock_has_source(s->clk)) {
        error_setg(errp, "GPIO: clk input must be connected");
        return;
    }
}

static const VMStateDescription vmstate_stm32f4xx_gpio = {
    .name = TYPE_STM32F4XX_GPIO,
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32(gpio_mode, STM32F4XXGPIOState),
        VMSTATE_UINT32(gpio_otyper, STM32F4XXGPIOState),
        VMSTATE_UINT32(gpio_ospeedr, STM32F4XXGPIOState),
        VMSTATE_UINT32(gpio_pupdr, STM32F4XXGPIOState),
        VMSTATE_UINT32(gpio_idr, STM32F4XXGPIOState),
        VMSTATE_UINT32(gpio_odr, STM32F4XXGPIOState),
        VMSTATE_UINT32(gpio_bsrr, STM32F4XXGPIOState),
        VMSTATE_UINT32(gpio_lckr, STM32F4XXGPIOState),
        VMSTATE_UINT32(gpio_afrl, STM32F4XXGPIOState),
        VMSTATE_UINT32(gpio_afrh, STM32F4XXGPIOState),
        VMSTATE_END_OF_LIST()
    }
};

static void stm32f4xx_gpio_init(Object *obj)
{
    STM32F4XXGPIOState *s = STM32F4XX_GPIO(obj);

    memory_region_init_io(&s->mmio, obj, &stm32f4xx_gpio_ops, s,
                          TYPE_STM32F4XX_GPIO, 0x400);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->mmio);

    sysbus_init_irq(SYS_BUS_DEVICE(obj), &s->irq);
}

static void stm32f4xx_gpio_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    // dc->reset = stm32f4xx_gpio_reset;
    dc->vmsd = &vmstate_stm32f4xx_gpio;
    dc->realize = stm32f4xx_gpio_realize;
}

static const TypeInfo stm32f4xx_gpio_info = {
    .name          = TYPE_STM32F4XX_GPIO,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(STM32F4XXGPIOState),
    .instance_init = stm32f4xx_gpio_init,
    .class_init    = stm32f4xx_gpio_class_init,
};

static void stm32f4xx_gpio_register_types(void)
{
    type_register_static(&stm32f4xx_gpio_info);
}

type_init(stm32f4xx_gpio_register_types)
