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
	
	int32_t*	osc_buffer;
	size_t		osc_size;
	size_t		osc_capacity;

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

size_t parser_get_osc_size(Parser* parser) {
	return parser->osc_size;
}

const int32_t* parser_get_osc_data(Parser* parser) {
	return parser->osc_buffer;
}

void parser_input(Parser* parser, int len, const int32_t* data, TE* te)
{
	parser->input_remaining = len;
	parser->input_data = data;

	while (parser->input_remaining > 0) {
		// Look for next state
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

			state->action(te);
		}

		parser->current_state = state->next_state;
		parser->input_data++;
		parser->input_remaining--;
	}
}

void _parser_unknown_esc(TE* te) {
	int32_t cp = *te->parser->input_data;
	if (cp >= 32 && cp <= 126) {
		WARNF("unknown esc dispatch: '%c'\n", cp);
	} else {
		WARNF("unknown esc dispatch: 0x%x\n", cp);
	}
}

void _parser_unknown_csi(TE* te) {
	int32_t cp = *te->parser->input_data;
	if (cp >= 32 && cp <= 126) {
		WARNF("unknown csi dispatch: '%c'\n", cp);
	} else {
		WARNF("unknown csi dispatch: 0x%x\n", cp);
	}
}

void _parser_osc_start(TE* te) {
	te->parser->osc_buffer = malloc(32 * sizeof(int32_t));
	te->parser->osc_capacity = 32;
	te->parser->osc_size = 0;
}

// TODO: for performance, grab all available chars until ST BEL etc?
void _parser_osc_put(TE* te) {
	if (te->parser->osc_size == te->parser->osc_capacity) {
		te->parser->osc_capacity *= 2;
		te->parser->osc_buffer = realloc(te->parser->osc_buffer, te->parser->osc_capacity*sizeof(int32_t));
	}
	
	te->parser->osc_buffer[te->parser->osc_size++] = *te->parser->input_data;
}

void _parser_osc_end(TE* te) {
#if 0
	char* tmp = malloc(te->parser->osc_size+1);
	for (uint i=0; i<te->parser->osc_size; i++) {
		tmp[i] = te->parser->osc_buffer[i];
//		printf("got osc put: '%d' '%c'\n", i, tmp[i]);
	}
	tmp[te->parser->osc_size] = '\0';
	DEBUGF("got osc data: '%s'\n", tmp);
	free(tmp);
#endif
	ac_osc(te);
	
	free(te->parser->osc_buffer);
	te->parser->osc_capacity = 0;
	te->parser->osc_size = 0;
	te->parser->osc_buffer = NULL;
}

void _parser_dcs_start(TE* te) {

}

void _parser_dcs_put(TE* te) {

}

void _parser_dcs_end(TE* te) {

}

void _parser_set_intermediate(TE* te) {
	te->parser->intermediate_chars[0] = *te->parser->input_data;
}

void _parser_clear_param(TE* te)
{
	te->parser->num_params = 0;
	memset(te->parser->params, 0, sizeof(te->parser->params));

	te->parser->intermediate_chars[0] = 0;
	te->parser->intermediate_chars[1] = 1;
}

// TODO: for performance, grab all available digits?
void _parser_param_digit(TE* te)
{
	if (te->parser->num_params == 0) {
		te->parser->num_params = 1;
		te->parser->params[0] = 0;
	}
	te->parser->params[te->parser->num_params-1] = te->parser->params[te->parser->num_params-1]*10 + (*te->parser->input_data)-'0';
}

void _parser_next_param(TE* te)
{
	// TODO: check so that number of params (16) don't overflow..
	te->parser->num_params++;
	te->parser->params[te->parser->num_params-1] = 0;
}

// For efficiency, this grabs all printing characters from buffer, up to
// the end of the line or end of buffer
void _parser_normal_input(TE* te)
{
	if (*te->parser->input_data < 32) {
		DEBUGF("%d is unhandled..., ignoring\n", *te->parser->input_data);
		return;
	}

	size_t n;
	for (n = 0; n < te->parser->input_remaining; n++) {
		const int32_t cp = te->parser->input_data[n];

		// we can't munch control characters or defined 8-bit controls
		if (cp < 32 || (cp >= 0x84 && cp <= 0x9f)) {
			break;
		}
	}

	be_input(te, te->parser->input_data, n);

	// Only advance the number of extra characters consumed by this operation
	// One character is always consumed by parser_input().
	te->parser->input_data += n-1;
	te->parser->input_remaining -= n-1;
}

