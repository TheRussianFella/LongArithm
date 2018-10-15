#include <stdio.h>


int main() {

	typedef unsigned long long ull;

	enum en{MAXSIZE = 1000000, BITS = sizeof(ull)*8};

	ull m[MAXSIZE / BITS];

	int insert(unsigned el) {
		m[el / BITS] |= 1 << (el%BITS);
		return 0;
	}

	int remove(unsigned el) {
		m[el / BITS] &=  ~(1 << (el%BITS)); return 0;
	}

	int find(unsigned el) {
		return (m[el / BITS] >> (el % BITS)) & 01; 
	}
}
