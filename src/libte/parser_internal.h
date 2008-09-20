/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#ifndef PARSER_INTERNAL_H_
#define PARSER_INTERNAL_H_

#include "typedef.h"

void _parser_unknown_esc(GTerm* gt);
void _parser_unknown_csi(GTerm* gt);

void _parser_osc_start(GTerm* gt);
void _parser_osc_put(GTerm* gt);
void _parser_osc_end(GTerm* gt);

void _parser_dcs_start(GTerm* gt);
void _parser_dcs_put(GTerm* gt);
void _parser_dcs_end(GTerm* gt);

void _parser_set_intermediate(GTerm* gt);

void _parser_clear_param(GTerm* gt);
void _parser_param_digit(GTerm* gt);
void _parser_next_param(GTerm* gt);
void _parser_vt52_cursory(GTerm* gt);
void _parser_vt52_cursorx(GTerm* gt);
void _parser_normal_input(GTerm* gt);

#endif /* PARSER_INTERNAL_H_ */
