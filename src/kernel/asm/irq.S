.macro irq_handler_start id
  .global irq_handler_start\id
  .type irq_handler_start\id, @function

  irq_handler_start\id:
    cli
    push $0x00
    push $\id
    jmp irq_handler_common
.endm

.extern c_irq_handler
.type c_irq_handler, @function

irq_handler_common:
  pusha
  cld
  call c_irq_handler
  popa
  add $8, %esp  # error code and IRQ id
  iret

# remap our IRQs to start at 0x20
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
