#ifndef DB_H
#define DB_H

#define MMAP_SIZE 0x7fff0000 /* 2147418112 */
#define PAGE_SIZE 4096

typedef struct Db {
  void* data;
  char* path;
  int size;
} Db;

Db db_open(char* path);

void db_close(Db* db);

#endif  // DB_H
