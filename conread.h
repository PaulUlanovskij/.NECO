#pragma once
#include "helpers.h"

#include <ctype.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>

char *conread_line(char *prompt, pubool ensure_nonempty) {
  if (prompt != NULL) {
    printf("%s : ", prompt);
  }

  int c;
  string line = {};
  char *res = NULL;

START_OVER:
  while ((c = getchar()) != '\n' && c != EOF) {
    da_append(&line, c);
  }

  if (c == EOF || line.length == 0) {
    if (ensure_nonempty) {
      printf("Помилка. Рядок має бути не пустим : ");
      goto START_OVER;
    }
    return (char *)"";
  }

  if (ensure_nonempty) {
    da_foreach(&line, i) {
      if (i != NULL && isspace(*i) == false) {
        goto COPY_AND_RETURN;
      }
    }
    printf("Помилка. Рядок має бути не пустим : ");
    goto START_OVER;
  } else {
    goto COPY_AND_RETURN;
  }

COPY_AND_RETURN:
  da_copy_terminate(res, &line);
  da_free(&line);

  return res;
}
void clear_input_buffer() {
  int c;
  while ((c = getchar()) != '\n' && c != EOF) {
  }
}

#define define_predicate(type, name, expr, error)                              \
  pubool name(type x, char **err_message) {                                    \
    *err_message = error;                                                      \
    return expr;                                                               \
  }

#define define_conread(name, type, fmt, err_m)                                 \
  type conread_##name(char *prompt, pubool (*predicate)(type, char **)) {      \
    if (prompt != NULL) {                                                      \
      printf("%s : ", prompt);                                               \
    }                                                                          \
    type input = 0;                                                            \
    char *error_message = {0};                                                 \
                                                                               \
    while (true) {                                                             \
      if (scanf(fmt, &input) == 1 && getchar() == '\n') {                      \
        if (predicate == NULL || predicate(input, &error_message) == true) {   \
          break;                                                               \
        } else {                                                               \
          printf("Помилка. %s : ", error_message);                                      \
        }                                                                      \
      } else {                                                                 \
        printf("Помилка. %s : ", err_m);                                                         \
        clear_input_buffer();                                                  \
      }                                                                        \
    }                                                                          \
                                                                               \
    return input;                                                              \
  }
define_conread(int, int, "%d", "Введіть ціле число");
define_conread(uint, unsigned int, "%u",
               "Введіть ціле невід'ємне число");
define_conread(long, long, "%ld", "Введіть ціле число");
define_conread(long_hex, long, "%ld",
               "Введіть ціле шістнядцяткове число");
define_conread(float, float, "%f", "Введіть дійсне число");

unsigned int conread_option(char **options, int length) {
  assert(options != NULL);
  for (int i = 0; i < length; i++) {
    assert(options[i] != NULL);
  }

  puts("______________________________Навігація_____________________________");
  for (int i = 0; i < length; i++) {
    printf("%d. %s\n", i + 1, options[i]);
  }

  int input = conread_long(NULL, NULL);
  while (input < 1 || input > length) {
    printf("Неправильний пункт навігації. Доступні пункти навігації між 1 і "
           "%u: ",
           length);
    input = conread_long(NULL, NULL);
  }

  return input;
}
