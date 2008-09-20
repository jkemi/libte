/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#ifndef PARSER_INTERNAL_H_
#define PARSER_INTERNAL_H_

#include "typedef.h"

void _parser_unknown_esc(TE* gt);
void _parser_unknown_csi(TE* gt);

void _parser_osc_start(TE* gt);
void _parser_osc_put(TE* gt);
void _parser_osc_end(TE* gt);

void _parser_dcs_start(TE* gt);
void _parser_dcs_put(TE* gt);
void _parser_dcs_end(TE* gt);

void _parser_set_intermediate(TE* gt);

void _parser_clear_param(TE* gt);
void _parser_param_digit(TE* gt);
void _parser_next_param(TE* gt);
void _parser_vt52_cursory(TE* gt);
void _parser_vt52_cursorx(TE* gt);
void _parser_normal_input(TE* gt);

#endif /* PARSER_INTERNAL_H_ */
