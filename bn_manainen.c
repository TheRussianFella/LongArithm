#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "bn.h"

#define ADDITIONAL_LENGTH 0.25

/// Structure definition

struct bn_s {
	int* number_array;

	int array_length;
	int number_length;
};

//////


/// Misc functions

// Function to print long number
int bn_print(const bn* t) {
	for (int i = 0; i < t->number_length; ++i)
		printf("%d", t->number_array[(t->array_length)-(t->number_length) + i]);

	printf("\n");	

	return 0;
};

// Function to copy elements from the end of one array to another one
int copy_to_end(int* to, const int* from, const int to_len, const int from_len) {

	assert(to_len >= from_len);

	for (int i = 0; i < from_len; ++i)
		to[to_len-1 - i] = from[from_len-1 - i];
	
	return 0;
}

// Create list of integers from string
int* string_to_int_list(const char* string) {
	int length = strlen(string);

	int* int_arr = (int*) malloc(length * sizeof(int));

	for (int i=0; i < length; ++i)
		int_arr[i] = string[i] - '0';

	return int_arr;
};

//////


/// Creation functions
bn* bn_new() {
	return (bn*) malloc(sizeof(bn));
};

int bn_delete(bn* t) {

	free(t->number_array);
	free(t);

	return 0;
};

int bn_change_size(bn* t, int new_array_length) {

	int* temp = t->number_array;

	t->number_array = (int*) malloc(new_array_length * sizeof(int));

	copy_to_end(t->number_array, temp, new_array_length, t->array_length);

	t->array_length = new_array_length;

	free(temp);

	return 0;
};



int bn_init_string(bn *t, const char *init_string) {
	int length = strlen(init_string);
	int array_length = ceil((1 + ADDITIONAL_LENGTH) * length);

	t->number_array = (int*) malloc(array_length * sizeof(int));
	t->array_length = array_length;
	t->number_length = length;

	// This thing is not necessery but it leads to clearer code structure
	int* int_init = string_to_int_list(init_string);
	/////

	copy_to_end(t->number_array, int_init, t->array_length, t->number_length);

	free(int_init);

	return 0;
}

//////



/// Arithmetic functions
bn* bn_add(bn const *left, bn const *right) {

	const int left_len = left->array_length, right_len = right->array_length;
	int biggest_len = 0, smallest_len = 0;

	bn* biggest;
	bn* smallest;

	if (left_len > right_len) {
		biggest_len = left_len;
		smallest_len = right_len;
		biggest = left; smallest = right;
	} else {
		biggest_len = right_len;
		smallest_len = left_len;
		biggest = right; smallest = left;
	}
	
	bn* answer = bn_new();

	answer->number_array = (int*) malloc(biggest_len * sizeof(int));

	int left_over = 0;
	int answer_num_length = biggest_len;

	for (int i = 0; i < biggest_len; ++i) {

		if (biggest->number_length < (i+1) && left_over == 0){
			answer_num_length = i;
			break;
		}

		int new_term_add = i < smallest_len ? smallest->number_array[smallest_len-1 - i] : 0;

		int new_term = biggest->number_array[biggest_len-1 - i] + new_term_add + left_over;

		div_t dev_res = div(new_term, 2);

		answer->number_array[biggest_len-1 - i] = dev_res.rem;

		left_over = dev_res.quot;
	}


	if (left_over != 0) {

		int new_array_length = ceil((1+ADDITIONAL_LENGTH)*biggest_len);

		bn_change_size(answer, new_array_length);

		answer->number_array[new_array_length - biggest_len - 1] = left_over;
		answer->array_length = new_array_length;
		answer->number_length = biggest_len + 1;

	} else {
		answer->array_length = biggest_len;
		answer->number_length = answer_num_length;
	}

	return answer;
};

/// !!! In multiplication you have to resize to number_digits_one + number_digits_two


//////
int main() {
	
	bn* num_1 = bn_new();
	bn* num_2 = bn_new();

	char num_str_1[] = "10000000000000000000000000000000000000000";
	char num_str_2[] = "1000000000000000000000";

	bn_init_string(num_1, num_str_1);
	bn_init_string(num_2, num_str_2);

	bn* ans = bn_add(num_1, num_2);
	
	bn_print(ans);

	printf("%d %d \n", ans->array_length, ans->number_length);

	bn_delete(num_1);
	bn_delete(num_2);

	return 0;
}