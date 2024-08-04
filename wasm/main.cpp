#include <emscripten.h>
#include <stdio.h>
#include <string.h>

using Callback = void (*)(int i);

static void test(int x) {}

int main() {
  Callback f = &test;
  f(32);
  return 0;
}