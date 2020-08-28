.global load_idt
.type load_idt, @function
load_idt:
    mov 4(%esp), %eax
    lidt (%eax)
    ret
