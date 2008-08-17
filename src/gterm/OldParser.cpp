/*
 * OldParser.cpp
 *
 *  Created on: Aug 17, 2008
 *      Author: jakob
 */

#include "misc.h"
#include "BufferRow.h"
#include "Buffer.h"

#include "gterm.hpp"

void parser_init (GTerm* gt) {
	gt->current_state = gt->normal_state;
}

void GTerm::process_input(int len, const int32_t* data)
{
	parser.input_remaining = len;
	parser.input_data = data;

	while (parser.input_remaining > 0) {
		const StateOption* state = current_state;
		while (state->cp != -1 && state->cp != *parser.input_data) {
			state++;
		}

		current_state = state->next_state;
		if (state->action) {
			(this->*(state->action))();
		}
		parser.input_data++;
		parser.input_remaining--;
	}

	if (!is_mode_set(DEFERUPDATE) || pending_scroll > scroll_bot-scroll_top) {
		update_changes();
	}
}

void GTerm::set_q_mode()
{
	parser.intermediate_chars[0] = '?';
}

// The verification test used some strange sequence which was
// ^[[61"p
// in a function called set_level,
// but it didn't explain the meaning.  Just in case I ever find out,
// and just so that it doesn't leave garbage on the screen, I accept
// the quote and mark a flag.
void GTerm::set_quote_mode()
{
	parser.intermediate_chars[0] = '"';
}


void GTerm::clear_param()
{
	parser.num_params = 0;
	memset(parser.params, 0, sizeof(parser.params));

	parser.intermediate_chars[0] = 0;
	parser.intermediate_chars[1] = 1;
}

// for performance, this grabs all digits
void GTerm::param_digit()
{
	if (parser.num_params == 0) {
		parser.num_params = 1;
		parser.params[0] = 0;
	}
	parser.params[parser.num_params-1] = parser.params[parser.num_params-1]*10 + (*parser.input_data)-'0';
}

void GTerm::next_param()
{
	parser.num_params++;
	parser.params[parser.num_params-1] = 0;
}

void GTerm::vt52_cursory() {
	const int y = *parser.input_data;
	parser.params[0] = y;
	parser.num_params++;
}

void GTerm::vt52_cursorx()
{
	const int x = *parser.input_data;
	parser.params[1] = x;
	parser.num_params++;

	vt52_cursor();
}


// For efficiency, this grabs all printing characters from buffer, up to
// the end of the line or end of buffer
void GTerm::normal_input()
{
	if (*parser.input_data < 32) {
		return;
	}

	size_t n = 0;
	while (n < parser.input_remaining && parser.input_data[n] > 31) {
		n++;
	}

	size_t nconsumed = input(parser.input_data, n);

	// TODO: why -1 ??
	parser.input_data += nconsumed-1;
	parser.input_remaining -= nconsumed-1;
}

