#ifndef PTY_H_
#define PTY_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _PTY	PTY;

/**
 * Spawns program 'exe' in a new pseudo-terminal
 * fd is returned on success, or -1 on error.
 */
PTY* pty_spawn(const char* exe, const char* const* envdata);
void pty_restore(PTY* pty);

int pty_getfd(PTY* pty);

#ifdef __cplusplus
}
#endif


#endif
