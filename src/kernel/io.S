.global outb
.type outb, @function
outb:
    mov -8(%esp), %al
    mov -4(%esp), %dx
    out %al, %dx
    ret

.global inb
.type inb, @function
inb:
    mov -4(%esp), %dx
    in %dx, %ax
    ret
