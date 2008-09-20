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

DLLLOCAL void ac_cr					(GTerm* gt);	// Carriage-return
DLLLOCAL void ac_lf					(GTerm* gt);	// Line-Feed, same as Vertical-Tab and Form-Feed
DLLLOCAL void ac_tab				(GTerm* gt);	// Horizontal tab
DLLLOCAL void ac_bs					(GTerm* gt);	// Backspace
DLLLOCAL void ac_bell				(GTerm* gt);	// Alarm bell
DLLLOCAL void ac_keypad_normal		(GTerm* gt);
DLLLOCAL void ac_keypad_application	(GTerm* gt);
DLLLOCAL void ac_save_cursor		(GTerm* gt);
DLLLOCAL void ac_restore_cursor		(GTerm* gt);
DLLLOCAL void ac_set_tab			(GTerm* gt);	// Horizontal Tabulation Set (HTS)
DLLLOCAL void ac_index_down			(GTerm* gt);	// Index (IND)
DLLLOCAL void ac_next_line			(GTerm* gt);	// Next line (NEL)
DLLLOCAL void ac_index_up			(GTerm* gt);
DLLLOCAL void ac_reset				(GTerm* gt);
DLLLOCAL void ac_cursor_left		(GTerm* gt);	// Cursor Backward P s Times (default = 1) (CUB)
DLLLOCAL void ac_cursor_right		(GTerm* gt);	// Cursor Forward P s Times (default = 1) (CUF)
DLLLOCAL void ac_cursor_up			(GTerm* gt);	// Cursor Up P s Times (default = 1) (CUU)
DLLLOCAL void ac_cursor_down		(GTerm* gt);	// Cursor Down P s Times (default = 1) (CUD)
DLLLOCAL void ac_cursor_position	(GTerm* gt);	// Cursor Position (CUP)
DLLLOCAL void ac_column_position	(GTerm* gt);	// Cursor Character Absolute [column] (default = [row,1]) (CHA)
DLLLOCAL void ac_line_position		(GTerm* gt);	// Line Position Absolute [row] (default = [1,column]) (VPA)
DLLLOCAL void ac_device_attrib		(GTerm* gt);
DLLLOCAL void ac_delete_char		(GTerm* gt);	// Delete P s Character(s) (default = 1) (DCH)
DLLLOCAL void ac_set_mode			(GTerm* gt);	// Set Mode (SM) and DEC Private Mode Set (DECSET)
DLLLOCAL void ac_clear_mode			(GTerm* gt);	// Reset Mode (RM) and DEC Private Mode Reset (DECRST)
DLLLOCAL void ac_set_conformance	(GTerm* gt);	// Set conformance level (DECSCL) and Soft terminal reset (DECSTR)
DLLLOCAL void ac_request_param		(GTerm* gt);
DLLLOCAL void ac_set_margins		(GTerm* gt);	// Set Scrolling Region [top;bottom] (default = full size of window) (DECSTBM)
DLLLOCAL void ac_delete_line		(GTerm* gt);	// Delete P s Line(s) (default = 1) (DL)
DLLLOCAL void ac_status_report		(GTerm* gt);	// Device Status Report (DSR)
DLLLOCAL void ac_erase_display		(GTerm* gt);	// Erase in Display (ED)
DLLLOCAL void ac_erase_line			(GTerm* gt);	// Erase in Line (EL)
DLLLOCAL void ac_insert_line		(GTerm* gt);	// Insert P s Line(s) (default = 1) (IL)
DLLLOCAL void ac_char_attrs			(GTerm* gt);	// Character Attributes (SGR)
DLLLOCAL void ac_clear_tab			(GTerm* gt);	// Tab Clear (TBC)
DLLLOCAL void ac_insert_char		(GTerm* gt);	// Insert P s (Blank) Character(s) (default = 1) (ICH)
DLLLOCAL void ac_screen_align		(GTerm* gt);	// DEC Screen Alignment Test (DECALN)
DLLLOCAL void ac_erase_char			(GTerm* gt);	// Erase P s Character(s) (default = 1) (ECH)
DLLLOCAL void ac_scroll_up			(GTerm* gt);	// Scroll up Ps lines (default = 1) (SU)
DLLLOCAL void ac_scroll_down		(GTerm* gt);	// Scroll up Ps lines (default = 1) (SD)
DLLLOCAL void ac_vt52_cursor		(GTerm* gt);
DLLLOCAL void ac_vt52_ident			(GTerm* gt);

#endif /* ACTIONS_H_ */
