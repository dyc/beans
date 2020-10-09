#include <dirent.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct rd_prologue {
  uint32_t magic;
} __attribute__((packed));

struct rd_entry {
  // first byte of this file
  uint32_t start;
  // first byte of next rd_entry
  uint32_t next;
  // name goes here
} __attribute__((packed));

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("usage: /path/to/output /path/to/ramdisk/root\n");
    exit(EXIT_FAILURE);
  }

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
  struct rd_prologue prologue = {.magic = 0xfeedbeeb};
  fwrite(&prologue, sizeof(struct rd_prologue), 1, out);

  chdir(argv[2]);
  // todo: subdirs?
  struct dirent *de;
  struct rd_entry current_entry = {.start = 0, .next = ftell(out)};
  while ((de = readdir(in)) != NULL) {
    if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) {
      continue;
    }

    FILE *f = fopen(de->d_name, "rb");
    fseek(f, 0, SEEK_END);
    size_t f_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *f_ptr = (uint8_t *)malloc(f_size);
    fread(f_ptr, sizeof(uint8_t), f_size, f);

    // take the null byte too
    size_t name_size = strlen(de->d_name) + 1;
    // use previous entry's next to calculate current start
    current_entry.start =
        current_entry.next + sizeof(struct rd_entry) + name_size;
    current_entry.next = current_entry.start + f_size;

    printf("writing entry for file %s starting at %x until %x\n", de->d_name,
           current_entry.start, current_entry.next);
    fwrite(&current_entry, sizeof(struct rd_entry), 1, out);
    fwrite(de->d_name, name_size, 1, out);
    fwrite(f_ptr, sizeof(uint8_t), f_size, out);
    fclose(f);
  }

  closedir(in);
  fclose(out);
  return 0;
}
