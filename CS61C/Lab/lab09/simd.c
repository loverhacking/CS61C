#include <time.h>
#include <stdio.h>
#include <x86intrin.h>
#include "simd.h"

long long int sum(int vals[NUM_ELEMS]) {
	clock_t start = clock();

	long long int sum = 0;
	for(unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
		for(unsigned int i = 0; i < NUM_ELEMS; i++) {
			if(vals[i] >= 128) {
				sum += vals[i];
			}
		}
	}
	clock_t end = clock();
	printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
	return sum;
}

long long int sum_unrolled(int vals[NUM_ELEMS]) {
	clock_t start = clock();
	long long int sum = 0;

	for(unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
		for(unsigned int i = 0; i < NUM_ELEMS / 4 * 4; i += 4) {
			if(vals[i] >= 128) sum += vals[i];
			if(vals[i + 1] >= 128) sum += vals[i + 1];
			if(vals[i + 2] >= 128) sum += vals[i + 2];
			if(vals[i + 3] >= 128) sum += vals[i + 3];
		}

		//This is what we call the TAIL CASE
		//For when NUM_ELEMS isn't a multiple of 4
		//NONTRIVIAL FACT: NUM_ELEMS / 4 * 4 is the largest multiple of 4 less than NUM_ELEMS
		for(unsigned int i = NUM_ELEMS / 4 * 4; i < NUM_ELEMS; i++) {
			if (vals[i] >= 128) {
				sum += vals[i];
			}
		}
	}
	clock_t end = clock();
	printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
	return sum;
}

long long int sum_simd(int vals[NUM_ELEMS]) {
	clock_t start = clock();
	__m128i _127 = _mm_set1_epi32(127);		// This is a vector with 127s in it... Why might you need this?
	long long int result = 0;				   // This is where you should put your final result!
	/* DO NOT DO NOT DO NOT DO NOT WRITE ANYTHING ABOVE THIS LINE. */
	
	for(unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
		__m128i _val = _mm_setzero_si128();

		for(unsigned int i = 0; i  < NUM_ELEMS / 4 * 4; i += 4) {
			__m128i _p = _mm_loadu_si128((__m128i*)(vals + i));
			__m128i _cmp_res = _mm_cmpgt_epi32(_p, _127);
			__m128i _res = _mm_and_si128(_cmp_res, _p);
			_val = _mm_add_epi32(_val, _res);
		}

		int sum_arr[4];
		_mm_storeu_si128((__m128i*)sum_arr, _val);
		result += sum_arr[0] + sum_arr[1] + sum_arr[2] + sum_arr[3];

		/* You'll need a tail case. */
		for(unsigned int i = NUM_ELEMS / 4 * 4; i < NUM_ELEMS; i++) {
			if (vals[i] >= 128) {
				result += vals[i];
			}
		}

	}
	clock_t end = clock();
	printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
	return result;
}

long long int sum_simd_unrolled(int vals[NUM_ELEMS]) {
	clock_t start = clock();
	__m128i _127 = _mm_set1_epi32(127);
	long long int result = 0;
	for(unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
		/* COPY AND PASTE YOUR sum_simd() HERE */
		/* MODIFY IT BY UNROLLING IT */
		__m128i _val1 = _mm_setzero_si128();
		__m128i _val2 = _mm_setzero_si128();
		__m128i _val3 = _mm_setzero_si128();
		__m128i _val4 = _mm_setzero_si128();

		for(unsigned int i = 0; i  < NUM_ELEMS / 16 * 16; i += 16) {
			__m128i _p1 = _mm_loadu_si128((__m128i*)(vals + i));
			__m128i _p2 = _mm_loadu_si128((__m128i*)(vals + i + 4));
			__m128i _p3 = _mm_loadu_si128((__m128i*)(vals + i + 8));
			__m128i _p4 = _mm_loadu_si128((__m128i*)(vals + i + 12));

			__m128i _cmp_res1 = _mm_cmpgt_epi32(_p1, _127);
			__m128i _cmp_res2 = _mm_cmpgt_epi32(_p2, _127);
			__m128i _cmp_res3 = _mm_cmpgt_epi32(_p3, _127);
			__m128i _cmp_res4 = _mm_cmpgt_epi32(_p4, _127);

			__m128i _res1 = _mm_and_si128(_cmp_res1, _p1);
			__m128i _res2 = _mm_and_si128(_cmp_res2, _p2);
			__m128i _res3 = _mm_and_si128(_cmp_res3, _p3);
			__m128i _res4 = _mm_and_si128(_cmp_res4, _p4);

			_val1 = _mm_add_epi32(_val1, _res1);
			_val2 = _mm_add_epi32(_val2, _res2);
			_val3 = _mm_add_epi32(_val3, _res3);
			_val4 = _mm_add_epi32(_val4, _res4);
		}

		int sum_arr[4];
		_mm_storeu_si128((__m128i*)sum_arr, _val1);
		result += sum_arr[0] + sum_arr[1] + sum_arr[2] + sum_arr[3];

		_mm_storeu_si128((__m128i*)sum_arr, _val2);
		result += sum_arr[0] + sum_arr[1] + sum_arr[2] + sum_arr[3];

		_mm_storeu_si128((__m128i*)sum_arr, _val3);
		result += sum_arr[0] + sum_arr[1] + sum_arr[2] + sum_arr[3];

		_mm_storeu_si128((__m128i*)sum_arr, _val4);
		result += sum_arr[0] + sum_arr[1] + sum_arr[2] + sum_arr[3];

		/* You'll need 1 or maybe 2 tail cases here. */
		for(unsigned int i = NUM_ELEMS / 16 * 16; i < NUM_ELEMS; i++) {
			if (vals[i] >= 128) {
				result += vals[i];
			}
		}

	}
	clock_t end = clock();
	printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
	return result;
}