#include <stdio.h>

#include "db.h"

int main(int argc, char* argv[]) {
  (void)argc;
  char* path = argv[1];
  Db db = db_open(path);
  DbHeader header = db_header(&db);
  printf("%s\n", header.header_string);
  printf("%d\n", header.page_size);
  printf("%d\n", header.database_size);
  printf("%d\n", header.sqlite_version_number);

  return 0;
}
