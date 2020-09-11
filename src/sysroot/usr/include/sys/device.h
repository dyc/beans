#pragma once

#include <kernel/desc.h>

// todo: temporary home for kbd output until we have a filesystem
extern char KEYBOARD_BUFFER[128];
extern size_t KEYBOARD_CURSOR;
extern void keyboard_install();

extern void pit_install();
extern void pit_set_freq_hz(size_t hz);
extern void pit_set_timer_cb(void (*cb)(unsigned long));
