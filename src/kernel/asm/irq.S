.macro irq_handler_start irq
  .global irq_handler_start\irq
  .type irq_handler_start\irq, @function

  irq_handler_start\irq:
    cli
    push $0x00
    push $\irq
    jmp irq_handler_common
.endm

// remap our IRQs to start at 0x20
irq_handler_start 32
irq_handler_start 33
irq_handler_start 34
irq_handler_start 35
irq_handler_start 36
irq_handler_start 37
irq_handler_start 38
irq_handler_start 39
irq_handler_start 40
irq_handler_start 41
irq_handler_start 42
irq_handler_start 43
irq_handler_start 44
irq_handler_start 45
irq_handler_start 46
irq_handler_start 47

.extern c_irq_handler
.type c_irq_handler, @function

irq_handler_common:
  pusha
  // save segment registers
  // and load kernel data segment
  // it seems like ss is saved by control unit
  push %ds
  push %es
  push %fs
  push %gs
  mov $0x10, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs

  cld
  push %esp
  call c_irq_handler
  add $4, %esp

  pop %gs
  pop %fs
  pop %es
  pop %ds
  popa

  add $8, %esp  // error code and IRQ num
  iret
