#ifndef PTY_H_
#define PPTY_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Spawns program 'exe' in a new pseudo-terminal
 * fd is returned on success, or -1 on error.
 */
int			spawn(const char* exe);
void		restore_ttyp();

#ifdef __cplusplus
}
#endif


#endif /* PSEUDO_HPP_ */
