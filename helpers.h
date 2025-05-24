#pragma once
#include <assert.h>
#include <stdlib.h>

#define pubool char

#if !__bool_true_false_are_defined
#define true 1
#define false 0
#define bool pubool
#endif

typedef struct {
  char *items;
  int length;
  int capacity;
} string;

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

#define unreachable assert(0)
#define len(arr) (sizeof((arr)) / sizeof((arr)[0]))

#define ll_append(xs, x, nullness_check)                                       \
  do {                                                                         \
    if (nullness_check) {                                                      \
      (xs)->this = (x);                                                        \
    } else {                                                                   \
      while ((xs)->next != NULL) {                                             \
        (xs) = (xs)->next;                                                     \
      }                                                                        \
      (xs)->next = malloc(sizeof(*xs));                                        \
      (xs)->next->next = NULL;                                                 \
      (xs)->next->this = (x);                                                  \
    }                                                                          \
  } while (0)

#define ll_dealloc(xs, temp, prev)                                             \
  do {                                                                         \
    typeof((xs)) temp = xs;                                                    \
    typeof((xs)) prev = NULL;                                                  \
    while ((xs)->next != NULL) {                                               \
      for (temp = (xs); temp->next != NULL; temp = temp->next) {    \
        prev = temp;                                                           \
      }                                                                        \
      prev->next = NULL;                                                       \
        free(temp);                                                            \
    }                                                                          \
  } while (0)

#define ll_foreach(xs, name)                                                   \
  for (typeof(xs) name = (xs); name != NULL; name = name->next)
