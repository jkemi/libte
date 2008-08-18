/*
 * OldParser.cpp
 *
 *  Created on: Aug 17, 2008
 *      Author: jakob
 */

#include "misc.h"
#include "BufferRow.h"
#include "Buffer.h"

#include "actions.hpp"

#include "gterm.hpp"

// This is found int states.cpp
extern const StateOption* state_normal;

void parser_init (GTerm* gt) {
	gt->parser.current_state = state_normal;
}

void GTerm::process_input(int len, const int32_t* data)
{
	parser.input_remaining = len;
	parser.input_data = data;

	while (parser.input_remaining > 0) {
		const StateOption* state = parser.current_state;
		while (state->cp != -1 && state->cp != *parser.input_data) {
			state++;
		}

		parser.current_state = state->next_state;
		if (state->action != NULL) {
			state->action(this);
		}
		parser.input_data++;
		parser.input_remaining--;
	}

	if (!is_mode_set(SMOOTHSCROLL) || pending_scroll > scroll_bot-scroll_top) {
		update_changes();
	}
}

void _parser_osc_start(GTerm* gt) {

}

void _parser_osc_put(GTerm* gt) {

}

void _parser_osc_end(GTerm* gt) {

}

void _parser_dcs_start(GTerm* gt) {

}

void _parser_dcs_put(GTerm* gt) {

}

void _parser_dcs_end(GTerm* gt) {

}

void _parser_set_q_mode(GTerm* gt)
{
	gt->parser.intermediate_chars[0] = '?';
}

// The verification test used some strange sequence which was
// ^[[61"p
// in a function called set_level,
// but it didn't explain the meaning.  Just in case I ever find out,
// and just so that it doesn't leave garbage on the screen, I accept
// the quote and mark a flag.
void _parser_set_quote_mode(GTerm* gt)
{
	gt->parser.intermediate_chars[0] = '"';
}


void _parser_clear_param(GTerm* gt)
{
	gt->parser.num_params = 0;
	memset(gt->parser.params, 0, sizeof(gt->parser.params));

	gt->parser.intermediate_chars[0] = 0;
	gt->parser.intermediate_chars[1] = 1;
}

// for performance, this grabs all digits
void _parser_param_digit(GTerm* gt)
{
	if (gt->parser.num_params == 0) {
		gt->parser.num_params = 1;
		gt->parser.params[0] = 0;
	}
	gt->parser.params[gt->parser.num_params-1] = gt->parser.params[gt->parser.num_params-1]*10 + (*gt->parser.input_data)-'0';
}

void _parser_next_param(GTerm* gt)
{
	gt->parser.num_params++;
	gt->parser.params[gt->parser.num_params-1] = 0;
}

// For efficiency, this grabs all printing characters from buffer, up to
// the end of the line or end of buffer
void _parser_normal_input(GTerm* gt)
{
	if (*gt->parser.input_data < 32) {
		return;
	}

	size_t n = 0;
	while (n < gt->parser.input_remaining) {
		const int32_t cp = gt->parser.input_data[n];

		// we can't munch control characters or defined 8-bit controls
		if (cp < 32 || (cp >= 0x84 && cp <= 0x9f)) {
			break;
		}
		n++;
	}

	gt->input(gt->parser.input_data, n);

	// TODO: why -1 ??
	gt->parser.input_data += n-1;
	gt->parser.input_remaining -= n-1;
}

