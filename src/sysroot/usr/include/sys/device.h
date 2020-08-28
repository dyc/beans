#pragma once

#include <kernel/desc.h>

extern const uint8_t KEYBOARD_IRQ;

extern int keyboard_handler(irq_state_t* s);
extern void keyboard_install();
