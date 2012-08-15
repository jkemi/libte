#ifndef PTY_H_
#define PTY_H_

#ifdef __cplusplus
extern "C" {
#endif

/** opaque handle to encapsulation of pty and child process logic */
typedef struct _PTY	PTY;

char** pty_env_augment(const char* const* envdata);
void pty_env_free(char** env);
	
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
PTY* pty_spawn(const char* exe, const char* const* args, const char* const* envdata, char** err);
	
/**
 * Restores resources used by pty_spawn()
 * also removes utmp(x) entry (if possible).
 *
 * \param slave handle
 *
 * \return	>=0	means status code on normal child process exit
 *			<0	means -signum if killed by signal signum
 */
int pty_restore(PTY* pty);

/**
 * Get file descriptor to use for communication.
 *
 * \param slave handle
 * \return file descriptor
 */
int pty_getfd(PTY* pty);
	
// 0 on success
int pty_set_window_size(int fd, int width, int height);

#ifdef __cplusplus
}
#endif


#endif
