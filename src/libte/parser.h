/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#ifndef PARSER_H_
#define PARSER_H_

#include "typedef.h"

typedef struct Parser_ Parser;

Parser* parser_new (void);
void	parser_delete(Parser* parser);

int		parser_get_param(Parser* parser, int paramno, int default_value);

int32_t	parser_get_intermediate(Parser* parser);

int		parser_get_nparams(Parser* parser);
const int*	parser_get_params(Parser* parser);

void	parser_input(Parser* parser, int len, const int32_t* data, TE* te);

#endif /* PARSER_H_ */
