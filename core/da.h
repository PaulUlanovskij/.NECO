#pragma once
#include <stdio.h>
#include <stdlib.h>

#define define_simple_da(T, name)                                              \
  typedef struct {                                                             \
    T *items;                                                                  \
    int length;                                                                \
    int capacity;                                                              \
  } name

define_simple_da(short, short_da);
define_simple_da(unsigned short, ushort_da);
define_simple_da(int, int_da);
define_simple_da(unsigned int, uint_da);
define_simple_da(long, long_da);
define_simple_da(unsigned long, ulong_da);
define_simple_da(float, float_da);
define_simple_da(double, double_da);

#define da_slice(xs, from, till)                                               \
do {                                                                           \
  if(from < 0 || from + till > xs.length){                                     \
    printf("[DA] Was unable to make a slice %s:%d\n", __FILE__, __LINE__);     \
    exit(1);                                                                   \
  }                                                                            \
  (xs).items += from;                                                           \
  (xs).length = (till == -1) ? (xs).length - from : till - from;               \
}while(0)

#define da_reserve(xs, size)                                                   \
  do {                                                                         \
    if ((xs)->capacity < (size)) {                                             \
      (xs)->capacity = size - 1;                                               \
      (xs)->capacity |= (xs)->capacity >> 1;                                   \
      (xs)->capacity |= (xs)->capacity >> 2;                                   \
      (xs)->capacity |= (xs)->capacity >> 4;                                   \
      (xs)->capacity |= (xs)->capacity >> 8;                                   \
      (xs)->capacity |= (xs)->capacity >> 16;                                  \
      (xs)->capacity++;                                                        \
      (xs)->items = (typeof((xs)->items))realloc(                              \
          (xs)->items, (xs)->capacity * sizeof(*(xs)->items));                 \
    }                                                                          \
  } while (0)


#define da_remove_at(xs, index)                                                \
  (xs)->items[(index)] = (xs)->items[(xs)->length--]

#define da_remove_at_ord(xs, iter, index)                                      \
do{                                                                            \
for(int iter = index; iter < (xs)->length-1; iter++)                           \
  (xs)->items[(index)] = (xs)->items[iter+1];                                  \
}while(0)

#define da_foreach(xs, name)                                                   \
  for (typeof((xs)->items) name = (xs)->items;                                 \
       name < (xs)->items + (xs)->length; name++)

#define da_for(xs, index, name)                                                \
  for (int index = 0; index < (xs)->length; )   \
       for(typeof((xs)->items) name = (xs)->items;\
           index < (xs)->length;\
           index++, name = &(xs)->items[index])

#define da_free(xs)                                                            \
  do {                                                                         \
    if ((xs)->items)                                                           \
      free((xs)->items);                                                       \
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
    *(dest) =                                                                  \
        (typeof((src)->items))calloc(((src)->length), sizeof(*(src)->items));  \
    memcpy(*(dest), (src)->items, sizeof(*(src)->items) * ((src)->length));    \
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
