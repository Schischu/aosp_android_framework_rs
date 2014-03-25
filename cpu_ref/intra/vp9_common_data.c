#include "vp9_common_data.h"

// Log 2 conversion lookup tables for block width and height
const int b_width_log2_lookup[BLOCK_SIZES] =
  {0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4};
const int b_height_log2_lookup[BLOCK_SIZES] =
  {0, 1, 0, 1, 2, 1, 2, 3, 2, 3, 4, 3, 4};
const int num_4x4_blocks_wide_lookup[BLOCK_SIZES] =
  {1, 1, 2, 2, 2, 4, 4, 4, 8, 8, 8, 16, 16};
const int num_4x4_blocks_high_lookup[BLOCK_SIZES] =
  {1, 2, 1, 2, 4, 2, 4, 8, 4, 8, 16, 8, 16};
// Log 2 conversion lookup tables for modeinfo width and height
const int mi_width_log2_lookup[BLOCK_SIZES] =
  {0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3};
const int num_8x8_blocks_wide_lookup[BLOCK_SIZES] =
  {1, 1, 1, 1, 1, 2, 2, 2, 4, 4, 4, 8, 8};
const int mi_height_log2_lookup[BLOCK_SIZES] =
  {0, 0, 0, 0, 1, 0, 1, 2, 1, 2, 3, 2, 3};
const int num_8x8_blocks_high_lookup[BLOCK_SIZES] =
  {1, 1, 1, 1, 2, 1, 2, 4, 2, 4, 8, 4, 8};

// MIN(3, MIN(b_width_log2(bsize), b_height_log2(bsize)))
const int size_group_lookup[BLOCK_SIZES] =
  {0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3};

const int num_pels_log2_lookup[BLOCK_SIZES] =
  {4, 5, 5, 6, 7, 7, 8, 9, 9, 10, 11, 11, 12};

const TX_SIZE max_txsize_lookup[BLOCK_SIZES] = {
  TX_4X4,   TX_4X4,   TX_4X4,
  TX_8X8,   TX_8X8,   TX_8X8,
  TX_16X16, TX_16X16, TX_16X16,
  TX_32X32, TX_32X32, TX_32X32, TX_32X32
};
const TX_SIZE max_uv_txsize_lookup[BLOCK_SIZES] = {
  TX_4X4,   TX_4X4,   TX_4X4,
  TX_4X4,   TX_4X4,   TX_4X4,
  TX_8X8,   TX_8X8,   TX_8X8,
  TX_16X16, TX_16X16, TX_16X16, TX_32X32
};

const TX_SIZE tx_mode_to_biggest_tx_size[TX_MODES] = {
  TX_4X4,  // ONLY_4X4
  TX_8X8,  // ALLOW_8X8
  TX_16X16,  // ALLOW_16X16
  TX_32X32,  // ALLOW_32X32
  TX_32X32,  // TX_MODE_SELECT
};

const BLOCK_SIZE ss_size_lookup[BLOCK_SIZES][2][2] = {
//  ss_x == 0    ss_x == 0        ss_x == 1      ss_x == 1
//  ss_y == 0    ss_y == 1        ss_y == 0      ss_y == 1
  {{BLOCK_4X4,   BLOCK_INVALID}, {BLOCK_INVALID, BLOCK_INVALID}},
  {{BLOCK_4X8,   BLOCK_4X4},     {BLOCK_INVALID, BLOCK_INVALID}},
  {{BLOCK_8X4,   BLOCK_INVALID}, {BLOCK_4X4,     BLOCK_INVALID}},
  {{BLOCK_8X8,   BLOCK_8X4},     {BLOCK_4X8,     BLOCK_4X4}},
  {{BLOCK_8X16,  BLOCK_8X8},     {BLOCK_INVALID, BLOCK_4X8}},
  {{BLOCK_16X8,  BLOCK_INVALID}, {BLOCK_8X8,     BLOCK_8X4}},
  {{BLOCK_16X16, BLOCK_16X8},    {BLOCK_8X16,    BLOCK_8X8}},
  {{BLOCK_16X32, BLOCK_16X16},   {BLOCK_INVALID, BLOCK_8X16}},
  {{BLOCK_32X16, BLOCK_INVALID}, {BLOCK_16X16,   BLOCK_16X8}},
  {{BLOCK_32X32, BLOCK_32X16},   {BLOCK_16X32,   BLOCK_16X16}},
  {{BLOCK_32X64, BLOCK_32X32},   {BLOCK_INVALID, BLOCK_16X32}},
  {{BLOCK_64X32, BLOCK_INVALID}, {BLOCK_32X32,   BLOCK_32X16}},
  {{BLOCK_64X64, BLOCK_64X32},   {BLOCK_32X64,   BLOCK_32X32}},
};
