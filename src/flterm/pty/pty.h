#ifndef PTY_H_
#define PTY_H_

#ifdef __cplusplus
extern "C" {
#endif

/** opaque handle to pty master-slave pair */
typedef struct _PTY	PTY;

/**
 * Spawns program 'exe' in a new pseudo-terminal
 * also writes utmp(x) log entry (if possible)
 *
 * returns NULL on failure
 */
PTY* pty_spawn(const char* exe, const char* const* envdata);
	
/**
 * Restores resources used by pty_spawn()
 * also removes utmp(x) entry (if possible)
 */
void pty_restore(PTY* pty);

/**
 * Returns master filedescriptor
 */
int pty_getfd(PTY* pty);

#ifdef __cplusplus
}
#endif


#endif
