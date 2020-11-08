#ifndef BEANS_KERNEL_MACROS_H_
#define BEANS_KERNEL_MACROS_H_

#define IGNORE(x) (void)x;
#define ALIGN(x, a) ((x) += (a) - ((x) % (a)));
#define PRINTF(fmt, ...)                                                       \
  {                                                                            \
    memset(buf, 0, sizeof(buf) / sizeof(buf[0]));                              \
    int _printf_n = sprintf(buf, "[%s:%d] ", __func__, __LINE__);              \
    sprintf(&buf[_printf_n], fmt, ##__VA_ARGS__);                              \
    serial_write(SERIAL_PORT_COM1, buf);                                       \
  };

#endif
