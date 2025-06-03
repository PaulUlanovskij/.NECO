#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "headers/pulib.h"
#include "headers/types.h"

cstr_o cstr_from_sb(StringBuilder *sb) {
  cstr str = NULL;
  da_copy_items(&str, sb);
  return str;
}

cstr_o cstr_copy(const cstr str) {
  return (cstr_o)malloc_copy(strlen(str) + 1, str);
}

cstr_o cstr_concat_many(void *nil, ...) {
  StringBuilder sb = {};
  va_list vl;
  va_start(vl, nil);
  cstr arg = NULL;
  while ((arg = va_arg(vl, cstr)) != NULL) {
    sb_append_cstr(&sb, arg);
  }
  va_end(vl);
  cstr str = cstr_from_sb(&sb);
  da_free(&sb);
  return str;
}

char cstr_ends_with(cstr haystack, cstr needle) {
  int haystack_length = strlen(haystack);
  int needle_length = strlen(needle);
  if (haystack_length < needle_length) {
    return false;
  }
  return strncmp(haystack + haystack_length - needle_length, needle,
                 needle_length) == 0;
}
char cstr_starts_with(cstr haystack, cstr needle) {
  int haystack_length = strlen(haystack);
  int needle_length = strlen(needle);
  if (haystack_length < needle_length) {
    return false;
  }
  return strncmp(haystack, needle, needle_length) == 0;
}

cstr_o cstr_quote_copy(cstr arg) {
  char *pos = NULL;
  StringBuilder sb = {};
  if (strpbrk(arg, " \t\n\v\r\"") == NULL) {
    return cstr_copy(arg);
  }

  sb_append(&sb, '\"');
  pos = strpbrk(arg, "\\\"");
  uintptr_t index = 0;
  while (pos != NULL) {
    index = pos - arg;

    sb_append_sv(&sb, sv_from_cstr(arg, 0, index));
    sb_append(&sb, '\\');
    sb_append(&sb, arg[index]);

    arg = arg + index + 1;
    pos = strpbrk(arg, "\\\"");
  }
  sb_append(&sb, '\"');

  cstr res = cstr_from_sb(&sb);
  da_free(&sb);

  return res;
}

StringView sv_from_sb(StringBuilder sb) {
  return (StringView){
      .items = sb.items,
      .length = sb.length,
  };
}
StringBuilder sb_from_sv(StringView sv) {
  return (StringBuilder){
      .items = sv.items,
      .length = sv.length,
      .capacity = sv.length,
  };
}
StringView sv_from_cstr(const cstr str, int start, int end) {
  return (StringView){.items = str + start, end - start};
}

StringView sv_copy(StringView sv) {
  return (StringView){
      .items = (char *)malloc_copy(sv.length, sv.items),
      .length = sv.length,
  };
}

cstr_o sv_copy_to_cstr(StringView sv) {
  cstr_o str = (cstr_o)calloc(1, sv.length + 1);
  memcpy(str, sv.items, sv.length);
  return str;
}

StringView sv_from(StringView sv, int index) {
  return (StringView){
      .items = sv.items + index,
      .length = sv.length - index,
  };
}
StringView sv_upto(StringView sv, int index) {
  return (StringView){
      .items = sv.items,
      .length = index,
  };
}

void sb_clear(StringBuilder *sb) {
  free(sb->items);
  sb->capacity = 0;
  sb->length = 0;
}
void sb_append(StringBuilder *sb, char c) {
  if (sb->length == 0) {
    da_reserve(sb, 2);
    sb->length = 1;
  }
  sb->items[sb->length - 1] = c;
  do {
    do {
      while (((sb))->capacity < ((sb)->length + 1)) {
        if (((sb))->capacity == 0)
          ((sb))->capacity = 16;
        else
          ((sb))->capacity *= 2;
      }
      ((sb))->items = (typeof(((sb))->items))realloc(
          ((sb))->items, ((sb))->capacity * sizeof(*((sb))->items));
    } while (0);
    (sb)->items[(sb)->length++] = ('\0');
  } while (0);
}

void sb_append_buf(StringBuilder *sb, const char *buf, int buf_length) {
  if (sb->length == 0) {
    sb->length = 1;
  }
  do {
    while ((sb)->capacity < (sb->length + buf_length)) {
      if ((sb)->capacity == 0)
        (sb)->capacity = 16;
      else
        (sb)->capacity *= 2;
    }
    (sb)->items = (typeof((sb)->items))realloc(
        (sb)->items, (sb)->capacity * sizeof(*(sb)->items));
  } while (0);
  memcpy(sb->items + sb->length - 1, buf, buf_length);
  sb->length += buf_length;
  sb->items[sb->length - 1] = '\0';
}
void sb_append_cstr(StringBuilder *sb, const cstr str) {
  sb_append_buf(sb, str, strlen(str));
}
void sb_append_sv(StringBuilder *sb, StringView sv) {
  sb_append_buf(sb, sv.items, sv.length);
}
void sb_append_sb(StringBuilder *sb, StringBuilder sb2) {
  sb_append_buf(sb, sb2.items, sb2.length);
}

bool sv_from_file(StringView *sv, const cstr path) {
  char result = true;
  FILE *file = fopen(path, "rb");
  int m = flength(file);
  if (m < 0) {
    defer_res(false);
  }
  sv->items = (char *)malloc(m);
  fread(sv->items, m, 1, file);
  if (ferror(file)) {
    defer_res(false);
  }
  sv->length = m;
defer:
  fclose(file);
  return result;
}

bool sb_append_file(StringBuilder *sb, const cstr path) {
  bool result = true;
  FILE *file = fopen(path, "rb");
  int m = flength(file);
  if (m < 0) {
    defer_res(false);
  }
  size_t new_length = sb->length + m;
  da_reserve(sb, new_length);
  fread(sb->items + sb->length, m, 1, file);
  if (ferror(file)) {
    defer_res(false);
  }

  sb->length = new_length;
defer:
  fclose(file);
  return result;
}

int sv_get_char_index(StringView sv, char c) {
#define ONES ((size_t)-1 / UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX / 2 + 1))
#define HASZERO(x) ((x) - ONES & ~(x) & HIGHS)
  int i = 0;
  while ((uintptr_t)(sv.items + i) % sizeof(size_t)) {
    if (i >= sv.length) {
      return -1;
    }
    if (sv.items[i] == c) {
      return i;
    }
    i++;
  }

  size_t pattern = ONES * (unsigned char)c;
  size_t *word_ptr = (size_t *)(sv.items + i);

  while (i + sizeof(size_t) < sv.length) {
    if (HASZERO(*word_ptr ^ pattern)) {
      break;
    }
    i += sizeof(size_t);
    word_ptr++;
  }

  while (i < sv.length) {
    if (sv.items[i] == c) {
      return i;
    }
    i++;
  }
  return -1;
#undef ONES
#undef HIGHS
#undef HASZERO
}

// TODO: consider using strpbrk
int sv_get_first_char_index(StringView sv, const cstr str) {
#define ONES ((size_t)-1 / UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX / 2 + 1))
#define HASZERO(x) ((x) - ONES & ~(x) & HIGHS)
  int result = -1;
  int i = 0;
  int pattern_count = strlen(str);
  size_t *patterns = (size_t *)malloc(pattern_count * sizeof(size_t));

  while ((uintptr_t)(sv.items + i) % sizeof(size_t)) {
    if (i >= sv.length) {
      defer_res(-1);
    }
    if (strchr(str, sv.items[i]) != NULL) {
      defer_res(i);
    }
    i++;
  }

  for (int j = 0; j < pattern_count; j++) {
    patterns[j] = ONES * (unsigned char)str[j];
  }
  size_t *word_ptr = (size_t *)(sv.items + i);

  while (i + sizeof(size_t) < sv.length) {
    for (int j = 0; j < pattern_count; j++) {
      if (HASZERO(*word_ptr ^ patterns[j])) {
        goto seek_single;
      }
    }
    i += sizeof(size_t);
    word_ptr++;
  }
seek_single:
  while (i < sv.length) {
    if (strchr(str, sv.items[i]) != NULL) {
      defer_res(i);
    }
    i++;
  }
defer:
  free(patterns);
  return result;
#undef ONES
#undef HIGHS
#undef HASZERO
}

int sv_get_word_index(StringView sv, const cstr delim) {
  int delim_length = strlen(delim);
  int i = 0;
  StringView temp = sv;
  temp.length -= (delim_length - 1);
  while (i < sv.length - delim_length) {
    int j = sv_get_char_index(temp, delim[0]);
    if (j < 0) {
      return -1;
    }
    if (strncmp(temp.items + j, delim, delim_length) == 0) {
      return i + j;
    }
    j++;
    i += j;
    temp.length -= j;
    temp.items += j;
  }
  return -1;
}
int sv_get_first_word_index(StringView sv, const cstr const *delim, int count) {
  int *delim_lengths = (int *)malloc(sizeof(int) * count);
  char *heads = (char *)malloc(count);
  for (int i = 0; i < count; i++) {
    heads[i] = delim[i][0];
    delim_lengths[i] = strlen(delim[i]);
  }
  int result = 0;
  StringView temp = sv;
  char matched = true;
  while (matched) {
    matched = false;
    int j = sv_get_first_char_index(temp, heads);
    if (j < 0) {
      defer_res(-1);
    }
    for (int k = 0; k < count; k++) {
      if (j > sv.length - (delim_lengths[k] - 1)) {
        continue;
      }
      if (result < sv.length - delim_lengths[k]) {
        continue;
      }
      matched = true;
      if (strncmp(temp.items + j, delim[k], delim_lengths[k]) == 0) {
        defer_res(result + j);
      }
    }
    j++;
    result += j;
    temp.length -= j;
    temp.items += j;
  }
defer:
  free(heads);
  free(delim_lengths);
  return result;
}
