#include <stddef.h>

#include <kernel/desc.h>
#include <kernel/macros.h>
#include <sys/device.h>
#include <sys/io.h>

static const uint8_t PIT_IRQ = 0x00;
static const size_t PIT_DEFAULT_HZ = 1193182;
static const uint8_t CHANNEL0_PORT = 0x40;
static const uint8_t COMMAND_PORT = 0x43;
// set channel 0 (b7b6 = 0b00) to 16 bit (b5b4 = 0b11)
// sw strobe mode (b3b2b1 = 0b100)
static const uint8_t CHANNEL0_LOHI_STROBE = 0x34;

static size_t divisor = 0xFFFFFFFF;
static unsigned long t = 0;
static void (*on_timer)(unsigned long);

void pit_set_timer_cb(void (*cb)(unsigned long)) { on_timer = cb; }

static int pit_handler(struct irq_state *s) {
  IGNORE(s)
  ++t;
  if (on_timer) {
    on_timer(t);
  }
  pic_ack(PIT_IRQ);
  return 1;
}

void pit_set_freq_hz(size_t hz) {
  // divisor is 16 bits so can only go down to 19MHz
  if (hz < 18 || hz > PIT_DEFAULT_HZ) {
    return;
  }

  if (hz == 18) {
    divisor = 0xFFFFFFFF;
  } else {
    divisor = PIT_DEFAULT_HZ / hz;
  }
  outb(COMMAND_PORT, CHANNEL0_LOHI_STROBE);
  outb(CHANNEL0_PORT, divisor & 0xFF);
  outb(CHANNEL0_PORT, (divisor >> 8) & 0xFF);
}

void pit_install() { irq_install_handler(PIT_IRQ, pit_handler); }
