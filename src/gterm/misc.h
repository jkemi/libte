/*
 * misc.h
 *
 *  Created on: Jul 4, 2008
 *      Author: jakob
 */

#ifndef MISC_H_
#define MISC_H_

#include <stdint.h>

typedef unsigned int uint;

static inline int int_min(int a, int b) {return (a < b) ? a : b;}
static inline int int_max(int a, int b) {return (a > b) ? a : b;}
static inline int uint_min(uint a, uint b) {return (a < b) ? a : b;}
static inline int uint_max(uint a, uint b) {return (a > b) ? a : b;}



#endif /* MISC_H_ */
