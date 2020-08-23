.global outb
.type outb, @function
# equivalent c:
# asm volatile ("outb %1, %0" :: "dN" (port), "a" (data));
outb:
    movl 8(%esp), %eax
    movl 4(%esp), %edx
    outb %al, %dx
    ret

.global inb
.type inb, @function
# equivalent c:
# asm volatile("inb %1, %0" : "=a" (value) : "dN" (port));
inb:
    movl 4(%esp), %edx
    inb %dx, %al
    ret
