// #include "tools.h"
#include <regex.h>
#include <stdio.h>
#define LOG(format, ...) printf(format, ##__VA_ARGS__)
// int main() {
//   LOG("Hello %d\n", 3);
//   return 0;
// }
#define CONACT(A, B) A##B

void a() {
  char ab[3] = "ab";
  char b[3] = "cd";
  LOG("hello %d %s\n", 234, b);
  LOG("%s", CONACT(a, b));
}

int test() {
  regex_t reg;
  const char *text = "3*5";

  regcomp(&reg, "\\*", REG_EXTENDED);

  if (regexec(&reg, text, 0, NULL, 0) == 0) {
    printf("found '*'\n");
  } else {
    printf("not found\n");
  }

  regfree(&reg);
  return 0;
}

int main() {
  test();
  return 0;
}