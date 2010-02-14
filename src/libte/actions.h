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

DLLLOCAL void ac_cr					(TE* te);	// Carriage-return
DLLLOCAL void ac_lf					(TE* te);	// Line-Feed, same as Vertical-Tab and Form-Feed
DLLLOCAL void ac_tab				(TE* te);	// Horizontal tab
DLLLOCAL void ac_bs					(TE* te);	// Backspace
DLLLOCAL void ac_bell				(TE* te);	// Alarm bell
DLLLOCAL void ac_keypad_normal		(TE* te);
DLLLOCAL void ac_keypad_application	(TE* te);
DLLLOCAL void ac_save_cursor		(TE* te);
DLLLOCAL void ac_restore_cursor		(TE* te);
DLLLOCAL void ac_set_tab			(TE* te);	// Horizontal Tabulation Set (HTS)
DLLLOCAL void ac_index_down			(TE* te);	// Index (IND)
DLLLOCAL void ac_next_line			(TE* te);	// Next line (NEL)
DLLLOCAL void ac_index_up			(TE* te);
DLLLOCAL void ac_reset				(TE* te);
DLLLOCAL void ac_cursor_left		(TE* te);	// Cursor Backward P s Times (default = 1) (CUB)
DLLLOCAL void ac_cursor_right		(TE* te);	// Cursor Forward P s Times (default = 1) (CUF)
DLLLOCAL void ac_cursor_up			(TE* te);	// Cursor Up P s Times (default = 1) (CUU)
DLLLOCAL void ac_cursor_down		(TE* te);	// Cursor Down P s Times (default = 1) (CUD)
DLLLOCAL void ac_cursor_position	(TE* te);	// Cursor Position (CUP)
DLLLOCAL void ac_column_position	(TE* te);	// Cursor Character Absolute [column] (default = [row,1]) (CHA)
DLLLOCAL void ac_line_position		(TE* te);	// Line Position Absolute [row] (default = [1,column]) (VPA)
DLLLOCAL void ac_device_attrib		(TE* te);
DLLLOCAL void ac_delete_char		(TE* te);	// Delete P s Character(s) (default = 1) (DCH)
DLLLOCAL void ac_set_mode			(TE* te);	// Set Mode (SM) and DEC Private Mode Set (DECSET)
DLLLOCAL void ac_clear_mode			(TE* te);	// Reset Mode (RM) and DEC Private Mode Reset (DECRST)
DLLLOCAL void ac_set_conformance	(TE* te);	// Set conformance level (DECSCL) and Soft terminal reset (DECSTR)
DLLLOCAL void ac_request_param		(TE* te);
DLLLOCAL void ac_set_margins		(TE* te);	// Set Scrolling Region [top;bottom] (default = full size of window) (DECSTBM)
DLLLOCAL void ac_delete_line		(TE* te);	// Delete P s Line(s) (default = 1) (DL)
DLLLOCAL void ac_status_report		(TE* te);	// Device Status Report (DSR)
DLLLOCAL void ac_erase_display		(TE* te);	// Erase in Display (ED)
DLLLOCAL void ac_erase_line			(TE* te);	// Erase in Line (EL)
DLLLOCAL void ac_insert_line		(TE* te);	// Insert P s Line(s) (default = 1) (IL)
DLLLOCAL void ac_char_attrs			(TE* te);	// Character Attributes (SGR)
DLLLOCAL void ac_clear_tab			(TE* te);	// Tab Clear (TBC)
DLLLOCAL void ac_insert_char		(TE* te);	// Insert P s (Blank) Character(s) (default = 1) (ICH)
DLLLOCAL void ac_screen_align		(TE* te);	// DEC Screen Alignment Test (DECALN)
DLLLOCAL void ac_erase_char			(TE* te);	// Erase P s Character(s) (default = 1) (ECH)
DLLLOCAL void ac_scroll_up			(TE* te);	// Scroll up Ps lines (default = 1) (SU)
DLLLOCAL void ac_scroll_down		(TE* te);	// Scroll up Ps lines (default = 1) (SD)
DLLLOCAL void ac_g0_set_uk			(TE* te);	// Set character set G0 to UK
DLLLOCAL void ac_g0_set_us			(TE* te);	// Set character set G0 to US
DLLLOCAL void ac_g0_set_sg			(TE* te);	// Set character set G0 to DEC special graphics
DLLLOCAL void ac_g1_set_uk			(TE* te);	// Set character set G1 to UK
DLLLOCAL void ac_g1_set_us			(TE* te);	// Set character set G1 to US
DLLLOCAL void ac_g1_set_sg			(TE* te);	// Set character set G1 to DEC special graphics


#endif /* ACTIONS_H_ */
