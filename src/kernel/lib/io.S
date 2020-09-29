.global outb
.type outb, @function
outb:
    movl 8(%esp), %eax
    movl 4(%esp), %edx
    outb %al, %dx
    ret

.global inb
.type inb, @function
inb:
    movl 4(%esp), %edx
    inb %dx, %al
    ret

.global iowait
.type iowait, @function
iowait:
    jmp 1f
    1: jmp 2f
    2:
    ret
