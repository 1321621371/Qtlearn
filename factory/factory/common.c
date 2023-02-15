#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

//#define __USE_GNU
#include <unistd.h>

#include "common.h"

#undef _PATH_BSHELL

#define _PATH_BSHELL "/sbin/sh"

#if 0
int __system(const char *command)
{
	pid_t pid;
	sig_t intsave, quitsave;
	sigset_t mask, omask;
	int pstat;
	char *argp[] = {"sh", "-c", NULL, NULL};

	if (!command)		/* just checking... */
		return(1);

	argp[2] = (char *)command;

	sigemptyset(&mask);
	sigaddset(&mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &mask, &omask);
	switch (pid = vfork()) {
	case -1:			/* error */
		sigprocmask(SIG_SETMASK, &omask, NULL);
		return(-1);
	case 0:				/* child */
		sigprocmask(SIG_SETMASK, &omask, NULL);
		execve(_PATH_BSHELL, argp, environ);
		_exit(127);
	}

	intsave = (sig_t)  bsd_signal(SIGINT, SIG_IGN);
	quitsave = (sig_t) bsd_signal(SIGQUIT, SIG_IGN);
	pid = waitpid(pid, (int *)&pstat, 0);
	sigprocmask(SIG_SETMASK, &omask, NULL);
	(void)bsd_signal(SIGINT, intsave);
	(void)bsd_signal(SIGQUIT, quitsave);
	return (pid == -1 ? -1 : pstat);
}
#endif

static char *string_replace(const char *orig, const char *old, const char *new)
{
	char *result;
	int i, cnt = 0;
	int newWlen = strlen(new);
	int oldWlen = strlen(old);

	if (!orig || !old || !new)
		return NULL;

	if (strlen(old) == 0)
		return NULL;

	// Counting the number of times old word
	// occur in the string
	for (i = 0; orig[i] != '\0'; i++) {
		if (strstr(&orig[i], old) == &orig[i]) {
			cnt++;
			i += oldWlen - 1;
		}
	}

	// Making new string of enough length
	result = (char *)malloc(i + cnt * (newWlen - oldWlen) + 1);
	if (!result)
		return NULL;

	i = 0;
	while (*orig) {
		// compare the substring with the result
		if (strstr(orig, old) == orig) {
			strcpy(&result[i], new);
			i += newWlen;
			orig += oldWlen;
		} else {
			result[i++] = *orig++;
		}
	}

	result[i] = '\0';
	return result;
}

int find_dev_node(const char *sysNode, const char *prefix, char *retDevNode)
{
        DIR *dir;
        struct dirent* de;
	char symlinkPath[32] = {0};
	char *realPath, *absPath;

        if (!sysNode || !prefix || !retDevNode)
                return -1;

        dir = opendir(sysNode);
        if (dir == NULL) {
                LOGE("sys dir not exist: %s\n", sysNode);
                return -1;
        }
        closedir(dir);

        dir = opendir("/sys/block");
        if (dir == NULL) {
                LOGE("open /sys/block failed.\n");
                return -1;
        }

        while ((de = readdir(dir))) {
                LOGV("/sys/block/%s\n", de->d_name);
                if (strncmp(de->d_name, prefix, strlen(prefix)) != 0)
                        continue;

		sprintf(symlinkPath, "/sys/block/%s", de->d_name);
		realPath = realpath(symlinkPath, NULL);
		if (realPath == NULL)
			continue;

		LOGV("realpath: %s\n", realPath);
		absPath = string_replace(realPath, "..", "/sys");
		if (strstr(absPath, sysNode)) {
			sprintf(retDevNode, "/dev/%s", de->d_name);
			free(realPath);
			free(absPath);
			return 0;
		}

		free(realPath);
		free(absPath);
	}

        closedir(dir);
	return -2;
}
