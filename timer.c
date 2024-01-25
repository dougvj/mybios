#include "timer.h"
#include "io.h"
#include "output.h"
#include "interrupt.h"
#include "util.h"

#define MAX_TIMERS 8
struct dev_timer {
  u32 base;
  int watchdog_counter;
  int watchdog_enabled;
  u32 timer_ticks;
  u32 frequency;
  timer_callback timer_callbacks[MAX_TIMERS];
  u32 callback_intervals[MAX_TIMERS];
  u32 callback_counters[MAX_TIMERS];
};

static void check_timer_callbacks(dev_timer* timer);

void handle_timer(u8 vector, itr_frame *frame, void *data) {
  dev_timer* timer = (dev_timer*)data;
  if (timer->watchdog_enabled) {
    timer->watchdog_counter++;
    if (timer->watchdog_counter > 1000) {
      printf("Watchdog!\n");
      printf("IP: 0x%x\n", frame->ip);
      asm("hlt");
    }
  }
  timer->timer_ticks++;
  check_timer_callbacks(timer);
  // Acknowledge the interrupt
  outb(0x20, 0x20);
}

void timer_enable_watchdog(dev_timer* timer) { timer->watchdog_enabled = 1; }

void timer_disable_watchdog(dev_timer* timer) { timer->watchdog_enabled = 0; }

void timer_watchdog_reset(dev_timer* timer) { timer->watchdog_counter = 0; }

u32 timer_get_ticks(dev_timer* timer) { return timer->timer_ticks; }


timer_callback_id timer_register_callback(dev_timer* timer, timer_callback callback, u32 interval) {
  printf("Registering timer callback\n");
  for (int i = 0; i < MAX_TIMERS; i++) {
    if (timer->timer_callbacks[i] == NULL) {
      timer->timer_callbacks[i] = callback;
      timer->callback_intervals[i] = interval;
      timer->callback_counters[i] = 0;
      printf("Registered timer callback %d\n", i);
      return i;
    }
  }
  return -1;
}

static void check_timer_callbacks(dev_timer* timer) {
  // TODO sort by next interval
  for (int i = 0; i < MAX_TIMERS; i++) {
    if (timer->timer_callbacks[i] != NULL) {
      timer->callback_counters[i]++;
      if (timer->callback_counters[i] >= timer->callback_intervals[i]) {
        u32 new_interval = timer->timer_callbacks[i](timer->callback_intervals[i]);
        if (new_interval == 0) {
          timer->timer_callbacks[i] = NULL;
          continue;
        }
        timer->callback_intervals[i] = new_interval;
        timer->callback_counters[i] = 0;
      }
    }
  }
}

bool timer_unregister_callback(dev_timer* timer, timer_callback_id id) {
  if (id < 0 || id >= 256) {
    return false;
  }
  timer->timer_callbacks[id] = NULL;
  return true;
}

// For now, we only support one timer
dev_timer timer = {};


dev_timer* timer_init(u32 base, u32 irq, u32 frequency) {
  printf("Initializing timer\n");
  dev_timer* t = &timer;
  t->base = base;
  t->frequency = frequency;
  t->watchdog_counter = 0;
  t->watchdog_enabled = 0;
  t->timer_ticks = 0;
  for (int i = 0; i < MAX_TIMERS; i++) {
    t->timer_callbacks[i] = NULL;
  }
  u32 divisor = 1193180 / frequency;
  outb(base + 3, 0x36);
  outb(base, divisor & 0xFF);
  outb(base, (divisor >> 8) & 0xFF);
  itr_set_handler(irq, handle_timer, &timer);
  return t;
}
