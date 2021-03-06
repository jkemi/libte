#ifndef TE_LIBTE_HPP_
#define TE_LIBTE_HPP_

/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#include "libte.h"

/**
 * C++ wrapper of libte
 */
class LibTE {
private:
	TE_Backend*	_te;
public:

	static const char*		binary_version_string() {return te_binary_version_string;}

	/**
	 * \return {major,minor,fix}
	 */
	static const int*		binary_version() {return te_binary_version;}


	LibTE() {_te = NULL;}
	LibTE(const LibTE&);	// Intentionally not defined.
	virtual ~LibTE() {
		if (_te != NULL) {
			te_destroy(_te);
		}
	}

	/**
	 * Initializes an instance of libte
	 */
	bool teInit(int width, int height, void* options, size_t options_size) {
		const static TE_Frontend _callbacks = {
				&_impl_draw_text,
				&_impl_draw_clear,
				&_impl_draw_cursor,
				&_impl_draw_move,

				&_impl_updated,
				&_impl_reset,
				&_impl_bell,
				&_impl_title,
				&_impl_send_back,
				&_impl_request_resize,
				&_impl_position,
				&_impl_palette,

				&_impl_set_clipboard,
				&_impl_request_clipboard,
				&_impl_request_clipboard_done
		};

		if (_te != NULL) {
			te_destroy(_te);
		}
		_te = te_create(&_callbacks, this, width, height, options, options_size);
		return _te != NULL;
	}
	
	/**
	 * Get palette
	 */
	const te_color_t* teGetPalette() {
		return te_get_palette(_te);
	}

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
	int teHandleButton(te_key_t key, te_modifier_t modifiers) {
		return te_handle_button(_te, key, modifiers);
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
	virtual void fe_draw_cursor(int x, int y, symbol_t symbol) = 0;
	virtual void fe_draw_move(int y, int height, int byoffset) = 0;
	virtual void fe_updated() = 0;
	virtual void fe_reset() = 0;
	virtual void fe_bell() = 0;
	virtual void fe_title(const int32_t* text, int len) = 0;
	virtual void fe_send_back(const int32_t* data, int len) = 0;
	virtual void fe_request_resize(int width, int height) = 0;
	virtual void fe_position(int offset, int size) = 0;
	virtual void fe_palette(int offset, int count, const te_color_t* data) = 0;
	virtual void fe_set_clipboard (te_clipbuf_t clipbuf, const int32_t* text, int len) = 0;
	virtual void* fe_request_clipboard (te_clipbuf_t clipbuf, int32_t* const* text, int* size) = 0;
	virtual void fe_request_clipboard_done (void* token) = 0;


private:
	static void _impl_draw_text (void* priv, int x, int y, const symbol_t* symbols, int len) {
		((LibTE*)priv)->fe_draw_text(x, y, symbols, len);
	}
	static void _impl_draw_clear (void* priv, int x, int y, const symbol_color_t bg_color, int len) {
		((LibTE*)priv)->fe_draw_clear(x, y, bg_color, len);
	}
	static void _impl_draw_cursor (void* priv, int x, int y, symbol_t symbol) {
		((LibTE*)priv)->fe_draw_cursor(x, y, symbol);
	}
	static void _impl_draw_move (void* priv, int y, int height, int byoffset) {
		((LibTE*)priv)->fe_draw_move(y, height, byoffset);
	}
	static void _impl_updated (void* priv) {
		((LibTE*)priv)->fe_updated();
	}
	static void _impl_reset (void* priv) {
		((LibTE*)priv)->fe_reset();
	}
	static void _impl_bell (void* priv) {
		((LibTE*)priv)->fe_bell();
	}
	static void _impl_title (void* priv, const int32_t* text, int len) {
		((LibTE*)priv)->fe_title(text, len);
	}
	static void _impl_send_back (void* priv, const int32_t* data, int len) {
		((LibTE*)priv)->fe_send_back(data, len);
	}
	static void _impl_request_resize (void* priv, int width, int height) {
		((LibTE*)priv)->fe_request_resize(width, height);
	}
	static void _impl_position (void* priv, int offset, int size) {
		((LibTE*)priv)->fe_position(offset, size);
	}
	static void _impl_palette (void* priv, int offset, int count, const te_color_t* data) {
		((LibTE*)priv)->fe_palette(offset, count, data);
	}
	static void _impl_set_clipboard (void* priv, te_clipbuf_t clipbuf, const int32_t* text, int len) {
		((LibTE*)priv)->fe_set_clipboard(clipbuf, text, len);
	}
	static void* _impl_request_clipboard (void* priv, te_clipbuf_t clipbuf, int32_t* const* text, int* size) {
		return ((LibTE*)priv)->fe_request_clipboard(clipbuf, text, size);
	}
	static void _impl_request_clipboard_done (void* priv, void* token) {
		((LibTE*)priv)->fe_request_clipboard_done(token);
	}
};

#endif // TE_LIBTE_HPP_
