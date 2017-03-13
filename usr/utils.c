/*
 * utils.c
 *
 *  Created on: 26.02.2014
 *      Author: alekseym
 */

#include "utils.h"
#include <stdlib.h>
#include <stdint.h>

//find the difference between the 2 moments of time
//start - first moment of time
//end	- second  (end > start)
//return value stored in timespec
struct timespec diff(struct timespec start, struct timespec end)
{
	struct timespec temp;

	//if number of nanoseconds in end less then in start we should take 1 sec
	if ((end.tv_nsec - start.tv_nsec) < 0)
	{
		temp.tv_sec = end.tv_sec - start.tv_sec-1;
		temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
	} else
	{
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

//srand with the good seed
void MySrand()
{
	unsigned int seed;
	seed = (unsigned int)__rdtsc();
	srand(seed);
}

//returns rand in [a , b]
int MyRand(int a, int b)
{
	if(b <= a )
		return b + rand() % (a-b+1);
	else
		return a + rand() % (b-a+1);
}
#define UINT64_CAST(val) (val##ULL)

unsigned long long x; /* The state must be seeded with a nonzero value. */

inline void init_xorshiftstar(struct xorshift64star *xorshift)
{
	xorshift->x = (rand() << 31) + rand();
}
inline unsigned long long generate_xorshiftstar(struct xorshift64star *xorshift)
{
	xorshift->x ^= xorshift->x >> 12; // a
	xorshift->x ^= xorshift->x << 25; // b
	xorshift->x ^= xorshift->x >> 27; // c
	return xorshift->x * UINT64_CAST(2685821657736338717);
}
inline  unsigned long long __rdtsc()
{
//	unsigned long lo, hi;
//		/**
//		 * __asm__ __volatile__ (      // serialize
//		 * "xorl %%eax,%%eax \n        cpuid"
//		 * ::: "%rax", "%rbx", "%rcx", "%rdx");
//		**/
//		/* We cannot use "=A", since this would use %rax on x86_64 */
//	__asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
//	return (unsigned long long)hi << 32 | lo;
	return 0;
}

inline unsigned long long rdtscp(void)
{
//    unsigned long lo, hi;
//    __asm__ __volatile__("rdtscp" : "=a"(lo), "=d"(hi) :: "ecx" );
//    return (unsigned long long)hi << 32 | lo;
	return 0;
}

inline void cpuid( int CPUInfo[4], int InfoType )
{
	asm ("cpuid"
		: "=a" (CPUInfo[0]),
		  "=b" (CPUInfo[1]),
		  "=c" (CPUInfo[2]),
		  "=d" (CPUInfo[3])
		: "a" (InfoType));
}
