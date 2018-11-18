#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

//#include "bn.h"

///////////////
struct bn_s;
typedef struct bn_s bn;

enum bn_codes {
BN_OK, BN_NULL_OBJECT, BN_NO_MEMORY, BN_DIVIDE_BY_ZERO
};

bn *bn_new(); // Создать новое BN
bn *bn_init(bn const *orig); // Создать копию существующего BN
// Инициализировать значение BN десятичным представлением строки
int bn_init_string(bn *t, const char *init_string);
// Инициализировать значение BN представлением строки
// в системе счисления radix
int bn_init_string_radix(bn *t, const char *init_string, int radix);
// Инициализировать значение BN заданным целым числом
int bn_init_int(bn *t, int init_int);
// Уничтожить BN (освободить память)
int bn_delete(bn *t);

// Операции, аналогичные +=, -=, *=, /=, %=
int bn_add_to(bn *t, bn const *right);
int bn_sub_to(bn *t, bn const *right);
int bn_mul_to(bn *t, bn const *right);
int bn_div_to(bn *t, bn const *right);
int bn_mod_to(bn *t, bn const *right);
// Возвести число в степень degree
int bn_pow_to(bn *t, int degree);
// Извлечь корень степени reciprocal из BN (бонусная функция)
int bn_root_to(bn *t, int reciprocal);
// Аналоги операций x = l+r (l-r, l*r, l/r, l%r)
bn* bn_add(bn const *left, bn const *right);
bn* bn_sub(bn const *left, bn const *right);
bn* bn_mul(bn const *left, bn const *right);
bn* bn_div(bn const *left, bn const *right);
bn* bn_mod(bn const *left, bn const *right);
// Выдать представление BN в системе счисления radix в виде строки
// Строку после использования потребуется удалить.
const char *bn_to_string(bn const *t, int radix);
// Если левое меньше, вернуть <0; если равны, вернуть 0; иначе >0
int bn_cmp(bn const *left, bn const *right);
int bn_neg(bn *t); // Изменить знак на противоположный
int bn_abs(bn *t); // Взять модуль
int bn_sign(bn const *t); //-1 если t<0; 0 если t = 0, 1 если t>0
///////////////////////////


typedef unsigned char limb_type;
typedef unsigned short bigger_type;
#define NUM_NEW_LIMBS 10

// Data structure


struct bn_s {
  limb_type* limbs;

  unsigned int num_limbs;
  unsigned int occupied_limbs;

  unsigned char sign;
  int radix;
};

/////

// Creation deletion
// Shortcut specification for sign: 1 == '+' 0 == '-'
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

  if (num_digit && t->occupied_limbs) {
    if (t->occupied_limbs+num_digit > t->num_limbs)
      bn_add_limbs(t, (t->occupied_limbs+num_digit - t->num_limbs)*1.25);

    for (int i = t->occupied_limbs-1; i >= 0; --i){
      t->limbs[i+num_digit] = t->limbs[i];
      t->limbs[i] = 0;
    }

    t->occupied_limbs += num_digit;
  }

  return BN_OK;
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

int bn_equals_init(bn*, bn const*);

typedef struct sd {
  limb_type result;
  bn* leftover;
} single_devide_result;

single_devide_result bn_limb_devide(bn const *a, bn const *b) {
  // Fucks up, when a or b is negative

  single_devide_result answer;
  answer.result = 0;

  int cmp = bn_cmp(a, b);

  if (cmp == 1) {
    answer.result = 0;
    answer.leftover = bn_init(a);

  } else if (cmp == 0) {
    answer.result = 1;
    answer.leftover = bn_new();

  } else {

    bigger_type left = 0, right = pow(2, sizeof(limb_type)*8);

    bn* mult_result = bn_new();

    while (right-left > 1) {

      float distance = right-left;
      bigger_type middle = left + ceil(distance / 2);

      bn_equals_init(mult_result, b);
      bn_multiply_limb_by_limb(mult_result, middle, 0);

      cmp = bn_cmp(mult_result, a);

      if (cmp == 0) {
        answer.result = middle;
        answer.leftover = bn_new();
        break;
      } else if (cmp == 1) {
        left = middle;
      } else {
        right = middle;
      }
    }

    if (!answer.result) {

      bn_equals_init(mult_result, b);
      bn_multiply_limb_by_limb(mult_result, left, 0);

      answer.result = left;

      bn* leftover = bn_sub(a, mult_result);

      answer.leftover = leftover;
    }

    bn_delete(mult_result);
  }

  return answer;
}

// Initialization

int bn_init_string_radix(bn *t, const char *init_string, int radix) {

  int sign = 1, start_idx = 0;
  if (init_string[0] == '-') {
    sign = 0;
    start_idx = 1;
  }
  t -> sign = sign;

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

/*
int bn_init_int(bn *t, int init_int) {


}
*/
// Signs and comparison

int bn_abs(bn *t) {
  t->sign = 1;
  return BN_OK;
}

int bn_neg(bn *t) {
  t->sign ^= 1;
  return BN_OK;
}

int bn_abs_cmp(bn const *left, bn const *right) {

  if (left->occupied_limbs != right->occupied_limbs) {
    if (left->occupied_limbs > right->occupied_limbs)
      return -1;
    else
      return 1;
  }

  for (int i = left->occupied_limbs-1; i>=0; --i) {
    if (left->limbs[i] != right->limbs[i]) {
      if (left->limbs[i] > right->limbs[i])
        return -1;
      else
        return 1;
    }
  }

  return 0;

}

int bn_cmp(bn const *left, bn const *right) {

  if (left->sign != right->sign){
    if (left->sign)
      return -1;
    else
      return 1;
  }

  int positive = left->sign ? 1 : -1;

  return bn_abs_cmp(left, right) * positive;

}

// One argument operations

int bn_add_to(bn *t, bn const *right) {

  if ( (!t->sign && !right->sign) || (t->sign && right->sign)){
    for (int i = 0; i < right->occupied_limbs; ++i)
      bn_add_limb_to_limb(t, right->limbs[i], i);

  } else {
    // Plus negative == Minus positive
    bn* right_copy = bn_init(right);

    if (!t->sign) {
      limb_type* temp = t->limbs;
      t->limbs = right_copy->limbs;
      right_copy->limbs = temp;

      unsigned int temp_size = t->occupied_limbs;
      t->occupied_limbs = right_copy->occupied_limbs;
      right_copy->occupied_limbs = temp_size;

      bn_abs(t);
    } else
      bn_abs(right_copy);

    bn_sub_to(t, right_copy);
  }

  return BN_OK;
}

int negate(bn* t, int num_limbs) {
  for (int i = 0; i < num_limbs; ++i)
    t->limbs[i] = ~(t->limbs[i]);
  t->occupied_limbs = num_limbs;
  bn_add_limb_to_limb(t, 1, 0);
  return BN_OK;
}

int bn_sub_to(bn *t, bn const *right) {

  bn* right_copy = bn_init(right);

  if (!right->sign) {
    // Minus negative == Plust positive
    bn_abs(right_copy);
    bn_add_to(t, right_copy);
  } else if (!t->sign) {
    // Negative - positive = negative + (-positive)
    bn_abs(t);
    bn_add_to(t, right);
    bn_neg(t);
  } else {

    int cmp = bn_cmp(t, right_copy);

    if (cmp == 1) {
      limb_type* temp = t->limbs;
      t->limbs = right_copy->limbs;
      right_copy->limbs = temp;

      unsigned int temp_size = t->occupied_limbs;
      t->occupied_limbs = right_copy->occupied_limbs;
      right_copy->occupied_limbs = temp_size;
    }

    if (cmp != 0) {
      bn* negative_num = bn_init(right_copy);

      int biggest_size = negative_num->occupied_limbs > t->occupied_limbs ?
                          negative_num->occupied_limbs : t->occupied_limbs;

      negate(negative_num, biggest_size);

      bn_add_to(t, negative_num);

      t->limbs[t->occupied_limbs-1] = 0;
      t->occupied_limbs -= 1;

      if (cmp == 1)
        t->sign=0;

    } else {
      memset(t->limbs, 0, t->occupied_limbs);
      t->occupied_limbs = 0;
    }
  }

  if(t->occupied_limbs != 0){
    while(1){
      if(t->limbs[t->occupied_limbs-1] == 0)
        t->occupied_limbs -= 1;
      else
        break;
      }
  }

  return BN_OK;
}

// Two argument operations

bn* bn_add(bn const *left, bn const *right) {

  bn* answer = bn_init(left);
  bn_add_to(answer, right);

  return answer;
}

bn* bn_sub(bn const *left, bn const *right) {

  bn* answer = bn_init(left);
  bn_sub_to(answer, right);

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

  if (left->sign == right->sign)
    answer -> sign = 1;
  else
    answer -> sign = 0;

  return answer;
}



typedef struct dr {
  bn* full;
  bn* leftover;
} division_result;


long long bn_to_decimal(bn* t);
division_result bn_div_full(bn const *left, bn const *right) {

  division_result result;
  result.full = bn_new();
  result.leftover = bn_new();

  int cmp = bn_cmp(left, right);

  if (cmp == 1) {
    result.leftover = bn_init(left);
  } else if (cmp == 0) {
    bn_init_string(result.full, "1");
  } else {

    bn* to_devide = bn_new();

    for (int i = left->occupied_limbs-1; i>=0; --i) {

      //printf("Before shift: %d, %lld\n", to_devide->occupied_limbs, bn_to_decimal(to_devide));
      bn_digit_shift(to_devide, 1);
      //printf("After shift: %d, %lld\n", to_devide->occupied_limbs, bn_to_decimal(to_devide));
      bn_digit_shift(result.full, 1);

      //printf("%lld %lld \n", bn_to_decimal(to_devide), bn_to_decimal(result.full));

      //printf("%d\n", left->limbs[i]);
      bn_add_limb_to_limb(to_devide, left->limbs[i], 0);

      //printf("%lld %lld \n", bn_to_decimal(to_devide), bn_to_decimal(result.full));

      //printf("Deviding: %lld\n", bn_to_decimal(to_devide));

      single_devide_result temp_res = bn_limb_devide(to_devide, right);

      //printf("%d %lld\n", temp_res.result, bn_to_decimal(temp_res.leftover));
      //printf("Leftover: %d, %lld\n", temp_res.leftover->occupied_limbs,  bn_to_decimal(temp_res.leftover));

      bn_equals_init(to_devide, temp_res.leftover);
      bn_add_limb_to_limb(result.full, temp_res.result, 0);

      //printf("%lld %lld \n", bn_to_decimal(to_devide), bn_to_decimal(result.full));

    }

    bn_equals_init(result.leftover, to_devide);
    bn_delete(to_devide);
  }

  return result;
}

bn* bn_div(bn const *left, bn const *right) {
  division_result result = bn_div_full(left, right);
  bn_delete(result.leftover);

  if (left->sign == right->sign)
    result.full->sign = 1;
  else
    result.full->sign = 0;

  return result.full;
}

bn* bn_mod(bn const *left, bn const *right) {
  division_result result = bn_div_full(left, right);
  bn_delete(result.full);
  return result.leftover;
}

// Bad boy that ruins beautiful structure

int bn_mul_to(bn *t, bn const *right) {

  bn* answer = bn_mul(t, right);
  bn_delete(t);
  t = bn_init(answer);

  return BN_OK;
}

int bn_div_to(bn *t, bn const *right) {

  bn* answer = bn_div(t, right);
  bn_delete(t);
  t = bn_init(answer);

  return BN_OK;
}

int bn_mod_to(bn *t, bn const *right) {

  bn* answer = bn_mod(t, right);
  bn_delete(t);
  t = bn_init(answer);

  return BN_OK;
}

// Printing functions

long long bn_to_decimal(bn* t) {
  long long ans = 0;

  for (int i = 0; i < t->num_limbs; ++i)
    ans += t->limbs[i] * pow(pow(2, sizeof(limb_type)*8), i);

  return ans * (t->sign ? 1 : -1);
}


char *strrev(char *str)
{
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return str;
}

const char *bn_to_string(bn const *t, int radix) {

  char* result_string = (char*) calloc(t->occupied_limbs*4, sizeof(char));

  if (t->occupied_limbs == 0) {
    result_string[0] = '0';
    result_string[1] = '\0';
    return (const char*) result_string;
  }

  bn* temp = bn_init(t);
  bn_abs(temp);

  bn* long_radix = bn_new();
  bn_init_string(long_radix, "10");

  for (int i = 0; 1; ++i) {

    /*
    printf("New num\n");
    for(int j = 0; j<temp->occupied_limbs; ++j)
      printf("%d ", temp->limbs[j]);
    printf("\n");
*/
    division_result res = bn_div_full(temp, long_radix);
//    printf("%lld %lld %lld\n", bn_to_decimal(temp), bn_to_decimal(res.full), bn_to_decimal(res.leftover));

    result_string[i] = res.leftover->limbs[0]+'0';
    bn_equals_init(temp, res.full);

    bn_delete(res.full);
    bn_delete(res.leftover);

    if (temp->occupied_limbs == 0) {
      if (!t->sign) {
        result_string[i+1] = '-';
        result_string[i+2] = '\0';
      } else
        result_string[i+1] = '\0';

      result_string = strrev(result_string);

      break;
    }
  }

  bn_delete(temp);
  bn_delete(long_radix);
  return (const char*) result_string;
}

///////////////////////

int main() {

  bn* first = bn_new();
  bn* second = bn_new();

  char* temp_str = (char*) calloc(sizeof(char), 100000);
  char new_char;

  scanf("%c", &new_char);
  int i;
  for(i = 0; new_char!='\n'; ++i) {
    temp_str[i] = new_char;
    scanf("%c", &new_char);
  }
  temp_str[i] = '\0';

  bn_init_string_radix(first, temp_str, 10);

  scanf("%c", &new_char);
  int sum = new_char=='+';

  scanf("%c", &new_char);
  scanf("%c", &new_char);
  for(i = 0; new_char!='\n'; ++i) {
    temp_str[i] = new_char;
    scanf("%c", &new_char);
  }
  temp_str[i] = '\0';

  bn_init_string_radix(second, temp_str, 10);

  if (sum)
    bn_add_to(first, second);
  else
    bn_sub_to(first, second);

  const char* result = bn_to_string(first, 10);

  printf("%s\n", result);

  bn_delete(first);
  bn_delete(second);

  free(temp_str);
  free((char*)result);
}
