/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>


#include <sys/types.h>
#include <unistd.h>			// environ, execve, fcntl. Should include "environ" symbol (needs _GNU_SOURCE)
#include <fcntl.h>			// open, fcntl
#include <sys/wait.h>		// waitpid
#include <sys/resource.h>	// getrlimit
#include <signal.h>			// sigemptyset, sigprocmask

// Usage of 'environ' is not possible directly from shared libs on OSX
#ifdef __APPLE__
#	include <crt_externs.h>	// for _NSGetEnviron()
#endif

#include <sys/ioctl.h>		// ioctl
//#include <termios.h>		// unused?


//#include <signal.h>

//#include <time.h>
//#include <sys/time.h>

#include <utmp.h>
#ifdef __APPLE__ // maybe other BSD's too?
#  include <utmpx.h> //imm
#  include <util.h> //imm
#endif


#include "misc.h"
#include "pty.h"

////////////////////////////////////////////////////////////////////////////////////////////

// 0 on success, <0 on error
int te_pty_set_window_size(int fd, int width, int height) {
	struct winsize ws;

	ws.ws_col = width;
	ws.ws_row = height;

	// TODO: these are unused? (seems so in Gnome's libvte and others at least...)
	ws.ws_xpixel = 0;
	ws.ws_ypixel = 0;
	
	DEBUGF("Setting ioctl TERM dimensions to %d, %d\n", width, height);
	if (ioctl(fd, TIOCSWINSZ, &ws) == -1) {
		return -1;
	}
	return 0;
}

static void term_set_utf8(int fd, int utf8) {
#if defined(IUTF8)
	struct termios tio;
	tcflag_t saved_cflag;
	if (fd != -1) {
		if (tcgetattr(fd, &tio) != -1) {
			saved_cflag = tio.c_iflag;
			tio.c_iflag &= ~IUTF8;
			if (utf8) {
				tio.c_iflag |= IUTF8;
			}
			if (saved_cflag != tio.c_iflag) {
				tcsetattr(fd, TCSANOW, &tio);
			}
		}
	}
#endif
}


////////////////////////////////////////////////////////////////////////////////////////////
//
// These methods handles the environment variables.
// a collection of environment variables are stored as a NULL-terminated array
// of strings on the form "KEY=value". Such as:
//   const char* const env[] = {"SHELL=/bin/sh", "HOME=/home/user", NULL};
//


/**
 * Lookup value in environment given a specific variable name.
 *
 * \param env		A collection of environment variables to search in
 * \param needle	Pointer to variable name to search for
 * \param needlelen	Length of variable name
 * \return A pointer to a zero-terminated string containing the value, or NULL if no such variable name exists.
 */
static const char* _env_find_value(const char* const* env, const char* needle, size_t needlelen) {
	for (const char* const* rowp = env; *rowp != NULL; rowp++) {
		const char* var = *rowp;

		if (strncmp(var, needle, needlelen) == 0) {
			return var+needlelen+1;
		}
	}

	return NULL;
}

/**
 * Free memory allocated in _env_augment.
 * \param env	A collection to free.
 */
void te_pty_env_free(char** env) {
	for (char** e = env; *e != NULL; e++) {
		free (*e);
	}
	free(env);
}

/**
 * Augments environment with extra variables.
 * If a variable with same name already exists it's overwritten with value from envdata
 *
 * Returned data will be that from global environment, environ(7) combined with envdata.
 * Returned data should be freed with _env_free
 * does not alter global environment
 *
 * \param envdata	A collection of variables to add.
 * \return The newly allocated combined environment.
 */
char** te_pty_env_augment(const char* const* envdata) {
	// Count maximum number of environment variables from both collections

#ifdef __APPLE__
	const char* const* globalenv = (const char**)*_NSGetEnviron();
#else
	const char* const* globalenv = (const char* const*)environ;
#endif

	int nvars = 0;

	for (const char* const* e = globalenv; *e != NULL; e++) {
		nvars++;
	}
	for (const char* const* e = envdata; *e != NULL; e++) {
		nvars++;
	}

	// Allocate and clear return collection
	char** ret = (char**)malloc(sizeof(char*)*(nvars+1));
	if (ret == NULL) {
		return NULL;
	}
	memset(ret, 0, sizeof(char*)*(nvars+1));

	int pos = 0;

	// Write all variables found in environ(7), but not in envdata
	for (const char* const* varp = globalenv; *varp != NULL; varp++) {
		const char* var = *varp;

		// Sanity check environ
		const char* eqpos = strchr(var, '=');
		if (eqpos == NULL) {
			te_pty_env_free(ret);
			return NULL;
		}

		const size_t len = (size_t)((uintptr_t)eqpos-(uintptr_t)var);
		if (_env_find_value(envdata, var, len) == NULL) {
			ret[pos++] = strdup(var);
		}
	}

	// Write all variables from envdata
	for (const char* const* e = envdata; *e != NULL; e++) {
		const char* var = *e;

		// Sanity check envdata
		const char* eqpos = strchr(var, '=');
		if (eqpos == NULL) {
			te_pty_env_free(ret);
			return NULL;
		}

		ret[pos++] = strdup(var);
	}

	return ret;
}


////////////////////////////////////////////////////////////////////////////////////////////

struct TE_Pty_ {
	int			mfd;
	pid_t		childpid;

#ifdef __APPLE__
	struct utmpx	ut_entry;
#else
	struct utmp		ut_entry;
#endif
};

int te_pty_getfd(TE_Pty* pty) {
	return pty->mfd;
}

// forward decls
static void _remove_utmp();
static void _add_utmp(TE_Pty* pty, int);


static void _err_errno(const char* message, int errnum, char** err) {
	if (err == NULL) {
		return;
	}

	char buf[256];
	strerror_r(errnum, buf, sizeof(buf));


	size_t lm = strlen(message);
	size_t le = strlen(buf);

	char* s = malloc(lm+le+1);
	if (s == NULL) {
		// not much to do, unable to allocate memory for error message..
		return;
	}
	memcpy(s, message, lm);
	memcpy(s+lm, buf, le);
	s[lm+le] = '\0';
	*err = s;
}

TE_Pty* te_pty_spawn(const char *exe, const char* const* args, const char* const* env, char** err) {
	pid_t pid;

	// create augmented environment
	TE_Pty* pty = (TE_Pty*)malloc(sizeof(TE_Pty));
	if (pty == NULL) {
		_err_errno("unable to allocate memory: ", errno, err);
		return NULL;
	}

// UNIX98 ptys
	int ptm, pts=-1;	// pty master and slave fd

	ptm = posix_openpt(O_RDWR);
	if (ptm == -1) {
		_err_errno("unable to open pty master: ", errno, err);
		goto fail;
	}

	if (grantpt(ptm)) {
		_err_errno("unable to set pty slave perms: ", errno, err);
		goto fail;
	}

	if (unlockpt(ptm)) {
		_err_errno("unable to unlock pty slave: ", errno, err);
		goto fail;
	}

	// open pty slave fd
	// on linux ptsname_r exists..
	const char* slavename = ptsname(ptm);
	if (slavename != NULL) {
		pts = open(slavename, O_RDWR);
	}
	if (pts == -1) {
		_err_errno("unable to open pty slave: ", errno, err);
		goto fail;
	}

	// fork process
	pid = fork();
	if (pid == -1) {
		_err_errno("unable to fork pty child: ", errno, err);
		goto fail;
	}

	if (pid == 0) {		// slave process
		close(ptm);		// close pty master

		sigset_t sigset;
		sigemptyset(&sigset);
		if (sigprocmask(SIG_SETMASK, &sigset, NULL)) {
			fprintf(stderr, "cannot reset blocked signals, reason: %s", strerror(errno));
		}

		// SET RAW MODE
		/*
		struct termios slave_orig_term_settings; // Saved terminal settings
		struct termios new_term_settings; // Current terminal settings
		new_term_settings = slave_orig_term_settings;
		cfmakeraw (&new_term_settings);
		tcsetattr (pts, TCSANOW, &new_term_settings);
		*/

//		fprintf(stderr, "sid: %d -> %d\n", getsid(getpid()), getpid());

		// make pty slave stdin,stdout,stderr of new process
		dup2(pts, STDIN_FILENO);
		dup2(pts, STDOUT_FILENO);
		dup2(pts, STDERR_FILENO);
		if (pts > STDERR_FILENO) {	// fd useless unless already was low fd
			close(pts);
		}

		// Close all filedescriptionrs but 0,1,2
		long maxfd=sysconf(_SC_OPEN_MAX);
		struct rlimit rl;
		if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
			if (maxfd == -1 || rl.rlim_max < maxfd) {
				maxfd = rl.rlim_max;
			}
		}
		if (maxfd == -1) {
			maxfd = 9999;
		}
		for(long fd=3; fd<maxfd; fd++) {
			if (close(fd) == 0) {
//				DEBUGF("closed filedes: %ld\n", fd);
			}
		}

		
		// Make the current process a new session leader
		setsid();

		// As the child is a session leader, set the controlling terminal to be the slave side of the PTY
		// (Mandatory for programs like the shell to make them manage correctly their outputs)
		ioctl(0, TIOCSCTTY, 1);

		// set terminal to utf-8 mode
		term_set_utf8(pts, 1);

		// TODO: make -l (login option to bash etc) configurable
		execve(exe, (char**)args, (char**)env);

		// we only ever get here if execle failed
		fprintf(stderr, "unable to exec slave '%s', reason: %s", exe, strerror(errno));
		exit(1);
	}



	// master process
	close(pts);	// close pty slave

	fcntl(ptm, F_SETFL, O_NONBLOCK);

	pty->mfd = ptm;
	pty->childpid = pid;
	_add_utmp(pty, pid);

	return pty;

fail:
	if (pts != -1) {
		close(pts);
	}
	if (ptm != -1) {
		close(ptm);
	}
	if (pty != NULL) {
		free(pty);
	}
	return NULL;
}

/**
 * >=0 for exit status on normal exit
 * <signum for exit by signal
 * -100 for logic error
 */
int te_pty_restore(TE_Pty* pty) {
	close(pty->mfd);

	int ret = -100;
	if (pty->childpid != -1) {
		int status;
		pid_t r = waitpid(pty->childpid, &status, WNOHANG);
		// -1 if already awaited (would be bug in our code)
		if (r == 0 || WIFSTOPPED(status)) {	// alive..
			// TODO: what to do??
			ret = -16;
		} else {
			if (WIFEXITED(status)) {	// exited normally
				ret = WEXITSTATUS(status);
			} else if (WIFSIGNALED(status)) {
				ret = -WTERMSIG(status);
			} else { // WIFSTOPPED
			}
		}

	}


	_remove_utmp(pty);
	free(pty);
	return ret;
}


/* Linux
struct utmp {
	char	ut_line[UT_LINESIZE];			// tty name
	char	ut_name[UT_NAMESIZE];
	char	ut_host[UT_HOSTSIZE];			// host name
	long	ut_time;
};
*/

/* OSX
struct utmpx {
             char ut_user[_UTX_USERSIZE];    // login name
             char ut_id[_UTX_IDSIZE];        // id
             char ut_line[_UTX_LINESIZE];    // tty name
             pid_t ut_pid;                   // process id creating the entry
             short ut_type;                  // type of this entry
             struct timeval ut_tv;           // time entry was created
             char ut_host[_UTX_HOSTSIZE];    // host name
             __uint32_t ut_pad[16];          // reserved for future use
     };
*/


static void _add_utmp(TE_Pty* pty, int spid) {
	memset(&pty->ut_entry, 0, sizeof(pty->ut_entry) );

#if 0
	pty->ut_entry.ut_type = USER_PROCESS;
	pty->ut_entry.ut_pid = spid;
//	strcpy(pty->ut_entry.ut_line, pty->ptyname+5);

	// printf("ut name \"%s\" (%d)\n", pty->ut_entry.ut_user, getuid());

#ifdef __APPLE__
//	strcpy(pty->ut_entry.ut_id, pty->ptyname+8);
	strcpy(pty->ut_entry.ut_user, getpwuid(getuid())->pw_name);

	gettimeofday(&pty->ut_entry.ut_tv, NULL);

	setutxent();
	pututxline(&pty->ut_entry);
	endutxent();
#else
	strcpy(pty->ut_entry.ut_host, getenv("DISPLAY"));

	time_t tt;
	time(&tt);
	pty->ut_entry.ut_time = tt;
	pty->ut_entry.ut_addr = 0;

	setutent();
	pututline(&pty->ut_entry);
	endutent();
#endif

#endif
}

static void _remove_utmp(TE_Pty* pty)
{
	pty->ut_entry.ut_type = DEAD_PROCESS;
#ifdef __APPLE__
	memset(pty->ut_entry.ut_line, 0, _UTX_LINESIZE);
	pty->ut_entry.ut_tv.tv_sec = 0;
	pty->ut_entry.ut_tv.tv_usec = 0;
	memset(pty->ut_entry.ut_user, 0, _UTX_USERSIZE);

	setutxent();
	pututxline(&pty->ut_entry);
	endutxent();
#else
	memset(pty->ut_entry.ut_line, 0, UT_LINESIZE);
	pty->ut_entry.ut_time = 0;
	memset(pty->ut_entry.ut_user, 0, UT_NAMESIZE);

	setutent();
	pututline(&pty->ut_entry);
	endutent();
#endif
}
