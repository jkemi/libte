// Copyright Timothy Miller, 1999

#include <stdlib.h>

#include "misc.h"
#include "Buffer.h"
#include "Dirty.h"

#include "../strutil.h"

#include "gterm.hpp"

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

void GTerm::handle_button(te_key_t key)
{
switch (key) {
case TE_KEY_RETURN: {
	if(GetMode() & GTerm::NEWLINE) {
		send_back("\r\n");	// send CRLF if GTerm::NEWLINE is set
	} else {
		send_back("\r");	// ^M (CR)
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
	set_mode_flag(GTerm::NEWLINE);
	clear_mode_flag(GTerm::TEXTONLY);  // disable "Text Only" mode
	clear_mode_flag(GTerm::LOCALECHO);  // disable "Text Only" mode
}

GTerm::~GTerm()
{
	delete buffer;
	delete dirty;
}

void GTerm::send_back(const char* data) {
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
	return te->gt->Width();
}

int te_get_height(TE_Backend* te) {
	return te->gt->Height();
}

void te_reqest_redraw(TE_Backend* te, int x, int y, int w, int h, bool force) {
	te->gt->RequestRedraw(x, y, w, h, force);
}

void te_process_input(TE_Backend* te, const int32_t* data, size_t len) {
	te->gt->process_input(len, data);
}

void te_handle_button(TE_Backend* te, te_key_t key) {
	te->gt->handle_button(key);
}

void te_update(TE_Backend* te) {
	te->gt->update_changes();
}
