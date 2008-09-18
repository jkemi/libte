/*
 * OldParser.hpp
 *
 *  Created on: Aug 17, 2008
 *      Author: jakob
 */

#ifndef OLDPARSER_HPP_
#define OLDPARSER_HPP_

class GTerm;

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

#endif /* OLDPARSER_HPP_ */
