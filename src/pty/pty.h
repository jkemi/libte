#ifndef PTY_H_
#define PTY_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Spawns program 'exe' in a new pseudo-terminal
 * fd is returned on success, or -1 on error.
 */
int			pty_spawn(const char* exe);
void		pty_restore(void);

#ifdef __cplusplus
}
#endif


#endif
