// Copyright Timothy Miller, 1999

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <stdio.h>
#include <unistd.h> // should include "environ" symbol (needs _GNU_SOURCE)
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <grp.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <pwd.h>
#include <utmp.h>
#ifdef __APPLE__ // maybe other BSD's too?
#  include <utmpx.h> //imm
#  include <util.h> //imm
#endif
#include <stdlib.h>
#include <stdint.h>

#include "term.h"

#include "pty.h"

// TODO: gross hack, please remove
#ifdef __APPLE__ // maybe other BSD's too?
	char** environ;
#endif

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
static void _env_free(char** env) {
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
 *
 * \param envdata	A collection of variables to add.
 * \return The newly allocated combined environment.
 */
static char** _env_augment(const char* const* envdata) {
	// Count maximum number of environment variables from both collections

	int nvars = 0;

	for (char** e = environ; *e != NULL; e++) {
		nvars++;
	}
	for (const char* const* e = envdata; *e != NULL; e++) {
		nvars++;
	}

	// Allocate and clear return collection
	char** ret = (char**)malloc(sizeof(char*)*(nvars+1));
	memset(ret, 0, sizeof(char*)*(nvars+1));

	int pos = 0;

	// Write all variables found in environ(7), but not in envdata
	for (char** varp = environ; *varp != NULL; varp++) {
		const char* var = *varp;

		// Sanity check environ
		const char* eqpos = strchr(var, '=');
		if (eqpos == NULL) {
			_env_free(ret);
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
			_env_free(ret);
			return NULL;
		}

		ret[pos++] = strdup(var);
	}

	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////


struct _PTY {
	int			mfd;
	char		ptyname[128];

#ifndef __APPLE__
	struct stat	ttystat;
#endif

#ifdef __APPLE__
	struct utmpx	ut_entry;
#else
	struct utmp		ut_entry;
#endif
};

int pty_getfd(PTY* pty) {
	return pty->mfd;
}

// forward decls
static void remove_utmp();
static void add_utmp(PTY* pty, int);

static int _pts_slave(PTY* pty) {
	int sfd;
	if (grantpt(pty->mfd) != 0) {
		// TODO: handle? (errno set)
		return -1;
	}
	if (unlockpt(pty->mfd) != 0) {
		// TODO: handle? (errno set)
		return -1;
	}

	// Get the full pathname of the slave device counterpart to the master
	// device (pty->mfd).

#ifdef __APPLE__	// TODO: really, if we have ptsname_r
	// Portable, non-reentrant version
	const char* name = ptsname(pty->mfd);
	if (name == NULL) {
		// TODO: handle? (errno set)
		return -1;
	}
	strncpy(pty->ptyname, name, sizeof(pty->ptyname));
	pty->ptyname[sizeof(pty->ptyname)-1] = '\0';
#else
	// Linux-specific reentrant version
	if (ptsname_r(pty->mfd, pty->ptyname, sizeof(pty->ptyname)) != 0) {
		// TODO: handle? (errno set)
		return -1;
	}
#endif

	struct stat statbuf;
	if (stat(pty->ptyname, &statbuf) != 0) {
		// TODO: handle? (errno set)
		return -1;
	}
	
	sfd = open(pty->ptyname, O_RDWR);
	return sfd;
}

PTY* pty_spawn(const char *exe, const char* const* envdata) {
	int pid;
	PTY* pty = (PTY*)malloc(sizeof(PTY));

#ifdef __APPLE__ /* or other BSD's? */
	const int uid = getuid();
	const int gid = getgid();

	pid = forkpty(&pty->mfd, pty->ptyname, NULL, NULL);
	if (pid < 0)
	{
		fprintf(stderr, "Can't fork\n");
		return NULL;
	}

	if (!pid)
	{ // slave process
		if (setuid(uid) != 0) {
			fprintf(stderr, "WARNING: setuid(%d) failed", uid);
		}
		if (setgid(gid) != 0) {
			fprintf(stderr, "WARNING: setgid(%d) failed", gid);
		}

		// now spawn the shell in the terminal

		char** env = _env_augment(envdata);
		if (env == NULL) {
			fprintf(stderr, "big problems...\n");
			exit(1);
		}
		// TODO: make -l (login option to bash etc) configurable
		execle(exe, exe, "-l", NULL, env);
		_env_free(env);
		exit(0);
	}
	// else master process
//printf("pty is: %s\n", pty_name);
	add_utmp(pty, pid);

	return pty;
#else // non-Apple pty fork
	pty->mfd = getpt();

	if (pty->mfd < 0) {
		fprintf(stderr, "Can't open master pty\n");
		free(pty);
		return NULL;
	}

	pid = fork();
	if (pid < 0) {
		fprintf(stderr, "Can't fork\n");
		free(pty);
		return NULL;
	}

	if (!pid) { // slave process
		int sfd = _pts_slave(pty);
		close(pty->mfd);

		if (sfd < 0) {
			fprintf(stderr, "Can't open child (%d)\n", sfd);
			free(pty);
			return NULL;
		}

//		setuid(uid);
//		setgid(gid);

		if (setsid() < 0)
			fprintf(stderr, "Could not set session leader\n");

//		if (ioctl(sfd, TIOCSCTTY, NULL))
//			fprintf(stderr, "Could not set controllint tty\n");

		dup2(sfd, STDIN_FILENO);
		dup2(sfd, STDOUT_FILENO);
		dup2(sfd, STDERR_FILENO);
		if (sfd > 2)
			close(sfd);

		term_set_utf8(sfd, 1);

		char** env = _env_augment(envdata);
		if (env == NULL) {
			fprintf(stderr, "big problems...\n");
			exit(1);
		}
		execle(exe, exe, "-l", NULL, env);
		// FIXME: execle doesn't ever return
		_env_free(env);
		exit(0);
	} // end of slave process
// else the master process...
	add_utmp(pty, pid);

	return pty;
#endif /* end of "non-Apple" pts fork */
}

void pty_restore(PTY* pty) {
#ifndef __APPLE__
	chown(pty->ptyname, pty->ttystat.st_uid, pty->ttystat.st_gid);
	chmod(pty->ptyname, pty->ttystat.st_mode);
#endif

	remove_utmp();

	free(pty);
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


static void add_utmp(PTY* pty, int spid) {
	memset(&pty->ut_entry, 0, sizeof(pty->ut_entry) );
	
	pty->ut_entry.ut_type = USER_PROCESS;
	pty->ut_entry.ut_pid = spid;
	strcpy(pty->ut_entry.ut_line, pty->ptyname+5);
	strcpy(pty->ut_entry.ut_id, pty->ptyname+8);
	strcpy(pty->ut_entry.ut_user, getpwuid(getuid())->pw_name);

	// printf("ut name \"%s\" (%d)\n", pty->ut_entry.ut_user, getuid());

#ifdef __APPLE__
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
}

static void remove_utmp(PTY* pty)
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

/* end of file */
