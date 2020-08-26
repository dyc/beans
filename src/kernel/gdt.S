.global load_gdt
.type load_gdt, @function
load_gdt:
    movl 4(%esp), %eax
    lgdt (%eax)

    mov %ds, 0x10
    mov %es, 0x10
    mov %fs, 0x10
    mov %gs, 0x10
    mov %ss, 0x10

    ljmp $0x08, $.flush_cs
  .flush_cs:
      ret
