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

  // Print all of the table names.
  Page page = db_get_page(&db, 0);
  CellIter iter = cell_iter(&page);

  do {
    Cell cell = iter_next(&iter);
    Value value = read_record(4, cell.as.table_leaf.payload);
    print_value(value);
  } while (!iter_done(&iter));

  return 0;
}
