// Copyright Timothy Miller, 1999

#include <stdlib.h>

#include "misc.h"
#include "Buffer.h"
#include "Dirty.h"

#include "../strutil.h"

#include "gterm.hpp"

static void _vt_debug(const char* label, unsigned char final, vtparse_t* vt) {
	printf("%s", label);
	printf("final: 0x%02x", final);
	if (final >= 32) {
		printf(" ('%c')", final);
	}
	printf(" intermediate: 0x%02x, 0x%02x params:", vt->intermediate_chars[0], vt->intermediate_chars[1]);
	for (int i = 0; i < vt->num_params; i++) {
		printf(" 0x%02x", vt->params[i]);
	}
	printf (" (%d)\n", vt->num_params);
}


static void _vt_print(GTerm* gt, unsigned char c) {
	//printf("%c", c);
	gt->normal_input2(c);
}

// Single-character functions
static void _vt_execute(GTerm* gt, unsigned char c) {
	switch (c) {
	case '\a':	gt->bell();	break;	// BEL	bell (CTRL-G)
	case '\b':	gt->bs();	break;	// BS	backspace (CTRL-H)
	case '\r':	gt->cr();	break;	// CR	carriage return (CTRL-M)
	case 5:					break;	// ENQ	terminal status (CTRL-E)
	case '\f':	gt->ff();	break;	// FF	form feed (CTRL-L)
	case '\n':	gt->lf();	break;	// LF	line feed (CTRL-J)
	case 14:				break;	// SO	shift out (CTRL-N)
	case ' ':	_vt_print(gt, ' ');			break;	// SP	space
	case '\t':	gt->tab();	break;	// TAG	horizontal tab (CTRL-I)
	case '\v':	gt->lf();	break;	// VT	vertical tab (CTRL-K)
	case 15:				break;	// SI	shift in (CTRL-O)
	}
}

// Controls beginning with ESC
static void _vt_esc_dispatch(GTerm* gt, unsigned char c) {

	switch (gt->parser.intermediate_chars[0]) {
	case 0:
		break;

	case ' ':
	case '#':
	case '%':
	case '(':
	case ')':
	case '*':
	case '+':
//		_vt_debug("ignoring intermediate esc dispatch: ", c, &gt->parser);
		return;
	default:
		_vt_debug("unexpected intermediate: ", c, &gt->parser);
	}

	switch (c) {
/*
	// VT100 and up
	case '0':	// DEC Special Character and Line Drawing Set
	case 'A':	// United Kingdom (UK)
	case 'B':	// United States (USASCII)

	// VT220 and up
	case '4':	// Dutch
	case 'C':	// Finnish
	case '5':	// Finnish
	case 'R':	// French
	case 'Q':	// French Canadian
	case 'K':	// German
	case 'Y':	// Italian
	case 'E':	// Norwegian/Danish
	case '6':	// Norwegian/Danish
	case 'Z':	// Spanish
	case 'H':	// Swedish
	case '7':	// Swedish
	case '=':	// Swiss
		printf("character set request (charset: '%c', request: '%c')\n", c, gt->parser.intermediate_chars[0]);
		break;
*/

	case 'H':	gt->set_tab();	break;		// Tab Set ( HTS is 0x88)
	case '=':	gt->keypad_application();	break;	// Application Keypad (DECPAM)
	case '>':	gt->keypad_normal();	break;		// Normal Keypad (DECPNM)
	case 'c':	gt->reset(); break;					// Full Reset (RIS)
	default:
		_vt_debug("unknown esc dispatch: ", c, &gt->parser);
	}
}

static void _vt_csi_dispatch(GTerm* gt, unsigned char c) {
	switch (c) {
	case 'A':	gt->cursor_up();	break;			// Cursor Up P s Times (default = 1) (CUU)
	case 'C':	gt->cursor_right();	break;			// Cursor Forward P s Times (default = 1) (CUF)
	case 'G':	gt->column_position(); break;		// Cursor Character Absolute [column] (default = [row,1]) (CHA)
	case 'H':	gt->cursor_position(); break;		// Cursor Position [row;column] (default = [1,1]) (CUP)
	case 'J':	gt->erase_display();	break;		// Erase in Display (ED)
	case 'K':	gt->erase_line();	break;			// Erase in Line (EL)
	case 'P':	gt->delete_char();	break;			// Delete P s Character(s) (default = 1) (DCH)
	case 'X':	gt->erase_char();	break;			// Erase P s Character(s) (default = 1) (ECH)
	case 'd':	gt->line_position(); break; 		// Line Position Absolute [row] (default = [1,column]) (VPA)
	case 'h':	gt->set_mode();		break;			// Set Mode (SM)
	case 'l':	gt->clear_mode();	break;			// Reset Mode (RM)
	case 'm':	gt->char_attrs();	break;			// Character Attributes (SGR)
	case 'p':	gt->reset();		break;			// Soft terminal reset (DECSTR)
	case 'r':	gt->set_margins();	break;			// Set Scrolling Region [top;bottom] (default = full size of window) (DECSTBM)
	case 's':	gt->save_cursor();	break;			// Save cursor (ANSI.SYS)
	default:
		_vt_debug("unknown csi dispatch: ", c, &gt->parser);
		//printf ("unknown csi dispatch: 0x%02x\n", c);
	}
}

static void _vt_parser_cb(struct vtparse* parser, vtparse_action_t action, unsigned char c) {
	GTerm* gt = (GTerm*)parser->user_data;

	switch (action) {
	case VTPARSE_ACTION_PRINT:
		_vt_print(gt, c);
		break;
	case VTPARSE_ACTION_EXECUTE:
		_vt_execute(gt, c);
		break;

	// Escape functions
	case VTPARSE_ACTION_ESC_DISPATCH:
		_vt_esc_dispatch(gt, c);
		break;

	// Control functions
	case VTPARSE_ACTION_CSI_DISPATCH:
		_vt_csi_dispatch(gt, c);
		break;

	// Protocol hook support
	case VTPARSE_ACTION_HOOK:
		printf("ignoring all hooks (request 0x%02x)\n", c);
		break;
	case VTPARSE_ACTION_PUT:
	case VTPARSE_ACTION_UNHOOK:

	// Operating system control
	case VTPARSE_ACTION_OSC_START:
		_vt_debug("osc start (ignored): ", c, parser);
		break;
	case VTPARSE_ACTION_OSC_PUT:
		_vt_debug("osc put (ignored): ", c, parser);
		break;
	case VTPARSE_ACTION_OSC_END:
		_vt_debug("osc end (ignored): ", c, parser);
		break;
	default:
		break;
	}

//	if (!gt->is_mode_set(gt->DEFERUPDATE) || gt->pending_scroll > gt->scroll_bot-gt->scroll_top) {
		gt->update_changes();
	//}
}

/*
void GTerm::process_input(int len, const int32_t* data)
{
	input_remaining = len;
	input_data = data;

	while (input_remaining > 0) {
		const StateOption* state = current_state;
		while (state->cp != -1 && state->cp != *input_data) {
			state++;
		}

		current_state = state->next_state;
		if (state->action) {
			(this->*(state->action))();
		}
		input_data++;
		input_remaining--;
	}

	if (!is_mode_set(DEFERUPDATE) || pending_scroll > scroll_bot-scroll_top) {
		update_changes();
	}
}
*/

void GTerm::handle_button(te_key_t key)
{
switch (key) {
case TE_KEY_RETURN: {
	if (is_mode_flag(GTerm::NEWLINE)) {
		fe_send_back("\r\n");	// send CRLF if GTerm::NEWLINE is set
	} else {
		fe_send_back("\r");	// ^M (CR)
	}
}
};
}


void GTerm::resize_terminal(int w, int h)
{
	bool* newtabs = new bool[w];
	if (w > width) {
		memset(newtabs+width, 0, sizeof(bool)*(width-w));
	}
	memcpy(newtabs, tab_stops, sizeof(bool)*int_min(width,w));
	tab_stops = newtabs;

	clear_area(int_min(width,w), 0, int_max(width,w)-1, h-1);
	clear_area(0, int_min(height,h), w-1, int_min(height,h)-1);

	width = w;
	height = h;
	scroll_bot = height-1;
	if (scroll_top >= height) {
		scroll_top = 0;
	}
	int cx = int_min(width-1, cursor_x);
	int cy = int_min(height-1, cursor_y);
	move_cursor(cx, cy);

	buffer->reshape(h, w);
	dirty->reshape(h, w);
}

GTerm::GTerm(const TE_Frontend* fe, void* fe_priv, int w, int h)
{
	_fe = fe;
	_fe_priv = fe_priv;

	vtparse_init(&parser, &_vt_parser_cb);
	parser.user_data = this;


	width = w;
	height = h;

	doing_update = false;

	buffer = new Buffer(h, w);
	dirty = new Dirty(h, w);

	tab_stops = new bool[w];
	memset(tab_stops, 0, sizeof(bool)*w);

	cursor_x = 0;
	cursor_y = 0;
	save_x = 0;
	save_y = 0;
	mode_flags = 0;
	reset();

	fg_color = 7;
	bg_color = 0;

	set_mode_flag(GTerm::NOEOLWRAP);   // disable line wrapping
	clear_mode_flag(GTerm::TEXTONLY);  // disable "Text Only" mode
	clear_mode_flag(GTerm::LOCALECHO);  // disable "Text Only" mode
}

GTerm::~GTerm()
{
	delete buffer;
	delete dirty;
}

void GTerm::fe_send_back(const char* data) {
	// TODO: speedup ?!
	size_t len = str_mbslen(data);
	int32_t buf[len+1];

	size_t nwritten;
	str_mbs_to_cps_n(buf, data, len, strlen(data), &nwritten, NULL);

	buf[nwritten] = L'\0';

	str_mbs_hexdump("mbs: ", data, strlen(data));
	str_cps_hexdump("cps: ", buf, nwritten);

	_fe->send_back(_fe_priv, buf);
}

void GTerm::fe_request_resize(int width, int height) {
	_fe->request_resize(_fe_priv, width, height);
}

void GTerm::fe_updated(void) {
	_fe->updated(_fe_priv);
}

void GTerm::fe_scroll(int y, int height, int offset) {
	_fe->scroll(_fe_priv, y, height, offset);
}

//
// Internal structure
//

struct _TE_Backend {
	GTerm*	gt;
};

//
// Public API below
//

TE_Backend* te_create(const TE_Frontend* front, void* priv, int width, int height) {
	TE_Backend* te = (TE_Backend*)malloc(sizeof(TE_Backend));
	if (te != NULL) {
		te->gt = new GTerm(front, priv, width, height);
	}
	return te;
}

void te_destroy(TE_Backend* te) {
	delete te->gt;
	free(te);
}

void te_resize(TE_Backend* te, int width, int height) {
	te->gt->resize_terminal(width, height);
}

int te_get_width(TE_Backend* te) {
	return te->gt->get_width();
}

int te_get_height(TE_Backend* te) {
	return te->gt->get_height();
}

void te_reqest_redraw(TE_Backend* te, int x, int y, int w, int h, bool force) {
	te->gt->request_redraw(x, y, w, h, force);
}

void te_process_input(TE_Backend* te, const int32_t* data, size_t len) {
	//te->gt->process_input(len, data);
}

void te_process_input_mbs(TE_Backend* te, const char* data, size_t len) {
	vtparse(&te->gt->parser, (const unsigned char*)data, len);
}

void te_handle_button(TE_Backend* te, te_key_t key) {
	te->gt->handle_button(key);
}

void te_update(TE_Backend* te) {
	te->gt->update_changes();
}
