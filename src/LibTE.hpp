#include "libte/libte.h"

class TE {
private:
	TE_Backend*	_te;
public:
	TE() {_te = NULL;}
	TE(const TE&);	// Intentionally not defined.
	virtual ~TE();

	void teInit(int width, int height);

	/**
	 * Resize terminal
	 */
	void teResize(int width, int height) {
		te_resize(_te, width, height);
	}

	/**
	 * Returns current width of terminal
	 */
	int teGetWidth() {
		return te_get_width(_te);
	}

	/**
	 * Returns current height of terminal
	 */
	int teGetHeight() {
		return te_get_height(_te);
	}

	/**
	 * Sent from GUI to terminal to request a redraw.
	 * This might trigger multiple calls to DrawText etc..
	 */
	void teRequestRedraw(int x, int y, int w, int h, bool force) {
		te_request_redraw(_te, x, y, w, h, force);
	}

	/**
	 * Send data from client here
	 */
	void teProcessInput(const int32_t* data, size_t len) {
		te_process_input(_te, data, len);
	}

	/**
	 * Handles host special key presses
	 * @returns non-zero if handled
	 */
	int teHandleButton(te_key_t key) {
		return te_handle_button(_te, key);
	}

	/**
	 * Handles ordinary letters/numbers key presses
	 */
	void teHandleKeypress(int32_t cp, te_modifier_t modifiers) {
		te_handle_keypress(_te, cp, modifiers);
	}

	void tePasteText(const int32_t* data, size_t len) {
		te_paste_text(_te, data, len);
	}

	void tePosition(int offset) {
		te_position(_te, offset);
	}

	void teLockScroll(bool scroll_lock) {
		te_lock_scroll(_te, scroll_lock);
	}

#ifndef NDEBUG
	void teDebug(FILE* where) {
		te_debug(_te, where);
	}
#endif

	virtual void fe_draw_text(int x, int y, const symbol_t* symbols, int len) = 0;
	virtual void fe_draw_clear(int x, int y, const symbol_color_t bg_color, int len) = 0;
	virtual void fe_draw_cursor(symbol_color_t fg_color, symbol_color_t bg_color, symbol_attributes_t attrs, int x, int y, int32_t cp) = 0;
	virtual void fe_draw_move(int y, int height, int byoffset) = 0;

	virtual void fe_updated() = 0;
	virtual void fe_reset() = 0;
	virtual void fe_bell() = 0;
	virtual void fe_mouse(int x, int y) = 0;
	virtual void fe_title(const int32_t* text) = 0;
	virtual void fe_send_back(const int32_t* data) = 0;
	virtual void fe_request_resize(int width, int height) = 0;
	virtual void fe_position(int offset, int size) = 0;
};
