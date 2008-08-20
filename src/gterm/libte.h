#ifndef LIBTE_H_
#define LIBTE_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "buffersymbol.h"

#include "macros.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _TE_Backend	TE_Backend;
typedef struct _TE_Frontend	TE_Frontend;

/**
 * This struct defines the callbacks made from terminal backend to frontend
 */
struct _TE_Frontend {
	void (*draw)		(void* priv, int x, int y, const symbol_t* data, int len);
	void (*clear)		(void* priv, int x, int y, const symbol_color_t bg_color, int len);
	void (*draw_cursor)	(void* priv, symbol_color_t fg_color, symbol_color_t bg_color, symbol_attributes_t attrs, int x, int y, int32_t cp);
	void (*move)		(void* priv, int y, int height, int byoffset);
	void (*resized) 	(void* priv, int width, int height);
	void (*updated) 	(void* priv);
	void (*reset)		(void* priv);
	void (*bell) 		(void* priv);
	void (*mouse) 		(void* priv, int x, int y);
	void (*title) 		(void* priv, const int32_t* text);
	void (*send_back)	(void* priv, const int32_t* data);
	void (*request_resize) (void* priv, int width, int height);
};

typedef enum _te_key {
	TE_KEY_UNDEFINED	= 0,

	TE_KEY_RETURN,
	TE_KEY_TAB,
	TE_KEY_ESCAPE,
	TE_KEY_BACKSPACE,

	TE_KEY_HOME,
	TE_KEY_INSERT,
	TE_KEY_DELETE,
	TE_KEY_END,
	TE_KEY_PGUP,
	TE_KEY_PGDN,

	TE_KEY_UP,
	TE_KEY_DOWN,
	TE_KEY_RIGHT,
	TE_KEY_LEFT,
} te_key_t;

typedef enum _te_modifier {
	TE_MOD_NONE = 0,
	TE_MOD_SHIFT = (1<<0),
	TE_MOD_CTRL	= (1<<1),
	TE_MOD_META	= (1<<2),
} te_modifier_t;

DLLEXPORT TE_Backend* te_create(const TE_Frontend* front, void* priv, int width, int height);
DLLEXPORT void		te_destroy(TE_Backend* te);

/**
 * Resize terminal
 */
DLLEXPORT void te_resize(TE_Backend* te, int width, int height);

/**
 * Returns current width of terminal
 */
DLLEXPORT int	 te_get_width(TE_Backend* te);

/**
 * Returns current height of terminal
 */
DLLEXPORT int	 te_get_height(TE_Backend* te);

/**
 * Sent from GUI to terminal to request a redraw.
 * This might trigger multiple calls to DrawText etc..
 */
DLLEXPORT void te_reqest_redraw(TE_Backend* te, int x, int y, int w, int h, bool force);

/**
 * Send data from client here
 */
DLLEXPORT void te_process_input(TE_Backend* te, const int32_t* data, size_t len);

DLLEXPORT void te_process_input_mbs(TE_Backend* te, const char* data, size_t len);

/**
 * Handles host special key presses
 */
DLLEXPORT void te_handle_button(TE_Backend* te, te_key_t key);

/**
 * Handles ordinary letters/numbers key presses
 */
DLLEXPORT void te_handle_keypress(TE_Backend* te, int32_t cp, te_modifier_t modifiers);

/**
 * TODO: document me
 */
DLLEXPORT void te_update(TE_Backend* te);


#ifdef __cplusplus
}
#endif

#endif
