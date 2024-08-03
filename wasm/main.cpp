#include <emscripten.h>
#include <stdio.h>
#include <string.h>

int main() {
  puts("hello world!\n");
  char *c = new char[1024];
  strcpy(c, "hello niu2x");
  printf("%s\n", c);
  delete []c;
  return 0;
}