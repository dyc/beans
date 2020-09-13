#include <dlfcn.h>
#include <unistd.h>

#define DYLD_INTERPOSE(_replacment,_replacee) \
__attribute__((used)) static struct{ const void* replacment; const void* replacee; } _interpose_##_replacee \
__attribute__ ((section ("__DATA,__interpose"))) = { (const void*)(unsigned long)&_replacment, (const void*)(unsigned long)&_replacee };

int _isatty(int fd) {
  return 1;
}

DYLD_INTERPOSE(_isatty, isatty);
