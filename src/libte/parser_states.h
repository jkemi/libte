/*
 * states.h
 *
 *  Created on: Aug 21, 2008
 *      Author: jakob
 */

#ifndef PARSER_STATES_H_
#define PARSER_STATES_H_

#include "gt_typedef.h"

typedef void (*StateFunc)(GTerm* gt);

typedef struct StateOption_ {
	int							cp;			// codepoint value to look for; -1==end/default
	StateFunc					action;		// action to execute on this transition
	const struct StateOption_*	next_state;	// state to transfer to next
} StateOption;

extern const StateOption state_normal[];

#endif /* PARSER_STATES_H_ */
