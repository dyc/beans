#include <dlfcn.h>
#include <unistd.h>

int _isatty(int fd) { return 1; }

#ifndef DYLD_INTERPOSE
#define DYLD_INTERPOSE(_replacment, _replacee)                                 \
  __attribute__((used)) static struct {                                        \
    const void *replacment;                                                    \
    const void *replacee;                                                      \
  } _interpose_##_replacee __attribute__((section("__DATA,__interpose"))) = {  \
      (const void *)(unsigned long)&_replacment,                               \
      (const void *)(unsigned long)&_replacee};
DYLD_INTERPOSE(_isatty, isatty);
#undef DYLD_INTERPOSE
#endif
