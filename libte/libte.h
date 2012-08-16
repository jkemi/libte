/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */


#ifndef LIBTE_H_
#define LIBTE_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifndef NDEBUG
#	include <stdio.h>
#endif

#include "symbol.h"
#include "macros.h"

#include "config.h"
#ifdef TE_ENABLE_PTY
#	include "pty.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// compile time header version
#define TE_HEADER_VERSION_MAJOR	0
#define TE_HEADER_VERSION_MINOR	8
#define TE_HEADER_VERSION_FIX	0
#define TE_HEADER_VERSION		"0.8.0"
	
typedef struct TE_Backend_	TE_Backend;
typedef struct TE_Frontend_	TE_Frontend;

/// Specifies what clipboard to act on.
typedef enum _te_selection {
	TE_CLIPBUF_PRIMARY		= (1<<0),	///< "selected" data in X11 (may be same as clipboard on other systems)
	TE_CLIPBUF_CLIPBOARD	= (1<<1),	///< clipboard data
} te_clipbuf_t;

/**
 * This struct defines the callbacks made from terminal backend to frontend
 */
struct TE_Frontend_ {
	// Drawing methods
	void (*draw_text)	(void* priv, int x, int y, const symbol_t* symbols, int len);
	void (*draw_clear)	(void* priv, int x, int y, const symbol_color_t bg_color, int len);
	void (*draw_cursor)	(void* priv, symbol_color_t fg_color, symbol_color_t bg_color, symbol_attributes_t attrs, int x, int y, int32_t cp);
	void (*draw_move)	(void* priv, int y, int height, int byoffset);

	void (*updated) 	(void* priv);
	void (*reset)		(void* priv);
	void (*bell) 		(void* priv);
	void (*title) 		(void* priv, const int32_t* text, int len);
	void (*send_back)	(void* priv, const int32_t* data, int len);
	
	/** terminal application requested a new size, honor by resizing frontend window, signalling pty, and call te_resize() */
	void (*request_resize) (void* priv, int width, int height);
	void (*position) (void* priv, int offset, int size);
	
	/**
	 * Called when terminal application requests copy to clipboard.
	 *
	 * \param priv		frontend private user data
	 * \param clipbuf	requested clipboard
	 * \param text		data to copy to clipboard
	 * \param len		size of data to copy
	 */
	void (*set_clipboard) (void* priv, te_clipbuf_t clipbuf, const int32_t* text, int len);

	/**
	 * Called when terminal application requests paste from clipboard.
	 * Once clipboard data is used, request_clipboard_done() will be called.
	 *
	 * \param priv		frontend private user data
	 * \param clipbuf	requested clipboard
	 * \param text		should be set to pointer to clipboard data
	 * \param size		should be set to size of clipboard data
	 * \return token used with request_clipboard_done() callback
	 */
	void* (*request_clipboard) (void* priv, te_clipbuf_t clipbuf, int32_t* const* text, int* size);
	
	/**
	 * Called when data from request_clipboard() is no longer needed.
	 * Should free any data allocated by request_clipboard() callback.
	 *
	 * \param priv	frontend private user data
	 * \param token	token returned from request_clipboard() callback
	 */
	void (*request_clipboard_done) (void* priv, void* token);
};

typedef enum _te_key {
	TE_KEY_UNDEFINED	= 0,

	TE_KEY_ENTER		= 1,
	TE_KEY_TAB			= 2,
	TE_KEY_ESCAPE		= 3,
	TE_KEY_BACKSPACE	= 4,

	TE_KEY_HOME			= 5,
	TE_KEY_INSERT		= 6,
	TE_KEY_DELETE		= 7,
	TE_KEY_END			= 8,
	TE_KEY_PGUP			= 9,
	TE_KEY_PGDN			= 10,

	TE_KEY_UP			= 11,
	TE_KEY_DOWN			= 12,
	TE_KEY_RIGHT		= 13,
	TE_KEY_LEFT			= 14,

	TE_KEY_F			= 20,
	TE_KEY_F1			= 21,
	TE_KEY_F2			= 22,
	TE_KEY_F3			= 23,
	TE_KEY_F4			= 24,
	TE_KEY_F5			= 25,
	TE_KEY_F6			= 26,
	TE_KEY_F7			= 27,
	TE_KEY_F8			= 28,
	TE_KEY_F9			= 29,
	TE_KEY_F10			= 30,
	TE_KEY_F11			= 31,
	TE_KEY_F12			= 32,
	TE_KEY_F13			= 33,
	TE_KEY_F14			= 34,
	TE_KEY_F15			= 35,
	TE_KEY_F16			= 36,
	TE_KEY_F17			= 37,
	TE_KEY_F18			= 38,
	TE_KEY_F19			= 39,
	TE_KEY_F20			= 40,

	TE_KP_EQUAL			= 50,
	TE_KP_DIVIDE		= 51,
	TE_KP_MULTIPLY		= 52,
	TE_KP_SUBSTRACT		= 53,
	TE_KP_ADD			= 54,
	TE_KP_PERIOD		= 55,
	TE_KP_COMMA			= 56,
	TE_KP_ENTER			= 57,

	TE_KP_0		= 58,
	TE_KP_1		= 59,
	TE_KP_2		= 60,
	TE_KP_3		= 61,
	TE_KP_4		= 62,
	TE_KP_5		= 63,
	TE_KP_6		= 64,
	TE_KP_7		= 65,
	TE_KP_8		= 66,
	TE_KP_9		= 67,
} te_key_t;

typedef enum _te_modifier {
	TE_MOD_NONE		= 0,
	TE_MOD_SHIFT	= (1<<0),
	TE_MOD_CTRL		= (1<<1),
	TE_MOD_META		= (1<<2),
} te_modifier_t;

typedef enum _te_mouse_button {
	TE_MOUSE_NONE		= 0,
	TE_MOUSE_LEFT		= (1<<0),
	TE_MOUSE_MIDDLE		= (1<<1),
	TE_MOUSE_RIGHT		= (1<<2),
	TE_MOUSE_WHEEL_UP	= (1<<3),
	TE_MOUSE_WHEEL_DOWN	= (1<<4),
	TE_MOUSE_DOUBLE		= (1<<5),	// Double-click
	TE_MOUSE_TRIPLE		= (1<<6),	// Triple-click (or more)
} te_mouse_button_t;

DLLEXPORT extern const char*	te_binary_version_string;
DLLEXPORT extern const int		te_binary_version[3];

DLLEXPORT TE_Backend* te_create(const TE_Frontend* front, void* user, int width, int height, const void* options, size_t options_size);
DLLEXPORT void te_destroy(TE_Backend* te);

/**
 * Resize terminal
 */
DLLEXPORT void te_resize(TE_Backend* te, int width, int height);

/**
 * Returns current width of terminal
 */
DLLEXPORT int te_get_width(TE_Backend* te);

/**
 * Returns current height of terminal
 */
DLLEXPORT int te_get_height(TE_Backend* te);

/**
 * Sent from GUI to terminal to request a redraw.
 * This might trigger multiple calls to DrawText etc..
 */
DLLEXPORT void te_request_redraw(TE_Backend* te, int x, int y, int w, int h, int force);

/**
 * Send data from client here
 */
DLLEXPORT void te_process_input(TE_Backend* te, const int32_t* data, size_t len);

/**
 * Handles host special key presses
 * @returns non-zero if handled
 */
DLLEXPORT int te_handle_button(TE_Backend* te, te_key_t key);

/**
 * Handles ordinary letters/numbers key presses
 */
DLLEXPORT void te_handle_keypress(TE_Backend* te, int32_t cp, te_modifier_t modifiers);

DLLEXPORT void te_handle_mouse(TE_Backend* te, int mouse_x, int mouse_y, te_mouse_button_t mouse_buttons, te_modifier_t modifiers);

DLLEXPORT void te_paste_text(TE_Backend* te, const int32_t* data, size_t len);

DLLEXPORT void te_position(TE_Backend* te, int offset);

DLLEXPORT void te_lock_scroll(TE_Backend* te, int scroll_lock);

#ifndef NDEBUG
DLLEXPORT void te_debug(TE_Backend* te, FILE* where);
#endif

#ifdef __cplusplus
}
#endif

#endif
