#ifndef BEANS_KERNEL_MACROS_H_
#define BEANS_KERNEL_MACROS_H_

#define IGNORE(x) (void)x;
#define ALIGN(x, a) ((x) += (a) - ((x) % (a)));

#endif
