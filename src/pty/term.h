/*
 * term.h
 *
 *  Created on: Aug 26, 2008
 *      Author: jakob
 */

#ifndef TERM_H_
#define TERM_H_


#ifdef __cplusplus
extern "C" {
#endif

void term_set_window_size(int fd, int width, int height);
void term_set_utf8(int fd, int utf8);

#ifdef __cplusplus
}
#endif

#endif /* TERM_H_ */
