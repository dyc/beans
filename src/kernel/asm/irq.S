.macro irq_handler_start id
  .global irq_handler_start\id
  .type irq_handler_start\id, @function

  irq_handler_start\id:
    cli
    push $0x00
    push $\id
    jmp irq_handler_common
.endm

.extern irq_handler
.type irq_handler, @function

irq_handler_common:
  pusha
  call irq_handler
  popa
  add $8, %esp  # error code and IRQ id
  iret

irq_handler_start 0
irq_handler_start 1
irq_handler_start 2
irq_handler_start 3
irq_handler_start 4
irq_handler_start 5
irq_handler_start 6
irq_handler_start 7
irq_handler_start 8
irq_handler_start 9
irq_handler_start 10
irq_handler_start 11
irq_handler_start 12
irq_handler_start 13
irq_handler_start 14
irq_handler_start 15
