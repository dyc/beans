#ifndef BEANS_SYS_DEVICE_H_
#define BEANS_SYS_DEVICE_H_

#include <kernel/desc.h>

// todo: temporary home for kbd output until we have a filesystem
extern char KEYBOARD_BUFFER[];
extern size_t KEYBOARD_CURSOR;
extern void keyboard_install();

extern void pit_install();
extern void pit_set_freq_hz(size_t hz);
extern void pit_set_timer_cb(void (*cb)(unsigned long));

#endif
