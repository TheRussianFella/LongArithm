#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "bn.h"

typedef unsigned char limb_type;
typedef unsigned short bigger_type;
#define NUM_NEW_LIMBS 10

// Data structure


struct bn_s {
  limb_type* limbs;

  int num_limbs;
  int occupied_limbs;

  char sign;
  int radix;
};

/////

// Creation deletion

bn* bn_new() {
  bn* new = (bn*) malloc(sizeof(bn));

  new -> limbs = (limb_type*) calloc(sizeof(limb_type), NUM_NEW_LIMBS);
  new -> num_limbs = NUM_NEW_LIMBS;

  new -> sign = 1;
  new -> radix = 0;
  new -> occupied_limbs = 0;

  return new;
}

int bn_delete(bn* t) {
  free(t -> limbs);
  free(t);

  return BN_OK;
}

// Utilities

int bn_increase_size(bn* t) {

  limb_type* old_limbs = t->limbs;

  t->limbs = (limb_type*) calloc(sizeof(limb_type), t->num_limbs*1.25);

  for (int i = 0; i < t->num_limbs; ++i)
    t->limbs[i] = old_limbs[i];

  t->num_limbs *= 1.25;

  free(old_limbs);

  return BN_OK;
}


int bn_add_limb_to_limb(bn* t, limb_type number, int limb_idx) {

  limb_type MAX = pow(2, sizeof(limb_type)*8)-1;
  char curry = (MAX - number) < t->limbs[limb_idx];

  t->limbs[limb_idx] += number;

  if (curry) {
    if (limb_idx + 1 == t->num_limbs)
      bn_increase_size(t);
    bn_add_limb_to_limb(t, 1, limb_idx + 1);
  }

  return BN_OK;
}


int bn_multiply_limb_by_limb(bn* t, limb_type number, int limb_idx) {

  bigger_type target_limb = t->limbs[limb_idx], target_number = number;
  bigger_type result = target_limb * target_number;

  bigger_type mask = pow(2, sizeof(limb_type)*8)-1;

  t->limbs[limb_idx] = result & mask;

  limb_type curry = (result & ~mask) >> sizeof(limb_type)*8;

  if (limb_idx + 1 == t->num_limbs)
    bn_increase_size(t);
  else {
    if (t->limbs[limb_idx+1] != 0)
      bn_multiply_limb_by_limb(t, number, limb_idx+1);
  }

  bn_add_limb_to_limb(t, curry, limb_idx+1);

  return BN_OK;
}

int bn_to_decimal(bn* t) {
  int ans = 0;

  for (int i = 0; i < t->num_limbs; ++i)
    ans += t->limbs[i] * pow(pow(2, sizeof(limb_type)*8), i);

  return ans;
}
// Initialization

int bn_init_string_radix(bn *t, const char *init_string, int radix) {

  int sign = 1, start_idx = 0;
  if (init_string[0] == '-') {
    sign = 0;
    start_idx = 1;
  }

  for (int i = start_idx; init_string[i]!='\0'; ++i) {
    bn_multiply_limb_by_limb(t, radix, 0);
    bn_add_limb_to_limb(t, init_string[i]-'0', 0);
  }

  t -> radix = radix;

  return BN_OK;
}

///////////////////////

int main() {

  bn* big_number = bn_new();

  bn_init_string_radix(big_number, "101010101010010101", 2);

  printf("%d %d %d\n", big_number->limbs[0], big_number->limbs[1], big_number->limbs[2]);
  printf("%d", bn_to_decimal(big_number));


  bn_delete(big_number);

  return 0;
}
