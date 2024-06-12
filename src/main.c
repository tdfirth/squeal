#include <stdio.h>
#include <string.h>

#include "db.h"

int main(int argc, char* argv[]) {
  char* path = argv[1];
  Db db = db_open(path);
  char buf[16];
  // memcpy(buf, db.data, 16);

  snprintf(buf, 16, "%s\n", (char*)db.data);
  printf("%s", buf);

  return 0;
}
