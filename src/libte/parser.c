/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#include <stdlib.h>
#include "misc.h"
#include "bufferrow.h"
#include "buffer.h"

#include "actions.h"

#include "internal.h"

#include "parser_states.h"
#include "parser_internal.h"

#include "parser.h"


struct Parser_ {
	// action parameters
	int num_params;
	int params[16];

	unsigned char intermediate_chars[2];

	const int32_t*	input_data;
	size_t			input_remaining;

	const StateOption* current_state;
};


Parser* parser_new (void) {
	Parser* parser = xnew(Parser, 1);
	parser->current_state = state_normal;

	return parser;
}

void parser_delete(Parser* parser) {
	free(parser);
}

int parser_get_param(Parser* parser, int paramno, int default_value) {
	if (parser->num_params > paramno) {
		return parser->params[paramno];
	} else {
		return default_value;
	}
}

int32_t	parser_get_intermediate(Parser* parser) {
	return parser->intermediate_chars[0];
}

int parser_get_nparams(Parser* parser) {
	return parser->num_params;
}

const int* parser_get_params(Parser* parser) {
	return parser->params;
}

void parser_input(Parser* parser, int len, const int32_t* data, GTerm* gt)
{
	parser->input_remaining = len;
	parser->input_data = data;

	while (parser->input_remaining > 0) {
		const StateOption* state = parser->current_state;
		while (state->cp != -1 && state->cp != *parser->input_data) {
			state++;
		}

		if (state->action != NULL) {
/*			int32_t cp = *parser.input_data;
			printf("performing state action for input: ");
			if (cp >= 32 && cp <= 126) {
				printf("'%c'\n", cp);
			} else {
				printf("0x%x\n", cp);
			}*/

			state->action(gt);
		}

		parser->current_state = state->next_state;
		parser->input_data++;
		parser->input_remaining--;
	}
}

void _parser_unknown_esc(GTerm* gt) {
	int32_t cp = *gt->parser->input_data;
	printf("unknown esc dispatch: ");
	if (cp >= 32 && cp <= 126) {
		printf("'%c'\n", cp);
	} else {
		printf("0x%x\n", cp);
	}
}

void _parser_unknown_csi(GTerm* gt) {
	int32_t cp = *gt->parser->input_data;
	printf("unknown csi dispatch: ");
	if (cp >= 32 && cp <= 126) {
		printf("'%c'\n", cp);
	} else {
		printf("0x%x\n", cp);
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

void _parser_set_intermediate(GTerm* gt) {
	gt->parser->intermediate_chars[0] = *gt->parser->input_data;
}

void _parser_clear_param(GTerm* gt)
{
	gt->parser->num_params = 0;
	memset(gt->parser->params, 0, sizeof(gt->parser->params));

	gt->parser->intermediate_chars[0] = 0;
	gt->parser->intermediate_chars[1] = 1;
}

// for performance, this grabs all digits
void _parser_param_digit(GTerm* gt)
{
	if (gt->parser->num_params == 0) {
		gt->parser->num_params = 1;
		gt->parser->params[0] = 0;
	}
	gt->parser->params[gt->parser->num_params-1] = gt->parser->params[gt->parser->num_params-1]*10 + (*gt->parser->input_data)-'0';
}

void _parser_next_param(GTerm* gt)
{
	gt->parser->num_params++;
	gt->parser->params[gt->parser->num_params-1] = 0;
}

// For efficiency, this grabs all printing characters from buffer, up to
// the end of the line or end of buffer
void _parser_normal_input(GTerm* gt)
{
	if (*gt->parser->input_data < 32) {
		return;
	}

	size_t n;
	for (n = 0; n < gt->parser->input_remaining; n++) {
		const int32_t cp = gt->parser->input_data[n];

		// we can't munch control characters or defined 8-bit controls
		if (cp < 32 || (cp >= 0x84 && cp <= 0x9f)) {
			break;
		}
	}

	gt_input(gt, gt->parser->input_data, n);

	// Only advance the number of extra characters consumed by this operation
	// One character is always consumed by process_input(), hence n-1 here
	gt->parser->input_data += n-1;
	gt->parser->input_remaining -= n-1;
}

