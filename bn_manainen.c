#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "bn.h"

typedef unsigned char limb_type;
typedef unsigned short bigger_type;
#define NUM_NEW_LIMBS 10

// Data structure


struct bn_s {
  limb_type* limbs;

  unsigned int num_limbs;
  unsigned int occupied_limbs;

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

int bn_add_limbs(bn* t, int num_limbs) {

  limb_type* old_limbs = t->limbs;

  t->limbs = (limb_type*) calloc(sizeof(limb_type), t->num_limbs+num_limbs);

  for (int i = 0; i < t->num_limbs; ++i)
    t->limbs[i] = old_limbs[i];

  t->num_limbs += num_limbs;

  free(old_limbs);

  return BN_OK;
}

int bn_increase_size(bn* t) {
  return bn_add_limbs(t, t->num_limbs*0.25);
}

int bn_digit_shift(bn* t, int num_digit) {

  if (num_digit) {
    if (t->occupied_limbs+num_digit > t->num_limbs)
      bn_add_limbs(t, (t->occupied_limbs+num_digit - t->num_limbs)*1.25);

    for (int i = t->occupied_limbs-1; i >= 0; --i){
      t->limbs[i+num_digit] = t->limbs[i];
      t->limbs[i] = 0;
    }
  }

  t->occupied_limbs += num_digit;

  return BN_OK;
}

unsigned long long bn_to_decimal(bn* t) {
  unsigned long long ans = 0;

  for (int i = 0; i < t->num_limbs; ++i)
    ans += t->limbs[i] * pow(pow(2, sizeof(limb_type)*8), i);

  return ans;
}

// By limb operations

int bn_add_limb_to_limb(bn* t, limb_type number, int limb_idx) {

  limb_type MAX = pow(2, sizeof(limb_type)*8)-1;
  char curry = (MAX - number) < t->limbs[limb_idx];

  if (limb_idx+1 > t->occupied_limbs && number)
    t->occupied_limbs += 1;

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

  if (limb_idx < t->occupied_limbs){

    if (limb_idx + 1 == t->num_limbs)
      bn_increase_size(t);
    else {
      //if (t->limbs[limb_idx+1] != 0)
      bn_multiply_limb_by_limb(t, number, limb_idx+1);
    }

    bn_add_limb_to_limb(t, curry, limb_idx+1);
  }

  return BN_OK;
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

int bn_init_string(bn* t, const char *init_string) {
  return bn_init_string_radix(t, init_string, 10);
}

bn *bn_init(bn const *orig) {

  bn* new = (bn*) malloc(sizeof(bn));

  new->limbs = (limb_type*) calloc(sizeof(limb_type), orig->num_limbs);
  new->num_limbs = orig->num_limbs;

  new -> sign = orig -> sign;
  new -> radix = orig -> radix;

  for (int i = 0; i < orig -> occupied_limbs; ++i)
    new->limbs[i] = orig->limbs[i];

  new -> occupied_limbs = orig -> occupied_limbs;

  return new;
}

int bn_equals_init(bn* t, bn const *orig) {

  if (t->num_limbs > orig->occupied_limbs) {

    memset(t->limbs, 0, t->num_limbs);
    for (int i = 0; i < orig->occupied_limbs; ++i)
        t->limbs[i] = orig->limbs[i];

    t -> occupied_limbs = orig->occupied_limbs;
    t -> radix = orig -> radix;
    t -> sign = orig -> sign;

  } else {

    bn_delete(t);
    t = bn_init(orig);
  }

  return BN_OK;
}

// One argument operations

int bn_add_to(bn *t, bn const *right) {

  for (int i = 0; i < right->occupied_limbs; ++i)
    bn_add_limb_to_limb(t, right->limbs[i], i);

  return BN_OK;
}

int negate(bn* t, int num_limbs) {
  for (int i = 0; i < num_limbs; ++i)
    t->limbs[i] = ~(t->limbs[i]);
  t->occupied_limbs = num_limbs;
  bn_add_limb_to_limb(t, 1, 0);
}

int bn_sub_to(bn *t, bn const *right) {

  bn* negative_num = bn_init(right);

  int biggest_size = negative_num->occupied_limbs > t->occupied_limbs ? negative_num->occupied_limbs : t->occupied_limbs;

  negate(negative_num, biggest_size);

  bn_add_to(t, negative_num);

  //t->limbs[0] += 1;
  t->limbs[t->occupied_limbs-1] = 0;
  t->occupied_limbs -= 1;

  //t->limbs = realloc(t->limbs, sizeof(limb_type)*biggest_size);
  //t->occupied_limbs = biggest_size;

  return BN_OK;
}

// Two argument operations

bn* bn_add(bn const *left, bn const *right) {

  bn* answer = bn_init(left);
  bn_add_to(answer, right);

  return answer;
}

bn* bn_mul(bn const *left, bn const *right) {

  bn* answer = bn_new();
  bn* temp = bn_init(left);

  for (int i = 0; i < right->occupied_limbs; ++i) {

    bn_multiply_limb_by_limb(temp, right->limbs[i], 0);
    bn_digit_shift(temp, i);

    bn_add_to(answer, temp);

    bn_equals_init(temp, left);
  }

  return answer;
}

///////////////////////

int main() {

  bn* first = bn_new();
  bn* second = bn_new();

  bn_init_string_radix(first, "2048", 10);
  bn_init_string_radix(second, "1024", 10);

  bn_sub_to(first, second);

  printf("\n %llu", bn_to_decimal(first));

  bn_delete(first);
  bn_delete(second);

  return 0;
}
