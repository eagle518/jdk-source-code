/*
 * Copyright (c) 2003, 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

# include "incls/_precompiled.incl"
# include "incls/_vmError_windows.cpp.incl"


void VMError::show_message_box(char *buf, int buflen) {
  bool yes;
  do {
    error_string(buf, buflen);
    int len = (int)strlen(buf);
    char *p = &buf[len];

    jio_snprintf(p, buflen - len,
               "\n\n"
               "Do you want to debug the problem?\n\n"
               "To debug, attach Visual Studio to process %d; then switch to thread 0x%x\n"
               "Select 'Yes' to launch Visual Studio automatically (PATH must include msdev)\n"
               "Otherwise, select 'No' to abort...",
               os::current_process_id(), os::current_thread_id());

    yes = os::message_box("Unexpected Error", buf) != 0;

    if (yes) {
      // yes, user asked VM to launch debugger
      //
      // os::breakpoint() calls DebugBreak(), which causes a breakpoint
      // exception. If VM is running inside a debugger, the debugger will
      // catch the exception. Otherwise, the breakpoint exception will reach
      // the default windows exception handler, which can spawn a debugger and
      // automatically attach to the dying VM.
      os::breakpoint();
      yes = false;
    }
  } while (yes);
}

int VMError::get_resetted_sigflags(int sig) {
  return -1;
}

address VMError::get_resetted_sighandler(int sig) {
  return NULL;
}

LONG WINAPI crash_handler(struct _EXCEPTION_POINTERS* exceptionInfo) {
  DWORD exception_code = exceptionInfo->ExceptionRecord->ExceptionCode;
  VMError err(NULL, exception_code, NULL,
                exceptionInfo->ExceptionRecord, exceptionInfo->ContextRecord);
  err.report_and_die();
  return EXCEPTION_CONTINUE_SEARCH;
}

void VMError::reset_signal_handlers() {
  SetUnhandledExceptionFilter(crash_handler);
}
