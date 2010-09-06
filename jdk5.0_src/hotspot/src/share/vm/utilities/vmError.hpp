#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)vmError.hpp	1.4 03/12/23 16:44:53 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class VMError : public StackObj {
  int          _id;          // Solaris/Linux signals: 0 - SIGRTMAX
                             // Windows exceptions: 0xCxxxxxxx system errors
                             //                     0x8xxxxxxx system warnings

  const char * _message;

  Thread *     _thread;      // NULL if it's native thread

  // additional info for crashes
  address      _pc;          // faulting PC
  void *       _siginfo;     // ExceptionRecord on Windows,
                             // siginfo_t on Solaris/Linux
  void *       _context;     // ContextRecord on Windows,
                             // ucontext_t on Solaris/Linux

  // additional info for VM internal errors
  const char * _filename;
  int          _lineno;

  // used by fatal error handler
  int          _current_step;
  int          _verbose;

  // run cmd in a separate process and return its exit code; or -1 on failures
  int fork_and_exec(char* cmd);

  // set signal handlers on Solaris/Linux or the default exception filter
  // on Windows, to handle recursive crashes.
  void reset_signal_handlers();

  // handle -XX:+ShowMessageBoxOnError. buf is used to format the message string
  void show_message_box(char* buf, int buflen);

  // generate an error report
  void report(outputStream* st);

public:
  // Constructor for crashes
  VMError(Thread* thread, int sig, address pc, void* siginfo, void* context);
  // Constructor for VM internal errors
  VMError(Thread* thread, const char* message, const char* filename, int lineno);

  // return a string to describe the error
  char *error_string(char* buf, int buflen);

  // main error reporting function
  void report_and_die();
};

