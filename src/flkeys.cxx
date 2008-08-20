// This is basically Timothy Miller's original X-key mapping, adjusted to work with the
// key values returned from fltk-1. Works, more or less, as of Dec. 2006.
// Original code copyright Timothy Miller, 1999

#include <FL/Enumerations.H>

#include "flkeys.h"

/* FLTK key codes are...
FL_Escape, FL_BackSpace, FL_Tab, FL_Enter, FL_Print, FL_Scroll_Lock,
FL_Pause, FL_Insert, FL_Home, FL_Page_Up, FL_Delete, FL_End, FL_Page_Down,
FL_Left, FL_Up, FL_Right, FL_Down, FL_Shift_L, FL_Shift_R, FL_Control_L, FL_Control_R,
FL_Caps_Lock, FL_Alt_L, FL_Alt_R, FL_Meta_L, FL_Meta_R, FL_Menu, FL_Num_Lock, FL_KP_Enter
*/

#define Key_Table_Void 0

static const keyseq _keypadkeys[] = {
	{(FL_KP + '/'),	"/"},
	{(FL_KP + '*'), "*"},
	{(FL_KP + '-'), "-"},
	{FL_Home, 	"\033[\000"},
	{(FL_KP + '7'), "7"},
	{(FL_KP + '8'),	"8"},
	{FL_Page_Up,	"\033[5~"},
	{(FL_KP + '9'),	"9"},
	{(FL_KP + '+'),	"+"},
	{(FL_KP + '4'),	"4"},
	{FL_Home,	"\033[s"},
	{(FL_KP + '5'),	"5"},
	{(FL_KP + '6'),	"6"},
	{FL_End,	"\033[e"},
	{(FL_KP + '1'),	"1"},
	{(FL_KP + '2'),	"2"},
	{FL_Page_Down, "\033[6~"},
	{(FL_KP + '3'),	"3"},
	{FL_Insert,	"\033[2~"},
	{(FL_KP + '0'),	"0"},
	{FL_Delete,	"\033[3~"},
	{(FL_KP + '.'),	"."},
	{FL_KP_Enter,	"\r"},
	{Key_Table_Void, 0}};

#if 0
static const keyseq _keypadappkeys[] = {
	{XK_KP_Divide, 	"\033Oo"},
	{XK_KP_Multiply, "\033Oj"},
	{XK_KP_Subtract, "\033Om"},
	{XK_KP_Home, 	"\033[\000"},
	{XK_KP_7, 	"\033Ow"},
	{XK_KP_8,	"\033Ox"},
	{FL_Page_Up,	"\033[5~"},
	{XK_KP_9,	"\033Oy"},
	{XK_KP_Add,	"\033Ok"},
	{XK_KP_4,	"\033Ot"},
	{FL_Home,	"\033[s"},
	{XK_KP_5,	"\033Ou"},
	{XK_KP_6,	"\033Ov"},
	{XK_KP_End,	"\033[e"},
	{XK_KP_1,	"\033Oq"},
	{XK_KP_2,	"\033Or"},
	{FL_Page_Down, "\033[6~"},
	{XK_KP_3,	"\033Os"},
	{FL_Insert,	"\033[2~"},
	{XK_KP_0,	"\033Op"},
	{FL_Delete,	"\033[3~"},
	{XK_KP_Decimal,	"\033On"},
	{XK_KP_Enter,	"\033OM"},
	{Key_Table_Void, 0}};
#endif

static const keyseq _otherkeys[] = {
	{FL_Home,      "\033[\000"},
	{FL_Page_Up,   "\033[5~"},
	{FL_Page_Down, "\033[6~"},
	{FL_End,       "\033[e"},
	{FL_Insert,    "\033[2~"},
	{(FL_F + 1),	"\033[11~"},
	{(FL_F + 2),	"\033[12~"},
	{(FL_F + 3),	"\033[13~"},
	{(FL_F + 4),	"\033[14~"},
	{(FL_F + 5),	"\033[15~"},
	{(FL_F + 6),	"\033[17~"},
	{(FL_F + 7),	"\033[18~"},
	{(FL_F + 8),	"\033[19~"},
	{(FL_F + 9),	"\033[20~"},
	{(FL_F + 10),	"\033[21~"},
	{(FL_F + 11),	"\033[23~"},
	{(FL_F + 12),	"\033[24~"},
	{Key_Table_Void, 0}};

const keyseq* keypadkeys = _keypadkeys;
//const keyseq* keypadappkeys = _keypadappkeys;
const keyseq* otherkeys = _otherkeys;

const char* find_key(int keysym, const keyseq* table)
{
	for (; table->keysym != Key_Table_Void; table++) {
		if (table->keysym == keysym) {
			return table->str;
		}
	}
	return 0;
}

/* End of File */
