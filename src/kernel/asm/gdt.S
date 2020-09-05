.section .text
.align 4
.global load_gdt
.type load_gdt, @function
load_gdt:
    mov 4(%esp), %eax
    lgdt (%eax)

    // load kernel data segment
    // reminder: can't mov immediates into segment registers
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    ljmp $0x08, $.flush_cs
.flush_cs:
    ret
