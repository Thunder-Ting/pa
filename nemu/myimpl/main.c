#include "tools.h"
#include <stdio.h>

#define LOG(format, ...) printf(format, ##__VA_ARGS__)
int main() {
  LOG("Hello %d", 3);
  return 0;
}
