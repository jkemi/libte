#include "libte/libte.h"

class TE {
private:
	TE_Backend*	_te;
public:

	TE() {_te = NULL;}
	TE(const TE&);	// Intentionally not defined.
	virtual ~TE() {
		if (_te != NULL) {
			te_destroy(_te);
		}
	}


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

	/**
	 * Handles mouse input
	 */
	void teHandleMouse(int mouse_x, int mouse_y, te_mouse_button_t mouse_buttons, te_modifier_t modifiers) {
		te_handle_mouse(_te, mouse_x, mouse_y, mouse_buttons, modifiers);
	}

	/**
	 * Handles pasting of text
	 */
	void tePasteText(const int32_t* data, size_t len) {
		te_paste_text(_te, data, len);
	}

	/**
	 * Handles setting of scroll-back position
	 */
	void tePosition(int offset) {
		te_position(_te, offset);
	}

	/**
	 * Handles setting of scroll-lock
	 */
	void teLockScroll(bool scroll_lock) {
		te_lock_scroll(_te, scroll_lock);
	}

#ifndef NDEBUG
	/**
	 * Dumps internal debug output from terminal implementation.
	 */
	void teDebug(FILE* where) {
		te_debug(_te, where);
	}
#endif

protected:
	virtual void fe_draw_text(int x, int y, const symbol_t* symbols, int len) = 0;
	virtual void fe_draw_clear(int x, int y, const symbol_color_t bg_color, int len) = 0;
	virtual void fe_draw_cursor(symbol_color_t fg_color, symbol_color_t bg_color, symbol_attributes_t attrs, int x, int y, int32_t cp) = 0;
	virtual void fe_draw_move(int y, int height, int byoffset) = 0;
	virtual void fe_updated() = 0;
	virtual void fe_reset() = 0;
	virtual void fe_bell() = 0;
	virtual void fe_title(const int32_t* text) = 0;
	virtual void fe_send_back(const int32_t* data, int len) = 0;
	virtual void fe_request_resize(int width, int height) = 0;
	virtual void fe_position(int offset, int size) = 0;

private:
	static void _impl_draw_text (void* priv, int x, int y, const symbol_t* symbols, int len) {
		((TE*)priv)->fe_draw_text(x, y, symbols, len);
	}
	static void _impl_draw_clear (void* priv, int x, int y, const symbol_color_t bg_color, int len) {
		((TE*)priv)->fe_draw_clear(x, y, bg_color, len);
	}
	static void _impl_draw_cursor (void* priv, symbol_color_t fg_color, symbol_color_t bg_color, symbol_attributes_t attrs, int x, int y, int32_t cp) {
		((TE*)priv)->fe_draw_cursor(fg_color, bg_color, attrs, x, y, cp);
	}
	static void _impl_draw_move (void* priv, int y, int height, int byoffset) {
		((TE*)priv)->fe_draw_move(y, height, byoffset);
	}
	static void _impl_updated (void* priv) {
		((TE*)priv)->fe_updated();
	}
	static void _impl_reset (void* priv) {
		((TE*)priv)->fe_reset();
	}
	static void _impl_bell (void* priv) {
		((TE*)priv)->fe_bell();
	}
	static void _impl_title (void* priv, const int32_t* text) {
		((TE*)priv)->fe_title(text);
	}
	static void _impl_send_back (void* priv, const int32_t* data, int len) {
		((TE*)priv)->fe_send_back(data, len);
	}
	static void _impl_request_resize (void* priv, int width, int height) {
		((TE*)priv)->fe_request_resize(width, height);
	}
	static void _impl_position (void* priv, int offset, int size) {
		((TE*)priv)->fe_position(offset, size);
	}

};
