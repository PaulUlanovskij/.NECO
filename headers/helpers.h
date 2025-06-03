#pragma once

#define defer_res(v)                                                           \
  do {                                                                         \
    result = v;                                                                \
    goto defer;                                                                \
  } while (0)
#define errdefer_res(v)                                                        \
  do {                                                                         \
    result = v;                                                                \
    goto errdefer;                                                             \
  } while (0)

#define len(arr) (sizeof((arr)) / sizeof(*(arr)))
