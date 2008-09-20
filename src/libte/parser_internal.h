/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#ifndef PARSER_INTERNAL_H_
#define PARSER_INTERNAL_H_

#include "typedef.h"

void _parser_unknown_esc(TE* te);
void _parser_unknown_csi(TE* te);

void _parser_osc_start(TE* te);
void _parser_osc_put(TE* te);
void _parser_osc_end(TE* te);

void _parser_dcs_start(TE* te);
void _parser_dcs_put(TE* te);
void _parser_dcs_end(TE* te);

void _parser_set_intermediate(TE* te);

void _parser_clear_param(TE* te);
void _parser_param_digit(TE* te);
void _parser_next_param(TE* te);
void _parser_vt52_cursory(TE* te);
void _parser_vt52_cursorx(TE* te);
void _parser_normal_input(TE* te);

#endif /* PARSER_INTERNAL_H_ */
