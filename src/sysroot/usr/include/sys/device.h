#pragma once

#include <kernel/desc.h>

// temporary home for kbd output until we have a filesystem
extern char KEYBOARD_BUFFER[128];
extern size_t KEYBOARD_CURSOR;

extern void keyboard_install();
