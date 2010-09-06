#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)vmError_solaris.cpp	1.3 03/12/23 16:37:51 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_vmError_solaris.cpp.incl"

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

extern char** environ;

// Run the specified command in a separate process. Return its exit value,
// or -1 on failure (e.g. can't fork a new process). 
// Unlike system(), this function can be called from signal handler. It
// doesn't block SIGINT et al.
int VMError::fork_and_exec(char* cmd) {
  char * argv[4];
  argv[0] = (char *)"sh";
  argv[1] = (char *)"-c";
  argv[2] = cmd;
  argv[3] = NULL;

  // fork is async-safe, fork1 is not.
  pid_t pid = fork();

  if (pid < 0) {
    // fork failed
    return -1;

  } else if (pid == 0) {
    // child process

    // try to be consistent with system(), which uses "/usr/bin/sh" on Solaris
    execve("/usr/bin/sh", argv, environ);

    // execve failed
    _exit(-1);

  } else  {
    // copied from J2SE ..._waitForProcessExit() in UNIXProcess_md.c; we don't
    // care about the actual exit code, for now.

    int status;

    // Wait for the child process to exit.  This returns immediately if
    // the child has already exited. */
    while (waitpid(pid, &status, 0) < 0) {
        switch (errno) {
        case ECHILD: return 0;
        case EINTR: break;
        default: return -1;
        }
    }

    if (WIFEXITED(status)) {
       // The child exited normally; get its exit code.
       return WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
       // The child exited because of a signal
       // The best value to return is 0x80 + signal number,
       // because that is what all Unix shells do, and because
       // it allows callers to distinguish between process exit and
       // process death by signal.
       return 0x80 + WTERMSIG(status);
    } else {
       // Unknown exit code; pass it through
       return status;
    }
  }
}

void VMError::show_message_box(char *buf, int buflen) {
  bool yes;
  do {
    error_string(buf, buflen);
    int len = (int)strlen(buf);
    char *p = &buf[len];

    jio_snprintf(p, buflen - len,
               "\n\n"
               "Do you want to debug the problem?\n\n"
               "To debug, run 'dbx - %d'; then switch to thread " INTX_FORMAT "\n"
               "Enter 'yes' to launch dbx automatically (PATH must include dbx)\n"
               "Otherwise, press RETURN to abort...",
               os::current_process_id(), os::current_thread_id());

    yes = os::message_box("Unexpected Error", buf);

    if (yes) {
      // yes, user asked VM to launch debugger
      jio_snprintf(buf, buflen, "dbx - %d", os::current_process_id());

      fork_and_exec(buf);
    }
  } while (yes);
}

static void crash_handler(int sig, siginfo_t* info, void* ucVoid) {
  // unmask current signal
  sigset_t newset;
  sigemptyset(&newset);
  sigaddset(&newset, sig);
  sigprocmask(SIG_UNBLOCK, &newset, NULL);

  VMError err(NULL, sig, NULL, info, ucVoid);
  err.report_and_die();
}

void VMError::reset_signal_handlers() {
  os::signal(SIGSEGV, CAST_FROM_FN_PTR(void *, crash_handler));
  os::signal(SIGBUS, CAST_FROM_FN_PTR(void *, crash_handler));
}

