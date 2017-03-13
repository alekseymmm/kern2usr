/*
 * utils.h
 *
 *  Created on: 26.02.2014
 *      Author: alekseym
 */

#ifndef UTILS_H_
#define UTILS_H_

#include "time.h"

struct xorshift64star
{
	unsigned long long x;
};
inline void init_xorshiftstar(struct xorshift64star *xorshift);

inline unsigned long long generate_xorshiftstar(struct xorshift64star *xorshift);

struct timespec diff(struct timespec start, struct timespec end);

void MySrand();

int MyRand(int a, int b);

inline  unsigned long long __rdtsc();

inline unsigned long long rdtscp(void);

inline void cpuid( int CPUInfo[4], int InfoType );


#endif /* UTILS_H_ */
