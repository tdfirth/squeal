#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>

typedef struct DatabaseHeader {
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
} DatabaseHeader;

typedef struct BTreeHeader {
  uint8_t page_type;
  uint16_t first_freeblock;
  uint16_t num_cells;
  uint16_t cell_content_area;
  uint8_t num_frag_free_bytes;
  // Only for interior pages
  // uint32_t   right_most_pointer ;
} BTreeHeader;

typedef struct TableLeafCell {
  uint64_t n_bytes;
  uint64_t row_id;
  // payload: []u8,
  uint32_t overflow_page;
} TableLeafCell;

#endif  // PAGE_H
