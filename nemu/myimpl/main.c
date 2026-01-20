// #include "tools.h"
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

int main() {
  int a = 10;
  printf("%d", a);
  return 0;
}