#include "tools.h"
#include <string.h>

char *strtok_m(char *str, const char *delim) {
  static char *last;
  char *start;
  if (str != NULL)
    last = str;
  if (last == NULL)
    return NULL;

  while (*last && strchr(delim, *last))
    last++;

  if (*last == '\0') {
    last = NULL;
    return NULL;
  }
  start = last;
  while (*last && !strchr(delim, *last))
    last++;

  if (*last) {
    *last = '\0';
    last++;
  } else {
    last = NULL;
  }
  return start;
}

char *strtok_m_r(char *str, const char *delim, char **saveptr) {
  char *start;

  if (str != NULL)
    *saveptr = str;
  if (*saveptr == NULL)
    return NULL;

  // 跳过前导分隔符
  while (**saveptr && strchr(delim, **saveptr))
    (*saveptr)++;

  if (**saveptr == '\0') {
    *saveptr = NULL;
    return NULL;
  }

  start = *saveptr;

  // 扫描 token
  while (**saveptr && !strchr(delim, **saveptr))
    (*saveptr)++;

  if (**saveptr) { // 字符串是否结束
    **saveptr = '\0';
    (*saveptr)++;
  } else {
    *saveptr = NULL;
  }

  return start;
}
