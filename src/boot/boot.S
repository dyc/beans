.code16
.global _start
_start:
  // real mode, cs:ip = (cs << 4) + ip
  ljmp $0x0000, $0x7c00

loop:
  jmp loop

.skip 510-(.-_start), 0
.word 0xaa55
