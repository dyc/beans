.section .text
.align 4
.global load_gdt
.type load_gdt, @function
load_gdt:
    mov 4(%esp), %eax
    lgdt (%eax)

    # load kernel data segment
    # https://stackoverflow.com/questions/23978486/far-jump-in-gdt-in-bootloader

    # haven't confirmed, but seems like it may not be possible
    # to mov immediates into segment registers:
    # https://c9x.me/x86/html/file_module_x86_id_176.html
    # https://stackoverflow.com/questions/19074666/8086-why-cant-we-move-an-immediate-data-into-segment-register
    # https://stackoverflow.com/questions/25702277/why-use-temporary-register-to-pass-a-value-to-a-segment-register
    # to be safe, we avoid doing this :-p
    mov 0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    ljmp $0x08, $.flush_cs
.flush_cs:
    ret
