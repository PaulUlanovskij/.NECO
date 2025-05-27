#define da_reserve(xs, size)                                                   \
  do {                                                                         \
    if ((xs)->capacity < size) {                                               \
      (xs)->capacity = size;                                                   \
    }                                                                          \
    (xs)->items =                                                              \
        (typeof((xs)->items)) realloc((xs)->items, (xs)->capacity * sizeof(*(xs)->items));    \
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

#define da_copy_terminate(dest, src)                                           \
  do {                                                                         \
    (dest) = (typeof((src)->items))calloc(((src)->length + 1),                 \
                                          sizeof(*(src)->items));              \
    memcpy((dest), (src)->items, sizeof(*(src)->items) * ((src)->length));     \
  } while (0)

#define da_copy(dest, src)                                                     \
  do {                                                                         \
    (dest) =                                                                   \
        (typeof((src)->items))calloc(((src)->length), sizeof(*(src)->items));  \
    memcpy((dest), (src)->items, sizeof(*(src)->items) * ((src)->length));     \
  } while (0)

#define da_append(xs, x)                                                       \
  do {                                                                         \
    if ((xs)->length == (xs)->capacity) {                                      \
      if ((xs)->capacity == 0)                                                 \
        (xs)->capacity = 16;                                                   \
      else                                                                     \
        (xs)->capacity *= 2;                                                   \
    }                                                                          \
    (xs)->items = (typeof((xs)->items))realloc(                                \
        (xs)->items, (xs)->capacity * sizeof(*(xs)->items));                   \
    (xs)->items[(xs)->length++] = (x);                                         \
  } while (0)


