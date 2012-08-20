/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#ifndef PTY_H_
#define PTY_H_

#include "macros.h"

#ifdef __cplusplus
extern "C" {
#endif

/** opaque handle to encapsulation of pty and child process logic */
typedef struct TE_Pty_	TE_Pty;

TE_EXPORT char** te_pty_env_augment(const char* const* envdata);
TE_EXPORT void te_pty_env_free(char** env);

/**
 * Spawns program 'exe' in a new pseudo-terminal
 * also writes utmp(x) log entry (if possible)
 *
 * \param exe		path to executable to spawn
 * \param args		NULL-terminated array of arguments (including arg0 which will be program name)
 * \param env		NULL-terminated array of pairs of environment variables
 * \param err		optional string pointer to allocate with error message
 *
 * \return NULL on failure	err might be set if not NULL, in which case it should be freed by free(2)
 */
TE_EXPORT TE_Pty* te_pty_spawn(const char* exe, const char* const* args, const char* const* envdata, char** err);

/**
 * Restores resources used by pty_spawn()
 * also removes utmp(x) entry (if possible).
 *
 * \param slave handle
 *
 * \return	>=0	means status code on normal child process exit
 *			<0	means -signum if killed by signal signum
 */
TE_EXPORT int te_pty_restore(TE_Pty* pty);

/**
 * Get file descriptor to use for communication.
 *
 * \param slave handle
 * \return file descriptor
 */
TE_EXPORT int te_pty_getfd(TE_Pty* pty);

// 0 on success
TE_EXPORT int te_pty_set_window_size(int fd, int width, int height);

#ifdef __cplusplus
}
#endif


#endif
