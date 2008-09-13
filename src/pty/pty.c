// Copyright Timothy Miller, 1999

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <stdio.h>
#include <unistd.h>
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

#include "pty.h"

static int master_fd;
static char pty_name[32];

static void remove_utmp();
static void add_utmp(int);

static const char* _find_env_value(const char* const* envdata, const char* needle, size_t needlelen) {
	for (const char* const* envpos = envdata; *envpos != NULL; envpos++) {
		const char* env = *envpos;

		if (strncmp(env, needle, needlelen) == 0) {
			return env+needlelen+1;
		}
	}

	return NULL;
}

static const char* _find_env_value_(char** envdata, const char* needle, size_t needlelen) {
	return _find_env_value((const char* const*)envdata, needle, needlelen);
}

static void _free_env(char** env) {
	for (char** e = env; *e != NULL; e++) {
		free (*e);
	}
	free(env);
}

static char**_augment_environment(const char* const* envdata) {
	// Count number of environment variables
	int n = 0;
	for (char** e = environ; *e != NULL; e++) {
		n++;
	}

	// Count number of arguments to augment with
	int nextra = 0;
	for (const char* const* e = envdata; *e != NULL; e++) {
		nextra++;
	}

	char** ret = (char**)malloc(sizeof(char*)*(n+nextra+1));
	memset(ret, 0, sizeof(char*)*(n+nextra+1));

	int retp = 0;

	for (int i = 0; i < n; i++) {
		const char* org = environ[i];
		const char* pos = strchr(org, '=');
		if (pos == NULL) {
			free(ret);
			return NULL;
		}

		const size_t len = (size_t)((uintptr_t)pos-(uintptr_t)org);

		const char* newval = _find_env_value(envdata, org, len);
		char* val = NULL;
		if (newval) {
			size_t sz = len+1+strlen(newval)+1;
			val = (char*)malloc(sizeof(char)*sz);
			char* pos = val;
			memcpy(pos, org, sizeof(char)*(len+1));
			pos += len+1;
			strcpy(pos, newval);
		} else {
			val = strdup(org);
		}

		ret[retp++] = val;
	}

	for (const char* const* e = envdata; *e != NULL; e++) {
		const char* env = *e;
		const char* pos = strchr(env, '=');
		if (pos == NULL) {
			_free_env(ret);
			return NULL;
		}

		const size_t len = (size_t)((uintptr_t)pos-(uintptr_t)env);
		if (_find_env_value_(environ, env, len) == NULL) {
			ret[retp++] = strdup(env);
		}

	}

	return ret;
}

int pts_slave(int mfd)
{
	struct stat statbuf;
	int sfd;
	int fail = grantpt(mfd);
	if (fail)
		return -1;
	fail = unlockpt(mfd);
	if (fail)
		return -2;
	strncpy(pty_name, ptsname(mfd), 255);
	if (stat(pty_name, &statbuf) < 0)
		return -3;

	sfd = open(pty_name, O_RDWR);
	return sfd;
}

struct stat tty_stat;

int pty_spawn(const char *exe, const char* const* envdata)
{
	int mfd, pid, sfd;
	int uid, gid;

	uid = getuid();
	gid = getgid();

#ifdef __APPLE__ /* or other BSD's? */
	pid = forkpty(&mfd, pty_name, NULL, NULL);
	if (pid < 0)
	{
		fprintf(stderr, "Can't fork\n");
		return -1;
	}

	sfd = 0; // what to do about the slave pty fd???

	if (!pid)
	{ // slave process
		setuid(uid);
		setgid(gid);

		// now spawn the shell in the terminal
		execl(exe, exe, NULL);
		exit(0);
	}
	// else master process
//printf("pty is: %s\n", pty_name);
	add_utmp(pid);

	master_fd = mfd;
	return mfd;
#else // non-Apple pty fork
	mfd = getpt();

	if (mfd < 0) {
		fprintf(stderr, "Can't open master pty\n");
		return -1;
	}

	pid = fork();
	if (pid < 0)
	{
		fprintf(stderr, "Can't fork\n");
		return -1;
	}

	if (!pid)
	{ // slave process
		sfd = pts_slave(mfd);
		close(mfd);

		if (sfd < 0)
		{
			fprintf(stderr, "Can't open child (%d)\n", sfd);
			return -1;
		}

//		setuid(uid);
//		setgid(gid);

		if (setsid() < 0)
			fprintf(stderr, "Could not set session leader\n");

//		if (ioctl(sfd, TIOCSCTTY, NULL))
//			fprintf(stderr, "Could not set controllint tty\n");

		dup2(sfd, 0);
		dup2(sfd, 1);
		dup2(sfd, 2);
		if (sfd > 2)
			close(sfd);

		char** env = _augment_environment(envdata);
		if (env == NULL) {
			fprintf(stderr, "big problems...\n");
			exit(1);
		}
		execle(exe, exe, NULL, env);
		_free_env(env);
		exit(0);
	} // end of slave process
// else the master process...
	add_utmp(pid);

	master_fd = mfd;
	return mfd;
#endif /* end of "non-Apple" pts fork */
}

void pty_restore()
{
	chown(pty_name, tty_stat.st_uid, tty_stat.st_gid);
	chmod(pty_name, tty_stat.st_mode);

	remove_utmp();
}

/*
struct utmp {
	char	ut_line[UT_LINESIZE];
	char	ut_name[UT_NAMESIZE];
	char	ut_host[UT_HOSTSIZE];
	long	ut_time;
};
*/

#if 0
struct utmpx {
             char ut_user[_UTX_USERSIZE];    /* login name */
             char ut_id[_UTX_IDSIZE];        /* id */
             char ut_line[_UTX_LINESIZE];    /* tty name */
             pid_t ut_pid;                   /* process id creating the entry */
             short ut_type;                  /* type of this entry */
             struct timeval ut_tv;           /* time entry was created */
             char ut_host[_UTX_HOSTSIZE];    /* host name */
             __uint32_t ut_pad[16];          /* reserved for future use */
     };
#endif

#ifdef __APPLE__
struct utmpx ut_entry;
#else
struct utmp ut_entry;
#endif

static void add_utmp(int spid)
{

	ut_entry.ut_type = USER_PROCESS;
	ut_entry.ut_pid = spid;
	strcpy(ut_entry.ut_line, pty_name+5);
	strcpy(ut_entry.ut_id, pty_name+8);
	strcpy(ut_entry.ut_user, getpwuid(getuid())->pw_name);

//	printf("ut name \"%s\" (%d)\n", ut_entry.ut_user, getuid());

#ifdef __APPLE__
	gettimeofday(&ut_entry.ut_tv, NULL);

	setutxent();
	pututxline(&ut_entry);
	endutxent();
#else
	strcpy(ut_entry.ut_host, getenv("DISPLAY"));

	time_t tt;
	time(&tt);
	ut_entry.ut_time = tt;
	ut_entry.ut_addr = 0;

	setutent();
	pututline(&ut_entry);
	endutent();
#endif
}

static void remove_utmp()
{
	ut_entry.ut_type = DEAD_PROCESS;
#ifdef __APPLE__
	memset(ut_entry.ut_line, 0, _UTX_LINESIZE);
	ut_entry.ut_tv.tv_sec = 0;
	ut_entry.ut_tv.tv_usec = 0;
	memset(ut_entry.ut_user, 0, _UTX_USERSIZE);

	setutxent();
	pututxline(&ut_entry);
	endutxent();
#else
	memset(ut_entry.ut_line, 0, UT_LINESIZE);
	ut_entry.ut_time = 0;
	memset(ut_entry.ut_user, 0, UT_NAMESIZE);

	setutent();
	pututline(&ut_entry);
	endutent();
#endif
}

/* end of file */
