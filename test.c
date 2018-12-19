#include <stdio.h>
#include <stdlib.h>

#include "bn.h"

/*
int main() {

  bn* first = bn_new();
  bn* second = bn_new();

  bn_init_string_radix(first, "465748589", 10);
  bn_init_string_radix(second, "50", 10);

  struct dr res = bn_div_full(first, second);

  bn_delete(first);
  bn_delete(second);

  bn_delete(res.full);
  bn_delete(res.leftover);

  return 0;
}
*/

int main() {

  bn* first = bn_new();

  bn_init_string(first, "10349803294823094832094823094802394802398402398029384092384082340983240923840938204982304982309482309483048230482309482039480239480294802984029348023948023948023948023984023984023984092384032984023948");

  //bn_init_string(first, "245");

  char* temp = bn_to_string(first, 2);

  printf("%s\n", temp);

  bn_delete(first);
  free(temp);

  return 0;
}
