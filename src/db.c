#include "db.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

struct MappedRegion {
  int size;
  int page_offset;
  uint8_t *data;
};

struct MappedRegion map_file(const char *filename) {
  int fd = open(filename, O_RDONLY);
  struct stat sb;
  fstat(fd, &sb);
  int size = sb.st_size > MMAP_SIZE ? MMAP_SIZE : sb.st_size;
  uint8_t *mapped = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  close(fd);
  return (struct MappedRegion){
      .size = size,
      .page_offset = 0,
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

uint8_t read_i8(const uint8_t *buf) { return buf[0]; }

uint16_t read_i16(const uint8_t *buf) { return (buf[0] << 8) | buf[1]; }

uint32_t read_i32(const uint8_t *buf) {
  return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

Page db_get_page(Db *db, int offset) {
  uint8_t *page_start = db->data + (PAGE_SIZE * offset);
  uint8_t *page_header_start = page_start + (offset == 0 ? 100 : 0);

  Page page = {
      .type = page_header_start[0],
      .first_freeblock = read_i16(page_header_start + 1),
      .num_cells = read_i16(page_header_start + 3),
      .cell_content_area = read_i16(page_header_start + 5),
      .num_frag_free_bytes = read_i16(page_header_start + 7),
      ._page_offset = offset,
      ._page_start = page_start,
  };

  bool is_interior = page.type == TABLE_INTERIOR || page.type == INDEX_INTERIOR;

  page.cell_ptrs = page_header_start + (is_interior ? 12 : 8);
  page.cells = page_header_start + page.cell_content_area;

  if (is_interior) {
    page.right_most_pointer = read_i32(page_header_start + 8);
  }

  return page;
}

DbHeader db_header(Db *db) {
  Page page = db_get_page(db, 0);
  uint8_t *data = page._page_start;
  DbHeader header;
  memcpy(header.header_string, data, 16);
  header.page_size = read_i16(data + 16);
  header.file_format_write_version = read_i8(data + 18);
  header.file_format_read_version = read_i8(data + 19);
  header.reserved_space = read_i8(data + 20);
  header.max_embedded_payload_fraction = read_i8(data + 21);
  header.min_embedded_payload_fraction = read_i8(data + 22);
  header.leaf_payload_fraction = read_i8(data + 23);
  header.file_change_counter = read_i32(data + 24);
  header.database_size = read_i32(data + 28);
  header.first_freelist_trunk_page = read_i32(data + 32);
  header.total_freelist_pages = read_i32(data + 36);
  header.schema_cookie = read_i32(data + 40);
  header.schema_format_number = read_i32(data + 44);
  header.default_page_cache_size = read_i32(data + 48);
  header.largest_root_b_tree_page_number = read_i32(data + 52);
  header.text_encoding = read_i32(data + 56);
  header.user_version = read_i32(data + 60);
  header.incremental_vacuum_mode = read_i32(data + 64);
  header.application_id = read_i32(data + 68);
  memcpy(header.reserved, data + 72, 20);
  header.version_valid_for = read_i32(data + 92);
  header.sqlite_version_number = read_i32(data + 96);
  return header;
}

struct CellIter {
  Page *page;
  int pos;
};

/*
 * Connundrum...
 * The logic to iterate through cells is the same for all cell types. This
 * encourages the cell_iter pattern. However, depending on the page type, the
 * cells look different.
 */

CellIter cell_iter(Page *p) {
  return (CellIter){
      .page = p,
      .pos = 0,
  };
}

Cell cell_next(CellIter *ci) {
  uint16_t next_offset = read_i16(ci->page->cell_ptrs + ci->pos);
  ci->pos++;
  return (Cell){
      .n_bytes = 0,
      .row_id = 0,
      .overflow_page = 0,
  };
};

bool cell_done(CellIter *ci) { return ci->page->num_cells == ci->pos; };
