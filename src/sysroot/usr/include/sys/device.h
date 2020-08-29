#pragma once

#include <kernel/desc.h>

// todo: temporary home for kbd output until we have a filesystem
extern char KEYBOARD_BUFFER[128];
extern size_t KEYBOARD_CURSOR;
extern void keyboard_install();

typedef void (*timer_cb_t)(unsigned long);
extern void pit_install();
extern void pit_set_freq_hz(size_t hz);
extern void pit_set_timer_cb(timer_cb_t cb);
