// Copyright Timothy Miller, 1999

#include "misc.h"
#include "Buffer.h"
#include "Dirty.h"

#include "gterm.hpp"



void GTerm::Update()
{
	update_changes();
}

void GTerm::ProcessInput(int len, const int32_t* data)
{
	input_remaining = len;
	input_data = data;

	while (input_remaining > 0) {
		const StateOption* state = current_state;
		while (state->byte != -1 && state->byte != *input_data) {
			state++;
		}

		// TODO: is the below comment a request, a fact or what?
		// action must be allowed to redirect state change
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

void GTerm::Reset()
{
	reset();
}

void GTerm::ExposeArea(int x, int y, int w, int h)
{
	for (int i=0; i<h; i++) {
		changed_line(i+y, x, x+w);
	}
	if (!is_mode_set(DEFERUPDATE)) {
		update_changes();
	}
}

void GTerm::ResizeTerminal(int w, int h)
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

GTerm::GTerm(int w, int h) : width(w), height(h)
{
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
}

GTerm::~GTerm()
{
	delete buffer;
	delete dirty;
}

/* End of File */
