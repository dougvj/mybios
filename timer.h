#ifndef _TIMER_H_
#define _TIMER_H_


#include "types.h"

typedef struct dev_timer dev_timer;

dev_timer *timer_init(u32 base, u32 irq, u32 frequency);

u32 timer_get_ticks(dev_timer *dev);
u32 timer_set_frequency(dev_timer *dev, u32 frequency);void timer_watchdog(dev_timer *dev, u32 ticks);
void timer_watchdog_reset(dev_timer *dev);
void timer_watchdog_enabled(dev_timer *dev);
void timer_watchdog_enable(dev_timer *dev);
typedef int timer_callback_id;
typedef u32 (*timer_callback)(u32 ticks);
timer_callback_id timer_register_callback(dev_timer *dev, timer_callback callback, u32 ticks);
bool timer_unregister_callback(dev_timer *dev, timer_callback_id id);
#endif
