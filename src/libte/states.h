/*
 * states.h
 *
 *  Created on: Aug 21, 2008
 *      Author: jakob
 */

#ifndef STATES_H_
#define STATES_H_

#include "macros.h"

#include "gt_typedef.h"

CDECLS_BEGIN

typedef void (*StateFunc)(GTerm* gt);

typedef struct StateOption_ {
	int							cp;			// codepoint value to look for; -1==end/default
	StateFunc					action;		// action to execute on this transition
	const struct StateOption_*	next_state;	// state to transfer to next
} StateOption;

extern const StateOption state_normal[];

CDECLS_END

#endif /* STATES_H_ */
