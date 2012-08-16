#ifndef TEXTRENDERER_HPP_
#define TEXTRENDERER_HPP_

#include "libte/libte.h"

#ifdef __cplusplus
extern "C" {
#endif

void	tr_init(const uint8_t* palette);
void	tr_term(void);

// Returns character width in pixels
int		tr_width(void);

// Returns character height in pixels
int		tr_height(void);

// Returns rendered symbol as RGB data
const uint8_t*	tr_get(symbol_t sym);

#ifdef __cplusplus
}
#endif

#endif /* TEXTRENDERER_HPP_ */
