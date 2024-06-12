#include "db.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

struct MappedRegion {
  int size;
  void *data;
};

struct MappedRegion map_file(const char *filename) {
  int fd = open(filename, O_RDONLY);
  struct stat sb;
  fstat(fd, &sb);
  int size = sb.st_size > MMAP_SIZE ? MMAP_SIZE : sb.st_size;
  void *mapped = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  close(fd);
  return (struct MappedRegion){
      .size = size,
      .data = mapped,
  };
}

void unmap_file(void *mapped, size_t file_size) { munmap(mapped, file_size); }

Db db_open(char *path) {
  struct MappedRegion region = map_file(path);
  return (Db){
      .data = region.data,
      .path = path,
      .size = region.size,
  };
}

void db_close(Db *db) { unmap_file(db->data, db->size); }
