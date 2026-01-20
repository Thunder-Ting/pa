#include "tools.h"
#include <stdio.h>

int main() {
  char buf[] = "  a,b,,c,d  ";
  char *saveptr = NULL;
  char *token;

  token = strtok_m_r(buf, ", ", &saveptr);
  while (token) {
    printf("[%s]\n", token);
    token = strtok_m_r(NULL, ", ", &saveptr);
  }

  return 0;
}
