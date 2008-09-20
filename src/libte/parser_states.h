/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 * Some parts are copyright (c) 1999 by Timothy Miller.
 */

#ifndef PARSER_STATES_H_
#define PARSER_STATES_H_

#include "typedef.h"

typedef void (*StateFunc)(TE* gt);

typedef struct StateOption_ {
	int							cp;			// codepoint value to look for; -1==end/default
	StateFunc					action;		// action to execute on this transition
	const struct StateOption_*	next_state;	// state to transfer to next
} StateOption;

extern const StateOption state_normal[];

#endif /* PARSER_STATES_H_ */
