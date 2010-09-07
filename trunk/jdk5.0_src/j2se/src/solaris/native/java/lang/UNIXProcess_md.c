/*
 * @(#)UNIXProcess_md.c	1.71 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jvm.h"
#include "jvm_md.h"
#include "jni_util.h"

/*
 * Platform-specific support for java.lang.UNIXProcess
 */
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>
#include <wait.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

/* path in the environment */
static char **PATH = 0;
/* effective uid */
static uid_t uid;
/* effective group id */
static gid_t gid;

static void
parsePath(void)
{
    char *path, *c, *len;
    int count = 0;
    int i;

    /* get uid, gid */
    uid = geteuid();
    gid = getegid();
    if ((path = getenv("PATH")) == 0) {
	return;
    }
    path = strdup(path);
    len = path + strlen(path);
    /* count path elements */
    for (c = path; c < len; c++) {
	if (*c == ':')
	    count++;
    }
    PATH = (char **)malloc((++count+1) * sizeof(char*));

    /* fill it in */
    PATH[0] = path;
    PATH[count] = 0;

    for (i = 1; i < count; i++) {
	c = strchr(path, ':');
	if (c == 0) { /* shouldn't happen */
	    /*	    jio_fprintf(stderr, "processmd.c: wrong count parsing path: i=%d, count=%d\n", i, count); */
	    break;
	}
	*c++ = 0;
	PATH[i] = path = c;
    }
}

/* return 0 if it is executable && readable by this process
 * -1 if no such file, -2 if it cannot be executed.
 */
static int
statExecutable(const char *exe, struct stat *b)
{
    if (stat(exe, b)) { /* doesn't exist */
	return -1;
    }
    if (S_ISDIR(b->st_mode)) {
	/* cannot execute */
	return -2;
    }
    /* check for user permissions */
    if (b->st_uid == uid) {
	return (b->st_mode & S_IXUSR) ? 0 : -2;
    }
    /* check for group permissions */
    if (b->st_gid == gid) {
	return (b->st_mode & S_IXGRP) ? 0 : -2;
    }

    /* check for world permissions */
    return b->st_mode & S_IXOTH ? 0 : -2;
}

/* Find the command like a shell would.
 * signal an error for things not executable || not readable || not found.
 */

static char *
fullPath(JNIEnv *env, const char *part, char *full)
{
    char **tmp;
    struct stat b;
    int ret;
    /*
     * If the filename we want to exec has any slashes in it then
     * we shouldn't do a path search, as in /foo ./foo or foo/bar.
     */
    if ((strchr(part, '/') == NULL) && PATH) {
	for (tmp = PATH; *tmp; tmp++) {
	    strcpy(full, *tmp);
	    /*
	     * empty path elements are like '.' so we don't want to append
	     * a slash to them.  Otherwise foo becomes /foo.
	     */
	    if (full[0] != '\0') {
		strcat(full, "/");
	    }
	    strcat(full, part);
	    ret = statExecutable(full, &b);
	    if (ret == -1) { /* doesn't exist */
		continue;
	    } else if (ret == -2) { /* can't execute */
		continue;    /* bug 4199993 - got to keep searching. */
	    } else {
		return full;
	    }
	}
    } else if (!(ret = statExecutable(part, &b))) {
	/* always copy value to be returned so `part' may always be freed
	 * after call (if needed) */
	strcpy(full, part);
	return full;
    } else if (ret == -2) { /* cannot execute */
	jio_snprintf(full, MAXPATHLEN, "%s: cannot execute", part);
	JNU_ThrowIOException(env, full);
	return 0;
    }

    /* not found if we got here */
    jio_snprintf(full, MAXPATHLEN, "%s: not found", part);
    JNU_ThrowIOException(env, full);
    return 0;
}

#ifndef SA_NOCLDSTOP
#define SA_NOCLDSTOP 0
#endif

#ifndef SA_RESTART
#define SA_RESTART 0
#endif

static void
setSIGCHLDHandler(JNIEnv *env)
{
    /* There is a subtle difference between having the signal handler
     * for SIGCHLD be SIG_DFL and SIG_IGN.  We cannot obtain process
     * termination information for child processes if the signal
     * handler is SIG_IGN.  It must be SIG_DFL.
     *
     * We used to set the SIGCHLD handler only on Linux, but it's
     * safest to set it unconditionally.
     *
     * Consider what happens if java's parent process sets the SIGCHLD
     * handler to SIG_IGN.  Normally signal handlers are inherited by
     * children, but SIGCHLD is a controversial case.  Solaris appears
     * to always reset it to SIG_DFL, but this behavior may be
     * non-standard-compliant, and we shouldn't rely on it.
     *
     * References:
     * http://www.opengroup.org/onlinepubs/7908799/xsh/exec.html
     * http://www.pasc.org/interps/unofficial/db/p1003.1/pasc-1003.1-132.html
     */
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDSTOP | SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) < 0)
	JNU_ThrowInternalError(env, "Can't set SIGCHLD handler");
}


static jfieldID field_fd = 0;
static jfieldID field_exitcode = 0;

/* ## This should really be called just once, from a static initializer */

static int
initFieldIDs(JNIEnv *env, jobject process, jobject fd)
{
    jclass tmpC;

    if (field_exitcode != 0) return 0;

    tmpC = (*env)->GetObjectClass(env, process);
    field_exitcode = (*env)->GetFieldID(env, tmpC, "exitcode", "I");
    if (field_exitcode == 0) {
	JNU_ThrowInternalError(env, "Can't find field UNIXProcess.exitcode");
	return -1;
    }
    tmpC = (*env)->GetObjectClass(env, fd);
    field_fd = (*env)->GetFieldID(env, tmpC, "fd", "I");
    if (field_fd == 0) {
	JNU_ThrowInternalError(env, "Can't find field FileDescriptor.fd");
	field_exitcode = 0;
	return -1;
    }

    setSIGCHLDHandler(env);

    return 0;
}


#ifndef WIFEXITED
#define WIFEXITED(status) (((status)&0xFF) == 0)
#endif

#ifndef WEXITSTATUS
#define	WEXITSTATUS(status) (((status)>>8)&0xFF)
#endif

#ifndef WIFSIGNALED
#define	WIFSIGNALED(status) (((status)&0xFF) > 0 && ((status)&0xFF00) == 0)
#endif

#ifndef WTERMSIG
#define	WTERMSIG(status) ((status)&0x7F)
#endif

/* Block until a child process exits and return its exit code.
   Note, can only be called once for any given pid. */
JNIEXPORT jint JNICALL
Java_java_lang_UNIXProcess_waitForProcessExit(JNIEnv* env,
					      jobject junk,
					      jint pid)
{
    /* We used to use waitid() on Solaris, waitpid() on Linux, but
     * waitpid() is more standard, so use it on all POSIX platforms. */
    int status;
    /* Wait for the child process to exit.  This returns immediately if
       the child has already exited. */
    while (waitpid(pid, &status, 0) < 0) {
	switch (errno) {
	case ECHILD: return 0;
	case EINTR: break;
	default: return -1;
	}
    }

    if (WIFEXITED(status)) {
        /*
         * The child exited normally; get its exit code.
         */
        return WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        /* The child exited because of a signal.
	 * The best value to return is 0x80 + signal number,
	 * because that is what all Unix shells do, and because
	 * it allows callers to distinguish between process exit and
	 * process death by signal.
	 * Unfortunately, the historical behavior on Solaris is to return
	 * the signal number, and we preserve this for compatibility. */
#ifdef __solaris__
	return WTERMSIG(status);
#else
	return 0x80 + WTERMSIG(status);
#endif
    } else {
        /*
         * Unknown exit code; pass it through.
         */
	return status;
    }
}

static int
closeDescriptors(void)
{
    DIR *dp;
    struct dirent *dirp;
    int from_fd = 3;

    /* We're trying to close all file descriptors, but opendir() might
     * itself be implemented using a file descriptor, and we certainly
     * don't want to close that while it's in use.  We assume that if
     * opendir() is implemented using a file descriptor, then it uses
     * the lowest numbered file descriptor, just like open().  So we
     * close a couple explicitly.  */

    close(from_fd);		/* for possible use by opendir() */
    close(from_fd + 1);		/* another one for good luck */

    if ((dp = opendir("/proc/self/fd")) == NULL)
	return 0;

    while ((dirp = readdir(dp)) != NULL) {
	int fd;
        if (isdigit(dirp->d_name[0]) &&
	    (fd = strtol(dirp->d_name, NULL, 10)) >= from_fd + 2)
	    close(fd);
    }

    closedir(dp);

    return 1;
}

static void
moveDescriptor(int fd_from, int fd_to)
{
    if (fd_from != fd_to) {
	dup2(fd_from, fd_to);
	close(fd_from);
    }
}

static const char *
getBytes(JNIEnv *env, jbyteArray arr) {
    return arr == NULL ? NULL :
	(const char*) (*env)->GetByteArrayElements(env, arr, NULL);
}

static void
releaseBytes(JNIEnv *env, jbyteArray arr, const char* parr) {
    if (parr != NULL)
	(*env)->ReleaseByteArrayElements(env, arr, (jbyte*) parr, JNI_ABORT);
}

static void
initVectorFromBlock(const char**vector, const char* block, int count) {
    int i;
    const char *p;
    for (i = 0, p = block; i < count; i++) {
	/* Invariant: p always points to the start of a C string. */
	vector[i] = p;
	while (*(p++));
    }
    vector[count] = NULL;
}

#ifndef __solaris__
#undef fork1
#define fork1() fork()
#endif

JNIEXPORT jint JNICALL
Java_java_lang_UNIXProcess_forkAndExec(JNIEnv *env,
				       jobject process,
				       jbyteArray prog,
				       jbyteArray argBlock, jint argc,
				       jbyteArray envBlock, jint envc,
				       jbyteArray path,
				       jboolean redirectErrorStream,
				       jobject stdin_fd,
				       jobject stdout_fd,
				       jobject stderr_fd)
{
    int resultPid = -1;
    int fdin[2], fdout[2], fderr[2];
    const char **argv = NULL;
    const char **envv = NULL;
    char fullpath[MAXPATHLEN+1];
    const char *pprog     = getBytes(env, prog);
    const char *pargBlock = getBytes(env, argBlock);
    const char *penvBlock = getBytes(env, envBlock);
    const char *ppath     = getBytes(env, path);

    fdin[0] = fdin[1] = fdout[0] = fdout[1] = fderr[0] = fderr[1] = -1;

    assert(prog != NULL && argBlock != NULL);
    if (pprog     == NULL) goto failure;
    if (pargBlock == NULL) goto failure;
    if (envBlock  != NULL && penvBlock == NULL) goto failure;
    if (path      != NULL && ppath     == NULL) goto failure;

    if (initFieldIDs(env, process, stdin_fd) != 0) goto failure;

    if (PATH == 0)
	parsePath();

    /* Convert pprog + pargBlock into a char ** argv */
    if ((argv = (const char **) malloc((argc+2) * sizeof(char*))) == NULL) {
	JNU_ThrowNullPointerException(env, NULL);
	goto failure;
    }
    if (fullPath(env, pprog, fullpath) == NULL)
	/* fullPath has signalled an exception so we just return */
	goto failure;
    argv[0] = fullpath;
    initVectorFromBlock(argv+1, pargBlock, argc);

    if (envBlock != NULL) {
	/* Convert penvBlock into a char ** envv */
	if ((envv = (const char **) malloc((envc+1) * sizeof(char*))) == NULL) {
	    JNU_ThrowNullPointerException(env, NULL);
	    goto failure;
	}
	initVectorFromBlock(envv, penvBlock, envc);
    }

    if ((pipe(fdin)<0) || (pipe(fdout)<0) || (pipe(fderr)<0)) {
	char errmsg[128];
        sprintf(errmsg, "errno: %d, error: %s\n", errno, "Bad file descriptor");
        JNU_ThrowIOExceptionWithLastError(env, errmsg);
	goto failure;
    }

    resultPid = fork1();
    if (resultPid < 0) {
	char errmsg[128];
        sprintf(errmsg, "errno: %d, error: %s\n", errno, "Fork failed");
        JNU_ThrowIOExceptionWithLastError(env, errmsg);
	goto failure;
    }

    if (resultPid == 0) {
	/* Child process */

	/* Close the parent sides of the pipe.
	   Give the child sides of the pipes the right fileno's.
	   Closing pipe fds here is redundant, since closeDescriptors()
	   would do it anyways, but a little paranoia is a good thing. */
	/* Note: it is possible for fdin[0] == 0 */
	close(fdin[1]);
	moveDescriptor(fdin[0], STDIN_FILENO);
	close(fdout[0]);
	moveDescriptor(fdout[1], STDOUT_FILENO);
	close(fderr[0]);
	if (redirectErrorStream) {
	    close(fderr[1]);
	    dup2(STDOUT_FILENO, STDERR_FILENO);
	} else {
	    moveDescriptor(fderr[1], STDERR_FILENO);
	}

        /* close everything */
        if (closeDescriptors() == 0) { /* failed,  close the old way */
            int max_fd = (int)sysconf(_SC_OPEN_MAX);
	    int i;
            for (i = 3; i < max_fd; i++)
		close(i);
        }

        /* change to the new working directory */
        if (ppath != NULL) {
            if (chdir(ppath) < 0) {
#if 0 /*  __solaris__ */
		/* The well-intentioned code below tried to throw an
		   exception to our caller, but that doesn't work in a
		   child process -- it simply leaked a process.
		   Delete this code once we fix this properly. */

                /* failed to change directory, cleanup */
                char errmsg[128];
                sprintf(errmsg, "errno: %d, error: %s\n", errno, "Failed to change directory");
                JNU_ThrowByNameWithLastError(env, "java/io/IOException", errmsg);
                goto cleanup5;
#else
		/* Should really communicate this back to the parent so that it
		 * can be converted into an exception
		 */
		perror(ppath);
                _exit(-1);
#endif
            }
        }

	if (envv != NULL) {
	    /* So why isn't there an execvpe(), then? */
	    extern char **_environ;
	    _environ = (char **) envv;
	}
	execvp(argv[0], (char *const*) argv);

	/* Add error recovery here */
	_exit(-1);
    }

    /* parent process */
    (*env)->SetIntField(env, stdin_fd,  field_fd, fdin [1]);
    (*env)->SetIntField(env, stdout_fd, field_fd, fdout[0]);
    (*env)->SetIntField(env, stderr_fd, field_fd, fderr[0]);

    goto success;

 failure:
    /* Clean up the parent's side of the pipes in case of failure only */
    if (fdin [1] >= 0) close(fdin [1]);
    if (fdout[0] >= 0) close(fdout[0]);
    if (fderr[0] >= 0) close(fderr[0]);

 success:
    /* Always clean up the child's side of the pipes */
    if (fdin [0] >= 0) close(fdin [0]);
    if (fdout[1] >= 0) close(fdout[1]);
    if (fderr[1] >= 0) close(fderr[1]);

    if (argv != NULL) free(argv);
    if (envv != NULL) free(envv);

    releaseBytes(env, prog,     pprog);
    releaseBytes(env, argBlock, pargBlock);
    releaseBytes(env, envBlock, penvBlock);
    releaseBytes(env, path,     ppath);

    return resultPid;
}

JNIEXPORT void JNICALL
Java_java_lang_UNIXProcess_destroyProcess(JNIEnv *env, jobject junk, jint pid)
{
    kill(pid, SIGTERM);
}
