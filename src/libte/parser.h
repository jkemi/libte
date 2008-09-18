/*
 * parser.h
 *
 *  Created on: Sep 19, 2008
 *      Author: jakob
 */

#ifndef PARSER_H_
#define PARSER_H_

#include "gt_typedef.h"

typedef struct Parser_ Parser;

Parser* parser_new (void);
void	parser_delete(Parser* parser);

int		parser_get_param(Parser* parser, int paramno, int default_value);

int32_t	parser_get_intermediate(Parser* parser);

int		parser_get_nparams(Parser* parser);
const int*	parser_get_params(Parser* parser);

void	parser_input(Parser* parser, int len, const int32_t* data, GTerm* gt);

#endif /* PARSER_H_ */
