#ifndef PSEUDO_HPP_
#define PSEUDO_HPP_

/**
 * Spawns program 'exe' in a new pseudo-terminal
 * fd is returned on success, or -1 on error.
 */
int			spawn(const char* exe);
void		restore_ttyp();

#endif /* PSEUDO_HPP_ */
