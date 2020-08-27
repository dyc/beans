.global load_idt
.type load_idt, @function
load_idt:
    movl 4(%esp), %eax
    lidt (%eax)
    ret
