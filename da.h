#ifndef DA_INIT_CAPACITY
#define DA_INIT_CAPACITY 16
#endif // DA_INIT_CAPACITY

#define da_reserve(xs, size)                                                   \
  do {                                                                         \
    while ((xs)->capacity < size) {                                            \
      if ((xs)->capacity == 0)                                                 \
        (xs)->capacity = DA_INIT_CAPACITY;                                     \
      else                                                                     \
        (xs)->capacity *= 2;                                                   \
    }                                                                          \
    (xs)->items = (typeof((xs)->items))realloc(                                \
        (xs)->items, (xs)->capacity * sizeof(*(xs)->items));                   \
  } while (0)

#define da_foreach(xs, name)                                                   \
  for (typeof((xs)->items) name = (xs)->items;                                 \
       name < (xs)->items + (xs)->length; name++)

#define da_free(xs)                                                            \
  do {                                                                         \
    free((xs)->items);                                                         \
    (xs)->capacity = 0;                                                        \
    (xs)->length = 0;                                                          \
  } while (0)

#define da_append_null(xs)                                                     \
  do {                                                                         \
    da_reserve((xs), (xs)->length + 1);                                        \
    memset((xs)->items + (xs)->length, 0, sizeof(*(xs)->items));               \
    (xs)->length++;                                                            \
  } while (0)

#define da_copy_items(dest, src)                                               \
  do {                                                                         \
    (dest) =                                                                   \
        (typeof((src)->items))calloc(((src)->length), sizeof(*(src)->items));  \
    memcpy((dest), (src)->items, sizeof(*(src)->items) * ((src)->length));     \
  } while (0)

#define da_copy(dest, src)                                                     \
  do {                                                                         \
    da_copy_items((dest)->items, (src));                                       \
    (dest)->length = (src)->length;                                            \
  } while (0)

#define da_append(xs, x)                                                       \
  do {                                                                         \
    da_reserve((xs), (xs)->length + 1);                                        \
    (xs)->items[(xs)->length++] = (x);                                         \
  } while (0)

#define da_append_many(xs, name, as, n)                                        \
  do {                                                                         \
    da_reserve((xs), (xs)->length + n);                                        \
    for (typeof((as)) name = (as); name < (as) + n; name++) {                  \
      (xs)->items[(xs)->length] = name;                                        \
      (xs)->length++;                                                          \
    }                                                                          \
  } while (0)
