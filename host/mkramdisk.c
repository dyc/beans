#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct rd_prologue {
  uint32_t magic;
  uint32_t num_entries;
} __attribute__((packed));

struct rd_entry {
  // start offset, in bytes, of the file
  uint32_t offset;
  uint32_t next;
  char *name;
};

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("usage: /path/to/output /path/to/ramdisk/root\n");
    exit(EXIT_FAILURE);
  }

  struct rd_prologue prologue = {.magic = 0xfeedbeeb, .num_entries = 0};
  (void)prologue;

  FILE *out = fopen(argv[1], "wb");
  if (out == NULL) {
    printf("couldn't open %s\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  DIR *in = opendir(argv[2]);
  if (in == NULL) {
    printf("couldn't open %s\n", argv[2]);
    exit(EXIT_FAILURE);
  }

  printf("creating ramdisk %s from path %s\n", argv[1], argv[2]);

  struct dirent *de;
  while ((de = readdir(in)) != NULL) {
    printf("dir: %s\n", de->d_name);
  }

  closedir(in);
  fclose(out);

  return 0;
}
