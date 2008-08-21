/*
 * states.h
 *
 *  Created on: Aug 21, 2008
 *      Author: jakob
 */

#ifndef STATES_H_
#define STATES_H_

#include "macros.h"

class GTerm;

CDECLS_BEGIN

typedef void (*StateFunc)(GTerm* gt);

struct StateOption {
	int							cp;			// codepoint value to look for; -1==end/default
	StateFunc					action;		// action to execute on this transition
	const struct StateOption* 	next_state;	// state to transfer to next
};

extern const StateOption* const state_normal;

CDECLS_END

#endif /* STATES_H_ */
