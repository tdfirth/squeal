#include "db.h"

#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "varint.h"

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
  uint8_t *mapped = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
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

uint32_t read_i24(const uint8_t *buf) {
  return (buf[0] << 16) | (buf[1] << 8) | buf[2];
}

uint32_t read_i32(const uint8_t *buf) {
  return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

uint64_t read_i48(const uint8_t *buf) {
  return ((uint64_t)buf[0] << 32) | (buf[1] << 24) | (buf[2] << 16) |
         (buf[3] << 8) | buf[4];
}

uint64_t read_i64(const uint8_t *buf) {
  return ((uint64_t)buf[0] << 56) | ((uint64_t)buf[1] << 48) |
         ((uint64_t)buf[2] << 40) | ((uint64_t)buf[3] << 32) | (buf[4] << 24) |
         (buf[5] << 16) | (buf[6] << 8) | buf[7];
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
  page.cells = page_start + page.cell_content_area;

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

CellIter cell_iter(Page *p) {
  return (CellIter){
      .page = p,
      .pos = 0,
  };
}

Cell iter_next(CellIter *iter) {
  int cell_ptr_offset = iter->pos * 2;
  uint16_t offset = read_i16(iter->page->cell_ptrs + cell_ptr_offset);
  iter->pos++;
  uint8_t *cell = iter->page->_page_start + offset;
  Cell result;
  result.type = iter->page->type;
  switch (result.type) {
    case TABLE_LEAF: {
      uint64_t payload_len;
      int pl_bytes_read = varint_decode(cell, &payload_len);
      uint64_t row_id;
      int rid_bytes_read = varint_decode(cell + pl_bytes_read, &row_id);
      uint8_t payload_offset = pl_bytes_read + rid_bytes_read;
      uint32_t overflow_page = read_i32(cell + payload_offset + payload_len);

      result.as.table_leaf = (TableLeafCell){
          .payload_len = payload_len,
          .row_id = row_id,
          .overflow_page = overflow_page,
          .payload = cell + payload_offset,
      };
      break;
    }
    case TABLE_INTERIOR: {
      uint32_t left_child = read_i32(cell);
      uint64_t row_id;
      varint_decode(cell + 4, &row_id);
      result.as.table_interior = (TableInteriorCell){
          .left_child = left_child,
          .row_id = row_id,
      };
      break;
    }
    case INDEX_LEAF: {
      uint64_t payload_len;
      int bytes_read = varint_decode(cell, &payload_len);
      uint8_t payload_offset = bytes_read;
      uint32_t overflow_page = read_i32(cell + payload_offset + payload_len);
      result.as.index_leaf = (IndexLeafCell){
          .payload_len = payload_len,
          .overflow_page = overflow_page,
          .payload = cell + payload_offset,
      };
      break;
    }
    case INDEX_INTERIOR: {
      uint32_t left_child = read_i32(cell);
      uint64_t payload_len;
      int bytes_read = varint_decode(cell + 4, &payload_len);
      uint8_t payload_offset = 4 + bytes_read;
      uint32_t overflow_page = read_i32(cell + payload_offset + payload_len);
      result.as.index_interior = (IndexInteriorCell){
          .left_child = left_child,
          .overflow_page = overflow_page,
          .payload_len = payload_len,
          .payload = cell + payload_offset,
      };
      break;
    }
  }
  return result;
}

bool iter_done(CellIter *ci) { return ci->page->num_cells == ci->pos; }

/* Reading a record in SQLite is kind of annoying. Because of dynamic typing,
 * you can actually encounter a value of any kind in any column. This means you
 * can't do any clever JIT-type stuff like precalculating offsets. If you want
 * to access the 5th column, you just have to look through all of the headers
 * and accumulate the offsets until you get to the one you want.
 *
 * Additionally, the caller doesn't know how big the result is going to be, so
 * you actually have to allocate the memory (if required) for the value in here.
 *
 * For now, we're going to return a 'Value' struct that encodes the length of
 * the data and a pointer to the contents. As a small optimization, if the
 * length is 0 we'll interpret the pointer as an integer, and if it's 1 we'll
 * interpret it is a float. Otherwise it's a pointer back to the memory mapped
 * region.
 */
Value read_record(int col, uint8_t *payload) {
  uint64_t header_len;
  int bytes_read = varint_decode(payload, &header_len);
  uint8_t *column_headers = payload + bytes_read;
  uint8_t *body = payload + header_len;

  // First we move along the header, accumulating offsets until we get to the
  // value we want to read.

  int current_column = 0;

  int col_header_len;
  uint64_t serial_type;
  uint64_t value_len;
  do {
    col_header_len = varint_decode(column_headers, &serial_type);
    column_headers += col_header_len;

    switch (serial_type) {
      case 0:
        value_len = 0;
        break;
      case 1:
        value_len = 1;
        break;
      case 2:
        value_len = 2;
        break;
      case 3:
        value_len = 3;
        break;
      case 4:
        value_len = 4;
        break;
      case 5:
        value_len = 6;
        break;
      case 6:
        value_len = 8;
        break;
      case 7:
        value_len = 8;
        break;
      case 8:
        value_len = 0;
        break;
      case 9:
        value_len = 0;
        break;
      case 10:
      case 11:
        // 10 and 11 are reserved
        continue;
      default: {
        if (serial_type % 2 == 0) {
          // Blob
          value_len = (serial_type - 12) / 2;
        } else {
          // Text
          value_len = (serial_type - 13) / 2;
        }
      }
    }

    // If we're looking at the current column then break, because we don't want
    // to move the body pointer.
    if (current_column == col) {
      break;
    }

    // Move along to the next header and shift the body pointer.
    current_column++;
    body += value_len;
  } while (1);

  Value result;

  switch (serial_type) {
    case 0: {
      result.type = vNULL;
      result.blob = NULL;
      break;
    }
    case 1: {
      result.type = vINT;
      result.integer = read_i8(body);
      break;
    }
    case 2: {
      result.type = vINT;
      result.integer = read_i16(body);
      break;
    }
    case 3: {
      result.type = vINT;
      result.integer = read_i24(body);
      break;
    }
    case 4: {
      result.type = vINT;
      result.integer = read_i32(body);
      break;
    }
    case 5: {
      result.type = vINT;
      result.integer = read_i48(body);
      break;
    }
    case 6: {
      result.type = vINT;
      result.integer = read_i64(body);
      break;
    }
    case 7: {
      result.type = vFLOAT;
      result.floating_point = 0;
      break;
    }
    case 8: {
      result.type = vINT;
      result.integer = 0;
      break;
    }
    case 9: {
      result.type = vINT;
      result.integer = 1;
      break;
    }
    case 10:
    case 11:
      assert(0 &&
             "Unreachable, record serial types 10 and 11 are for internal use "
             "only.");
    default: {
      // Add two because we've used 0, 1, and 2 for the tagged union.
      result.type = value_len + 2;
      result.blob = body;
    }
  }

  return result;
}

void print_value(Value value) {
  switch (value.type) {
    case vNULL:
      printf("NULL\n");
      break;
    case vINT:
      printf("%lu\n", value.integer);
      break;
    case vFLOAT:
      printf("%f\n", value.floating_point);
      break;
    default:
      printf("%.*s\n", value.type - 2, value.blob);
  }
}
