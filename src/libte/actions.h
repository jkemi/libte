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

DLLLOCAL void ac_cr					(TE* gt);	// Carriage-return
DLLLOCAL void ac_lf					(TE* gt);	// Line-Feed, same as Vertical-Tab and Form-Feed
DLLLOCAL void ac_tab				(TE* gt);	// Horizontal tab
DLLLOCAL void ac_bs					(TE* gt);	// Backspace
DLLLOCAL void ac_bell				(TE* gt);	// Alarm bell
DLLLOCAL void ac_keypad_normal		(TE* gt);
DLLLOCAL void ac_keypad_application	(TE* gt);
DLLLOCAL void ac_save_cursor		(TE* gt);
DLLLOCAL void ac_restore_cursor		(TE* gt);
DLLLOCAL void ac_set_tab			(TE* gt);	// Horizontal Tabulation Set (HTS)
DLLLOCAL void ac_index_down			(TE* gt);	// Index (IND)
DLLLOCAL void ac_next_line			(TE* gt);	// Next line (NEL)
DLLLOCAL void ac_index_up			(TE* gt);
DLLLOCAL void ac_reset				(TE* gt);
DLLLOCAL void ac_cursor_left		(TE* gt);	// Cursor Backward P s Times (default = 1) (CUB)
DLLLOCAL void ac_cursor_right		(TE* gt);	// Cursor Forward P s Times (default = 1) (CUF)
DLLLOCAL void ac_cursor_up			(TE* gt);	// Cursor Up P s Times (default = 1) (CUU)
DLLLOCAL void ac_cursor_down		(TE* gt);	// Cursor Down P s Times (default = 1) (CUD)
DLLLOCAL void ac_cursor_position	(TE* gt);	// Cursor Position (CUP)
DLLLOCAL void ac_column_position	(TE* gt);	// Cursor Character Absolute [column] (default = [row,1]) (CHA)
DLLLOCAL void ac_line_position		(TE* gt);	// Line Position Absolute [row] (default = [1,column]) (VPA)
DLLLOCAL void ac_device_attrib		(TE* gt);
DLLLOCAL void ac_delete_char		(TE* gt);	// Delete P s Character(s) (default = 1) (DCH)
DLLLOCAL void ac_set_mode			(TE* gt);	// Set Mode (SM) and DEC Private Mode Set (DECSET)
DLLLOCAL void ac_clear_mode			(TE* gt);	// Reset Mode (RM) and DEC Private Mode Reset (DECRST)
DLLLOCAL void ac_set_conformance	(TE* gt);	// Set conformance level (DECSCL) and Soft terminal reset (DECSTR)
DLLLOCAL void ac_request_param		(TE* gt);
DLLLOCAL void ac_set_margins		(TE* gt);	// Set Scrolling Region [top;bottom] (default = full size of window) (DECSTBM)
DLLLOCAL void ac_delete_line		(TE* gt);	// Delete P s Line(s) (default = 1) (DL)
DLLLOCAL void ac_status_report		(TE* gt);	// Device Status Report (DSR)
DLLLOCAL void ac_erase_display		(TE* gt);	// Erase in Display (ED)
DLLLOCAL void ac_erase_line			(TE* gt);	// Erase in Line (EL)
DLLLOCAL void ac_insert_line		(TE* gt);	// Insert P s Line(s) (default = 1) (IL)
DLLLOCAL void ac_char_attrs			(TE* gt);	// Character Attributes (SGR)
DLLLOCAL void ac_clear_tab			(TE* gt);	// Tab Clear (TBC)
DLLLOCAL void ac_insert_char		(TE* gt);	// Insert P s (Blank) Character(s) (default = 1) (ICH)
DLLLOCAL void ac_screen_align		(TE* gt);	// DEC Screen Alignment Test (DECALN)
DLLLOCAL void ac_erase_char			(TE* gt);	// Erase P s Character(s) (default = 1) (ECH)
DLLLOCAL void ac_scroll_up			(TE* gt);	// Scroll up Ps lines (default = 1) (SU)
DLLLOCAL void ac_scroll_down		(TE* gt);	// Scroll up Ps lines (default = 1) (SD)
DLLLOCAL void ac_vt52_cursor		(TE* gt);
DLLLOCAL void ac_vt52_ident			(TE* gt);

#endif /* ACTIONS_H_ */
