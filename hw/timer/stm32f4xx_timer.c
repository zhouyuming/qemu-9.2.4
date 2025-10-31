#include "qemu/osdep.h"
#include "hw/timer/stm32f4xx_timer.h"
#include "qemu/log.h"
#include "hw/irq.h"
#include "hw/qdev-properties.h"
#include "qemu/module.h"
#include "migration/vmstate.h"

#ifndef STM_TIMER_ERR_DEBUG
#define STM_TIMER_ERR_DEBUG 0
#endif

#define DB_PRINT_L(lvl, fmt, args...) do { \
    if (STM_TIMER_ERR_DEBUG >= lvl) { \
        qemu_log("%s: " fmt, __func__, ## args); \
    } \
} while (0);

#define DB_PRINT(fmt, args...) DB_PRINT_L(1, fmt, ## args)

static void stm32f4xx_timer_set_alarm(STM32F4XXTimerState *s, int64_t now);

static void stm32f4xx_timer_interrupt(void *opaque)
{
    STM32F4XXTimerState *s = opaque;

    DB_PRINT("Interrupt\n");

    if (s->tim_dier & (TIM_DIER_UIE|TIM_DIER_CC1IE|TIM_DIER_CC2IE|TIM_DIER_CC3IE|TIM_DIER_CC4IE)
        && s->tim_cr1 & TIM_CR1_CEN) {
        if (!(s->tim_cr1 & TIM_CR1_URS)) {
            s->tim_sr |= TIM_SR_UIF;
            qemu_irq_pulse(s->irq);
        }
        stm32f4xx_timer_set_alarm(s, s->hit_time);
    }

    if (s->tim_ccmr1 & (TIM_CCMR1_OC2M2 | TIM_CCMR1_OC2M1) &&
        !(s->tim_ccmr1 & TIM_CCMR1_OC2M0) &&
        s->tim_ccmr1 & TIM_CCMR1_OC2PE &&
        s->tim_ccer & TIM_CCER_CC2E) {
        /* PWM 2 - Mode 1 */
        DB_PRINT("PWM2 Duty Cycle: %d%%\n",
                s->tim_ccr2 / (100 * (s->tim_psc + 1)));
    }
}

static inline int64_t stm32f4xx_ns_to_ticks(STM32F4XXTimerState *s, int64_t t)
{
    return muldiv64(t, s->freq_hz, 1000000000ULL) / (s->tim_psc + 1);
}

static void stm32f4xx_timer_set_alarm(STM32F4XXTimerState *s, int64_t now)
{
    uint64_t ticks;
    int64_t now_ticks;

    if (s->tim_arr == 0) {
        return;
    }

    DB_PRINT("Alarm set at: 0x%x\n", s->tim_cr1);

    now_ticks = stm32f4xx_ns_to_ticks(s, now);
    if (!(s->tim_cr1 & TIM_CR1_CMS))
        s->tim_arr = s->tim_arr - 1;
    ticks = s->tim_arr - (now_ticks - s->tick_offset);

    DB_PRINT("Alarm set in %d ticks\n", (int) ticks);

    s->hit_time = muldiv64((ticks + (uint64_t) now_ticks) * (s->tim_psc + 1),
                               1000000000ULL, s->freq_hz);

    timer_mod(s->timer, qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) + s->hit_time);
    DB_PRINT("Wait Time: %" PRId64 " ticks\n", s->hit_time);
}

static void stm32f4xx_timer_reset(DeviceState *dev)
{
    STM32F4XXTimerState *s = STM32F4XXTIMER(dev);
    int64_t now = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);

    s->tim_cr1 = 0;
    s->tim_cr2 = 0;
    s->tim_smcr = 0;
    s->tim_dier = 0;
    s->tim_sr = 0;
    s->tim_egr = 0;
    s->tim_ccmr1 = 0;
    s->tim_ccmr2 = 0;
    s->tim_ccer = 0;
    s->tim_cnt = 0;
    s->tim_psc = 0;
    s->tim_arr = 0;
    s->tim_ccr1 = 0;
    s->tim_ccr2 = 0;
    s->tim_ccr3 = 0;
    s->tim_ccr4 = 0;
    s->tim_dcr = 0;
    s->tim_dmar = 0;
    s->tim_or = 0;

    s->tick_offset = stm32f4xx_ns_to_ticks(s, now);
}

static uint64_t stm32f4xx_timer_read(void *opaque, hwaddr offset,
                           unsigned size)
{
    STM32F4XXTimerState *s = opaque;

    DB_PRINT("Read 0x%"HWADDR_PRIx"\n", offset);

    switch (offset) {
    case TIM_CR1:
        return s->tim_cr1;
    case TIM_CR2:
        return s->tim_cr2;
    case TIM_SMCR:
        return s->tim_smcr;
    case TIM_DIER:
        return s->tim_dier;
    case TIM_SR:
        return s->tim_sr;
    case TIM_EGR:
        return s->tim_egr;
    case TIM_CCMR1:
        return s->tim_ccmr1;
    case TIM_CCMR2:
        return s->tim_ccmr2;
    case TIM_CCER:
        return s->tim_ccer;
    case TIM_CNT:
        return stm32f4xx_ns_to_ticks(s, qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL)) -
               s->tick_offset;
    case TIM_PSC:
        return s->tim_psc;
    case TIM_ARR:
        return s->tim_arr;
    case TIM_CCR1:
        return s->tim_ccr1;
    case TIM_CCR2:
        return s->tim_ccr2;
    case TIM_CCR3:
        return s->tim_ccr3;
    case TIM_CCR4:
        return s->tim_ccr4;
    case TIM_DCR:
        return s->tim_dcr;
    case TIM_DMAR:
        return s->tim_dmar;
    case TIM_OR:
        return s->tim_or;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "%s: Bad offset 0x%"HWADDR_PRIx"\n", __func__, offset);
    }

    return 0;
}

static void stm32f4xx_timer_write(void *opaque, hwaddr offset,
                        uint64_t val64, unsigned size)
{
    STM32F4XXTimerState *s = opaque;
    uint32_t value = val64;
    int64_t now = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
    uint32_t timer_val = 0;

    DB_PRINT("Write 0x%x, 0x%"HWADDR_PRIx"\n", value, offset);

    switch (offset) {
    case TIM_CR1:
        s->tim_cr1 |= value;
        if (s->tim_cr1 & TIM_CR1_CEN)
            stm32f4xx_timer_set_alarm(s, now);
        if ((value & TIM_CR1_DIR) && ((s->tim_cr1 & TIM_CR1_CMS)))
            s->tim_cr1 |= TIM_CR1_DIR;
        break;
    case TIM_CR2:
        s->tim_cr2 |= value;
        break;
    case TIM_SMCR:
        s->tim_smcr |= value;
        break;
    case TIM_DIER:
        s->tim_dier |= value;
        break;
    case TIM_SR:
        /* This is set by hardware and cleared by software */
        s->tim_sr &= value;
        break;
    case TIM_EGR:
        s->tim_egr = value;
        /* software generate Update event */
        if (s->tim_egr & TIM_EGR_UG) {
            s->tick_offset = stm32f4xx_ns_to_ticks(s, now);
            if (s->tim_cr1 & TIM_CR1_CEN)
                stm32f4xx_timer_set_alarm(s, now);
        }
        break;
    case TIM_CCMR1:
        s->tim_ccmr1 |= value;
        break;
    case TIM_CCMR2:
        s->tim_ccmr2 |= value;
        break;
    case TIM_CCER:
        s->tim_ccer |= value;
        break;
    case TIM_PSC:
        timer_val = stm32f4xx_ns_to_ticks(s, now) - s->tick_offset;
        s->tim_psc |= value & 0xFFFF;
        s->tick_offset = stm32f4xx_ns_to_ticks(s, now) - timer_val;
        if (s->tim_cr1 & TIM_CR1_CEN)
            stm32f4xx_timer_set_alarm(s, now);
        break;
    case TIM_CNT:
        timer_val |= value;
        break;
    case TIM_ARR:
        s->tim_arr |= value;
        break;
    case TIM_CCR1:
        s->tim_ccr1 |= value;
        break;
    case TIM_CCR2:
        s->tim_ccr2 |= value;
        break;
    case TIM_CCR3:
        s->tim_ccr3 |= value;
        break;
    case TIM_CCR4:
        s->tim_ccr4 |= value;
        break;
    case TIM_DCR:
        s->tim_dcr |= value;
        break;
    case TIM_DMAR:
        s->tim_dmar |= value;
        break;
    case TIM_OR:
        s->tim_or |= value;
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "%s: Bad offset 0x%"HWADDR_PRIx"\n", __func__, offset);
        break;
    }
}

static const MemoryRegionOps stm32f4xx_timer_ops = {
    .read = stm32f4xx_timer_read,
    .write = stm32f4xx_timer_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static const VMStateDescription vmstate_stm32f4xx_timer = {
    .name = TYPE_STM32F4XX_TIMER,
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_INT64(tick_offset, STM32F4XXTimerState),
        VMSTATE_UINT32(tim_cr1, STM32F4XXTimerState),
        VMSTATE_UINT32(tim_cr2, STM32F4XXTimerState),
        VMSTATE_UINT32(tim_smcr, STM32F4XXTimerState),
        VMSTATE_UINT32(tim_dier, STM32F4XXTimerState),
        VMSTATE_UINT32(tim_sr, STM32F4XXTimerState),
        VMSTATE_UINT32(tim_egr, STM32F4XXTimerState),
        VMSTATE_UINT32(tim_ccmr1, STM32F4XXTimerState),
        VMSTATE_UINT32(tim_ccmr2, STM32F4XXTimerState),
        VMSTATE_UINT32(tim_ccer, STM32F4XXTimerState),
        VMSTATE_UINT32(tim_psc, STM32F4XXTimerState),
        VMSTATE_UINT32(tim_arr, STM32F4XXTimerState),
        VMSTATE_UINT32(tim_ccr1, STM32F4XXTimerState),
        VMSTATE_UINT32(tim_ccr2, STM32F4XXTimerState),
        VMSTATE_UINT32(tim_ccr3, STM32F4XXTimerState),
        VMSTATE_UINT32(tim_ccr4, STM32F4XXTimerState),
        VMSTATE_UINT32(tim_dcr, STM32F4XXTimerState),
        VMSTATE_UINT32(tim_dmar, STM32F4XXTimerState),
        VMSTATE_UINT32(tim_or, STM32F4XXTimerState),
        VMSTATE_END_OF_LIST()
    }
};

static Property stm32f4xx_timer_properties[] = {
    DEFINE_PROP_UINT64("clock-frequency", struct STM32F4XXTimerState,
                       freq_hz, 1000000000),
    DEFINE_PROP_END_OF_LIST(),
};

static void stm32f4xx_timer_init(Object *obj)
{
    STM32F4XXTimerState *s = STM32F4XXTIMER(obj);

    sysbus_init_irq(SYS_BUS_DEVICE(obj), &s->irq);

    memory_region_init_io(&s->iomem, obj, &stm32f4xx_timer_ops, s,
                          "stm32f4xx_timer", 0x400);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->iomem);
}

static void stm32f4xx_timer_realize(DeviceState *dev, Error **errp)
{
    STM32F4XXTimerState *s = STM32F4XXTIMER(dev);
    s->timer = timer_new_ns(QEMU_CLOCK_VIRTUAL, stm32f4xx_timer_interrupt, s);
}

static void stm32f4xx_timer_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    device_class_set_legacy_reset(dc, stm32f4xx_timer_reset);
    device_class_set_props(dc, stm32f4xx_timer_properties);
    dc->vmsd = &vmstate_stm32f4xx_timer;
    dc->realize = stm32f4xx_timer_realize;
}

static const TypeInfo stm32f4xx_timer_info = {
    .name          = TYPE_STM32F4XX_TIMER,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(STM32F4XXTimerState),
    .instance_init = stm32f4xx_timer_init,
    .class_init    = stm32f4xx_timer_class_init,
};

static void stm32f4xx_timer_register_types(void)
{
    type_register_static(&stm32f4xx_timer_info);
}

type_init(stm32f4xx_timer_register_types)
