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

#define len(x) (sizeof((x)) / sizeof(*(x)))                                        \

#define try(expr) if((expr) == false) return false

#define defer(code) _defer(code, __LINE__)
#define _defer(code, line) __defer(code, line)
#define __defer(code, line) \
  void __cleanup_##line(int *dummy) { (void)dummy; code; } \
  int __defer_var_##line __attribute__((__cleanup__(__cleanup_##line))) = 0; \
  (void)__defer_var_##line
