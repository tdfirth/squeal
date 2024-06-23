#ifndef DB_H
#define DB_H

#include <stdbool.h>
#include <stdint.h>

#define MMAP_SIZE 0x7fff0000 /* 2147418112 */
#define PAGE_SIZE 4096

typedef struct Db {
  uint8_t* data;
  char* path;
  int size;
} Db;

Db db_open(char* path);

void db_close(Db* db);

typedef struct DbHeader {
  uint8_t header_string[16];
  uint16_t page_size;
  uint8_t file_format_write_version;
  uint8_t file_format_read_version;
  uint8_t reserved_space;
  uint8_t max_embedded_payload_fraction;
  uint8_t min_embedded_payload_fraction;
  uint8_t leaf_payload_fraction;
  uint32_t file_change_counter;
  uint32_t database_size;
  uint32_t first_freelist_trunk_page;
  uint32_t total_freelist_pages;
  uint32_t schema_cookie;
  uint32_t schema_format_number;
  uint32_t default_page_cache_size;
  uint32_t largest_root_b_tree_page_number;
  uint32_t text_encoding;
  uint32_t user_version;
  uint32_t incremental_vacuum_mode;
  uint32_t application_id;
  uint8_t reserved[20];
  uint32_t version_valid_for;
  uint32_t sqlite_version_number;
} DbHeader;

DbHeader db_header(Db* db);

typedef enum PageType {
  INDEX_INTERIOR = 0x02,
  TABLE_INTERIOR = 0x05,
  INDEX_LEAF = 0x0a,
  TABLE_LEAF = 0x0d,
} PageType;

typedef struct Page {
  PageType type;
  uint16_t first_freeblock;
  uint16_t num_cells;
  uint16_t cell_content_area;
  uint8_t num_frag_free_bytes;
  uint32_t right_most_pointer;  // Interior only.

  uint8_t* cell_ptrs;
  uint8_t* cells;

  int _page_offset;
  uint8_t* _page_start;
} Page;

Page db_get_page(Db* db, int offset);

typedef struct TableLeafCell {
  uint64_t row_id;
  uint64_t payload_len;
  uint8_t* payload;
  uint32_t overflow_page;
} TableLeafCell;

typedef struct TableInteriorCell {
  uint64_t row_id;
  uint32_t left_child;
} TableInteriorCell;

typedef struct IndexLeafCell {
  uint64_t payload_len;
  uint8_t* payload;
  uint32_t overflow_page;
} IndexLeafCell;

typedef struct IndexInteriorCell {
  uint64_t payload_len;
  uint8_t* payload;
  uint32_t left_child;
  uint32_t overflow_page;
} IndexInteriorCell;

typedef struct Cell {
  PageType type;
  union {
    TableLeafCell table_leaf;
    TableInteriorCell table_interior;
    IndexLeafCell index_leaf;
    IndexInteriorCell index_interior;
  } as;
} Cell;

typedef struct CellIter CellIter;

CellIter cell_iter(Page* p);
Cell cell_next(CellIter* ci);
bool cell_done(CellIter* ci);

#endif  // DB_H
