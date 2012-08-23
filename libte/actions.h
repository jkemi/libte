/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#ifndef ACTIONS_H_
#define ACTIONS_H_

#include "macros.h"

#include "typedef.h"

TE_LOCAL void ac_cr					(TE* te);	// Carriage-return
TE_LOCAL void ac_lf					(TE* te);	// Line-Feed, same as Vertical-Tab and Form-Feed
TE_LOCAL void ac_tab				(TE* te);	// Horizontal tab
TE_LOCAL void ac_bs					(TE* te);	// Backspace
TE_LOCAL void ac_bell				(TE* te);	// Alarm bell
TE_LOCAL void ac_keypad_normal		(TE* te);
TE_LOCAL void ac_keypad_application	(TE* te);
TE_LOCAL void ac_save_cursor		(TE* te);
TE_LOCAL void ac_restore_cursor		(TE* te);
TE_LOCAL void ac_set_tab			(TE* te);	// Horizontal Tabulation Set (HTS)
TE_LOCAL void ac_index_down			(TE* te);	// Index (IND)
TE_LOCAL void ac_next_line			(TE* te);	// Next line (NEL)
TE_LOCAL void ac_index_up			(TE* te);
TE_LOCAL void ac_reset				(TE* te);
TE_LOCAL void ac_cursor_left		(TE* te);	// Cursor Backward P s Times (default = 1) (CUB)
TE_LOCAL void ac_cursor_right		(TE* te);	// Cursor Forward P s Times (default = 1) (CUF)
TE_LOCAL void ac_cursor_up			(TE* te);	// Cursor Up P s Times (default = 1) (CUU)
TE_LOCAL void ac_cursor_down		(TE* te);	// Cursor Down P s Times (default = 1) (CUD)
TE_LOCAL void ac_cursor_position	(TE* te);	// Cursor Position (CUP)
TE_LOCAL void ac_column_position	(TE* te);	// Cursor Character Absolute [column] (default = [row,1]) (CHA)
TE_LOCAL void ac_line_position		(TE* te);	// Line Position Absolute [row] (default = [1,column]) (VPA)
TE_LOCAL void ac_device_attrib		(TE* te);
TE_LOCAL void ac_delete_char		(TE* te);	// Delete P s Character(s) (default = 1) (DCH)
TE_LOCAL void ac_set_mode			(TE* te);	// Set Mode (SM) and DEC Private Mode Set (DECSET)
TE_LOCAL void ac_clear_mode			(TE* te);	// Reset Mode (RM) and DEC Private Mode Reset (DECRST)
TE_LOCAL void ac_set_conformance	(TE* te);	// Set conformance level (DECSCL) and Soft terminal reset (DECSTR)
TE_LOCAL void ac_request_param		(TE* te);
TE_LOCAL void ac_set_margins		(TE* te);	// Set Scrolling Region [top;bottom] (default = full size of window) (DECSTBM)
TE_LOCAL void ac_delete_line		(TE* te);	// Delete P s Line(s) (default = 1) (DL)
TE_LOCAL void ac_status_report		(TE* te);	// Device Status Report (DSR)
TE_LOCAL void ac_erase_display		(TE* te);	// Erase in Display (ED)
TE_LOCAL void ac_erase_line			(TE* te);	// Erase in Line (EL)
TE_LOCAL void ac_insert_line		(TE* te);	// Insert P s Line(s) (default = 1) (IL)
TE_LOCAL void ac_char_attrs			(TE* te);	// Character Attributes (SGR)
TE_LOCAL void ac_clear_tab			(TE* te);	// Tab Clear (TBC)
TE_LOCAL void ac_insert_char		(TE* te);	// Insert P s (Blank) Character(s) (default = 1) (ICH)
TE_LOCAL void ac_screen_align		(TE* te);	// DEC Screen Alignment Test (DECALN)
TE_LOCAL void ac_erase_char			(TE* te);	// Erase P s Character(s) (default = 1) (ECH)
TE_LOCAL void ac_scroll_up			(TE* te);	// Scroll up Ps lines (default = 1) (SU)
TE_LOCAL void ac_scroll_down		(TE* te);	// Scroll up Ps lines (default = 1) (SD)
TE_LOCAL void ac_ls_g0				(TE* te);	// Locking Shift  g0
TE_LOCAL void ac_ls_g1				(TE* te);	// Shift in g1
TE_LOCAL void ac_g0_set_uk			(TE* te);	// Set character set G0 to UK
TE_LOCAL void ac_g0_set_us			(TE* te);	// Set character set G0 to US
TE_LOCAL void ac_g0_set_sg			(TE* te);	// Set character set G0 to DEC special graphics
TE_LOCAL void ac_g1_set_uk			(TE* te);	// Set character set G1 to UK
TE_LOCAL void ac_g1_set_us			(TE* te);	// Set character set G1 to US
TE_LOCAL void ac_g1_set_sg			(TE* te);	// Set character set G1 to DEC special graphics
TE_LOCAL void ac_osc				(TE* te);	// Perform OS control (OSC)

#endif /* ACTIONS_H_ */
