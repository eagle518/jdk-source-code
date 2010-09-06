#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)vmError.cpp	1.9 04/03/02 17:17:08 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_vmError.cpp.incl"

// List of environment variables that should be reported in error log file.
const char *env_list[] = {
  // All platforms
  "JAVA_HOME", "JRE_HOME", "JAVA_TOOL_OPTIONS", "_JAVA_OPTIONS", "CLASSPATH", 
  "JAVA_COMPILER", "PATH", "USERNAME",

  // Env variables that are defined on Solaris/Linux
  "LD_LIBRARY_PATH", "LD_PRELOAD", "SHELL", "DISPLAY", 
  "HOSTTYPE", "OSTYPE", "ARCH", "MACHTYPE",

  // defined on Linux
  "LD_ASSUME_KERNEL", "_JAVA_SR_SIGNUM",

  // defined on Windows
  "OS", "PROCESSOR_IDENTIFIER", "_ALT_JAVA_HOME_DIR",

  (const char *)0
};

// Fatal error handler for internal errors and crashes.
//
// The default behavior of fatal error handler is to print a brief message
// to standard out (defaultStream::output_fd()), then save detailed information
// into an error report file (hs_err_pid<pid>.log) and abort VM. If multiple 
// threads are having troubles at the same time, only one error is reported.
// The thread that is reporting error will abort VM when it is done, all other
// threads are blocked forever inside report_and_die().

// Constructor for crashes
VMError::VMError(Thread* thread, int sig, address pc, void* siginfo, void* context) {
    _thread = thread;
    _id = sig;
    _pc   = pc;
    _siginfo = siginfo;
    _context = context;

    _verbose = false;
    _current_step = 0;

    _message = "";
    _filename = NULL;
    _lineno = 0;
}

// Constructor for internal errors
VMError::VMError(Thread* thread, const char* message, const char* filename, int lineno) {
    _thread = thread;
    _id = 0xE0000000;     // set it to a value that's not an OS exception/signal
    _filename = filename;
    _lineno = lineno;
    _message = message;

    _verbose = false;
    _current_step = 0;

    _pc = NULL;
    _siginfo = NULL;
    _context = NULL;
}

// -XX:OnError=<string>, where <string> can be a list of commands, separated
// by ';'. "%p" is replaced by current process id (pid); "%%" is replaced by
// a single "%". Some examples:
//
// -XX:OnError="pmap %p"                // show memory map
// -XX:OnError="gcore %p; dbx - %p"     // dump core and launch debugger
// -XX:OnError="cat hs_err_pid%p.log | mail my_email@sun.com"
// -XX:OnError="kill -9 %p"             // ?#!@#

// A simple parser for -XX:OnError, usage:
//  ptr = OnError;
//  while ((cmd = next_OnError_command(buffer, sizeof(buffer), &ptr) != NULL)
//     ... ...
static char* next_OnError_command(char* buf, int buflen, const char** ptr) {
  if (ptr == NULL || *ptr == NULL) return NULL;

  const char* cmd = *ptr;

  // skip leading blanks or ';'
  while (*cmd == ' ' || *cmd == ';') cmd++;

  if (*cmd == '\0') return NULL;

  const char * cmdend = cmd;
  while (*cmdend != '\0' && *cmdend != ';') cmdend++;

  // copy command into buf, replace "%%" with "%" and "%p" with pid
  const char *p = cmd;
  char *b = buf;
  char *bufend = &buf[buflen - 1];

  while (p < cmdend && b < bufend) {
    if (*p == '%') {
     switch (*(++p)) {
       case '%':         // "%%" ==> "%"
          *b++ = *p++;
          break;
       case 'p':         //  "%p" ==> current process id
          jio_snprintf(b, bufend - b, "%d", os::current_process_id());
          while (*b != '\0') b++;
          p++;
          break;
       default : 
          *b++ = '%';
     }
    } else {
     *b++ = *p++;
    }
  }
  *b = '\0';

  *ptr = (*cmdend == '\0' ? cmdend : cmdend + 1);
  return buf;
}

// Return a string to describe the error
char* VMError::error_string(char* buf, int buflen) {
  char signame_buf[64];
  const char *signame = os::exception_name(_id, signame_buf, sizeof(signame_buf));

  if (signame) {
    jio_snprintf(buf, buflen,
                 "%s (0x%x) at pc=" PTR_FORMAT ", pid=%d, tid=" UINTX_FORMAT, 
                 signame, _id, _pc,
                 os::current_process_id(), os::current_thread_id());
  } else {
    if (_filename != NULL && _lineno > 0) {
      // skip directory names
      char separator = os::file_separator()[0];
      const char *p = strrchr(_filename, separator);

      jio_snprintf(buf, buflen,
        "Internal Error at %s:%d, pid=%d, tid=" UINTX_FORMAT,
        p ? p + 1 : _filename, _lineno, 
        os::current_process_id(), os::current_thread_id());
    } else {
      jio_snprintf(buf, buflen,
        "Internal Error (0x%x), pid=%d, tid=" UINTX_FORMAT,
        _id, os::current_process_id(), os::current_thread_id());
    }
  }

  return buf;
}

// This is the main function to report a fatal error. Only one thread can
// call this function, so we don't need to worry about MT-safety. But it's 
// possible that the error handler itself may crash or die on an internal 
// error, for example, when the stack/heap is badly damaged. We must be 
// able to handle recursive errors that happen inside error handler.
// 
// Error reporting is done in several steps. If a crash or internal error
// occurred when reporting an error, the nested signal/exception handler 
// can skip steps that are already (or partially) done. Error reporting will 
// continue from the next step. This allows us to retrieve and print 
// information that may be unsafe to get after a fatal error. If it happens,
// you may find nested report_and_die() frames when you look at the stack
// in a debugger.
//
// In general, a hang in error handler is much worse than a crash or internal
// error, as it's harder to recover from a hang. Deadlock can happen if we 
// try to grab a lock that is already owned by current thread, or if the 
// owner is blocked forever (e.g. in os::infinite_sleep()). If possible, the 
// error handler and all the functions it called should avoid grabbing any 
// lock. An important thing to notice is that memory allocation needs a lock.
//
// We should avoid using large stack allocated buffers. Many errors happen 
// when stack space is already low. Making things even worse is that there 
// could be nested report_and_die() calls on stack (see above). Only one
// thread can report error, so large buffers are statically allocated in data 
// segment.

void VMError::report(outputStream* st) {
# define BEGIN if (_current_step == 0) { _current_step = 1;
# define STEP(n) } if (_current_step < n) { _current_step = n;
# define END }

  // don't allocate large buffer on stack
  static char buf[O_BUFLEN];

  BEGIN

  STEP(10)

     st->print_cr("#");
     st->print_cr("# An unexpected error has been detected by HotSpot Virtual Machine:");

  STEP(20)

     st->print_cr("#");
     st->print("#  ");
     // Is it an OS exception/signal?
     if (os::exception_name(_id, buf, sizeof(buf))) {
       st->print("%s", buf);
       st->print(" (0x%x)", _id);                // signal number
       st->print(" at pc=" PTR_FORMAT, _pc);
     } else {
       st->print("Internal Error");
       if (_filename != NULL && _lineno > 0) {
         obfuscate_location(_filename, _lineno, buf, sizeof(buf));
         st->print(" (%s)", buf);
       } else {
         st->print(" (0x%x)", _id);
       }
     }

  STEP(30)

     // process id, thread id
     st->print(", pid=%d", os::current_process_id());
     st->print(", tid=" UINTX_FORMAT, os::current_thread_id());
     st->cr();

  STEP(40)

     // VM version
     st->print_cr("#");
     st->print_cr("# Java VM: %s (%s %s)",
                   Abstract_VM_Version::vm_name(),
                   Abstract_VM_Version::vm_release(),
                   Abstract_VM_Version::vm_info_string()
                 );

  STEP(50)

#ifndef PRODUCT
     // error message
     if (_message && _message[0] != '\0') {
       st->print_cr("#");
       st->print_cr("# Error: %s", _message);
     }
#endif PRODUCT

  STEP(60)

     // Print current frame if we have a context (i.e. it's a crash)
     if (_context) {
       st->print_cr("# Problematic frame:");
       st->print("# ");
       frame fr = os::fetch_frame_from_context(_context);
       fr.print_on_error(st, buf, sizeof(buf));
       st->cr();
       st->print_cr("#");
     }

  STEP(70)

     if (_verbose) {
       st->cr();
       st->print_cr("---------------  T H R E A D  ---------------");
       st->cr();
     }

  STEP(80)

     // current thread
     if (_verbose) {
       if (_thread) {
         st->print("Current thread (" PTR_FORMAT "):  ", _thread);
         _thread->print_on_error(st, buf, sizeof(buf));
         st->cr();
       } else {
         st->print_cr("Current thread is native thread");
       }
       st->cr();
     }

  STEP(90)

     // signal no, signal code, address that caused the fault
     if (_verbose && _siginfo) {
       os::print_siginfo(st, _siginfo);
       st->cr();
     }

  STEP(100)

     // registers, top of stack, instructions near pc
     if (_verbose && _context) {
       os::print_context(st, _context);
       st->cr();
     }

  STEP(110)

     if (_verbose) {
       st->print("Stack: ");
 
       address stack_top, stack_bottom;

       if (_thread) {
          stack_top = _thread->stack_base();
          size_t stack_size = _thread->stack_size();
          stack_bottom = stack_top - stack_size;
       } else {
          stack_top = os::current_stack_base();
          size_t stack_size = os::current_stack_size();
          stack_bottom = stack_top - stack_size;
       }

       st->print("[" PTR_FORMAT "," PTR_FORMAT ")", stack_bottom, stack_top);

       frame fr = _context ? os::fetch_frame_from_context(_context)
                           : os::current_frame();

       if (fr.sp()) {
         st->print(",  sp=" PTR_FORMAT, fr.sp());
         st->print(",  free space=%dk", 
                     ((intptr_t)fr.sp() - (intptr_t)stack_bottom) >> 10);
       }

       st->cr();
     }

  STEP(120)

     if (_verbose) {
       frame fr = _context ? os::fetch_frame_from_context(_context)
                           : os::current_frame();

       // see if it's a valid frame
       if (fr.pc()) {
          st->print_cr("Native frames: (J=compiled Java code, j=interpreted, Vv=VM code, C=native code)");

          int count = 0;

          while (count++ < StackPrintLimit) {
             fr.print_on_error(st, buf, sizeof(buf));
             st->cr();
             if (os::is_first_C_frame(&fr)) break;
             fr = os::get_sender_for_C_frame(&fr);
          }
                                                                                
          if (count > StackPrintLimit) {
             st->print_cr("...<more frames>...");
          }

          st->cr();
       }
     }

  STEP(130)

     if (_verbose && _thread && _thread->is_Java_thread()) {
       JavaThread* jt = (JavaThread*)_thread;
       if (jt->has_last_Java_frame()) {
         st->print_cr("Java frames: (J=compiled Java code, j=interpreted, Vv=VM code)");
         for(StackFrameStream sfs(jt); !sfs.is_done(); sfs.next()) {
           sfs.current()->print_on_error(st, buf, sizeof(buf));
           st->cr();
         }
       }
     }

  STEP(140)

     if (_verbose && _thread && _thread->is_VM_thread()) {
        VMThread* t = (VMThread*)_thread;
        VM_Operation* op = t->vm_operation();
        if (op) {
          op->print_on_error(st);
          st->cr();
          st->cr();
        }
     }

  STEP(150)

#ifndef CORE
     if (_verbose && _thread && _thread->is_Compiler_thread()) {
        CompilerThread* t = (CompilerThread*)_thread;
        if (t->task()) {
           st->cr();
           st->print_cr("Current CompileTask:");
           t->task()->print_line_on_error(st, buf, sizeof(buf));
           st->cr();
        }
     }
#endif

  STEP(160)

     if (_verbose) {
       st->cr();
       st->print_cr("---------------  P R O C E S S  ---------------");
       st->cr();
     }

  STEP(170)

     // all threads
     if (_verbose) {
       Threads::print_on_error(st, _thread, buf, sizeof(buf));
       st->cr();
     }

  STEP(175)

     if (_verbose) {
       // Safepoint state
       st->print("VM state:");

       if (SafepointSynchronize::is_synchronizing()) st->print("synchronizing");
       else if (SafepointSynchronize::is_at_safepoint()) st->print("at safepoint");
       else st->print("not at safepoint");

       // Also see if error occurred during initialization or shutdown
       if (!Universe::is_fully_initialized()) {
         st->print(" (not fully initilizated)");
       } else if (VM_Exit::vm_exited()) {
         st->print(" (shutting down)");
       } else {
         st->print(" (normal execution)");
       }
       st->cr();
       st->cr();
     }

  STEP(180)

     // mutexes/monitors that currently have an owner
     if (_verbose) {
       print_owned_locks_on_error(st);
       st->cr();
     }

  STEP(190)

     if (_verbose) {
       // print heap information before vm abort
       Universe::print_on(st);
       st->cr();
     }

  STEP(200)

     if (_verbose) {
       // dynamic libraries, or memory map
       os::print_dll_info(st);
       st->cr();
     }

  STEP(210)

     if (_verbose) {
       // VM options
       Arguments::print_on(st);
       st->cr();
     }

  STEP(220)

     if (_verbose) {
       os::print_environment_variables(st, env_list, buf, sizeof(buf));
       st->cr();
     }

  STEP(230)

     if (_verbose) {
       st->cr();
       st->print_cr("---------------  S Y S T E M  ---------------");
       st->cr();
     }

  STEP(240)

     if (_verbose) {
       os::print_os_info(st);
       st->cr();
     }

  STEP(250)
     if (_verbose) {
       os::print_cpu_info(st);
       st->cr();
     }

  STEP(260)

     if (_verbose) {
       os::print_memory_info(st);
       st->cr();
     }

  STEP(270)

     if (_verbose) {
       st->print_cr("vm_info: %s", Abstract_VM_Version::internal_vm_info_string());
       st->cr();
     }

  END

# undef BEGIN
# undef STEP
# undef END
}

void VMError::report_and_die() {
  // Don't allocate large buffer on stack
  static char buffer[O_BUFLEN];

  // First error, and its thread id. We must be able to handle native thread,
  // so use thread id instead of Thread* to identify thread.
  static VMError* first_error;
  static jlong    first_error_tid;

  // An error could happen before tty is initialized or after it has been
  // destroyed. Here we use a very simple unbuffered fdStream for printing.
  // Only out.print_raw() and out.print_raw_cr() should be used, as other
  // printing methods need to allocate large buffer on stack. To format a 
  // string, use jio_snprintf() with a static buffer or use staticBufferStream.
  static fdStream out(defaultStream::output_fd());

  // How many errors occurred in error handler when reporting first_error.
  static int recursive_error_count;

  // We will first print a brief message to standard out (verbose = false),
  // then save detailed information in log file (verbose = true).
  static bool out_done = false;         // done printing to standard out
  static bool log_done = false;         // done saving error log
  static fdStream log;                  // error log

  jlong mytid = os::current_thread_id();
  if (first_error == NULL &&
      Atomic::cmpxchg_ptr(this, &first_error, NULL) == NULL) {

    // first time
    first_error_tid = mytid;
    set_error_reported();

    if (ShowMessageBoxOnError) {
      show_message_box(buffer, sizeof(buffer));

      // User has asked JVM to abort. Reset ShowMessageBoxOnError so the 
      // WatcherThread can kill JVM if the error handler hangs.
      ShowMessageBoxOnError = false;
    }

    // reset signal handlers or exception filter; make sure recursive crashes
    // are handled properly.
    reset_signal_handlers();

  } else {
    // This is not the first error, see if it happened in a different thread
    // or in the same thread during error reporting.
    if (first_error_tid != mytid) {
      jio_snprintf(buffer, sizeof(buffer),
                   "[thread " INT64_FORMAT " also had an error]",
                   mytid);
      out.print_raw_cr(buffer);

      // error reporting is not MT-safe, block current thread
      os::infinite_sleep();

    } else {
      if (recursive_error_count++ > 30) {
        out.print_raw_cr("[Too many errors, abort]");
        os::die();
      }

      jio_snprintf(buffer, sizeof(buffer),
                   "[error occurred during error reporting, step %d, id 0x%x]",
                   first_error ? first_error->_current_step : -1,
                   _id);
      if (log.is_open()) {
        log.cr();
        log.print_raw_cr(buffer);
        log.cr();
      } else {
        out.cr();
        out.print_raw_cr(buffer);
        out.cr();
      }
    }
  }

  // print to screen
  if (!out_done) {
    first_error->_verbose = false;

    staticBufferStream sbs(buffer, sizeof(buffer), &out);
    first_error->report(&sbs);

    out_done = true;

    first_error->_current_step = 0;         // reset current_step
  }

  // print to error log file
  if (!log_done) {
    first_error->_verbose = true;

    // see if log file is already open
    if (!log.is_open()) {
      // open log file
      jio_snprintf(buffer, sizeof(buffer), "hs_err_pid%u.log", os::current_process_id());
      int fd = open(buffer, O_WRONLY | O_CREAT | O_TRUNC, 0666);

      if (fd == -1) {
        // try temp directory
        const char * tmpdir = os::get_temp_directory();
        jio_snprintf(buffer, sizeof(buffer), "%shs_err_pid%u.log",
                     (tmpdir ? tmpdir : ""), os::current_process_id());
        fd = open(buffer, O_WRONLY | O_CREAT | O_TRUNC, 0666);
      }

      if (fd != -1) {
        out.print_raw   ("# An error report file with more information is saved as ");
        out.print_raw_cr(buffer);

        log.set_fd(fd);
      } else {
        out.print_raw_cr("# Can not save log file, dump to screen..");
        log.set_fd(defaultStream::output_fd());
      }
    }

    staticBufferStream sbs(buffer, O_BUFLEN, &log);
    first_error->report(&sbs);

    if (log.fd() != defaultStream::output_fd()) {
      close(log.fd());
    }

    log.set_fd(-1);
    log_done = true;
  }

  static bool skip_OnError = false;
  if (!skip_OnError && OnError && OnError[0]) {
    skip_OnError = true;

    out.print_raw_cr("#");
    out.print_raw   ("# -XX:OnError=\"");
    out.print_raw   (OnError);
    out.print_raw_cr("\"");

    char* cmd;
    const char* ptr = OnError;
    while ((cmd = next_OnError_command(buffer, sizeof(buffer), &ptr)) != NULL){
      out.print_raw   ("#   Executing ");
#if defined(LINUX)
      out.print_raw   ("/bin/sh -c ");
#elif defined(SOLARIS)
      out.print_raw   ("/usr/bin/sh -c ");
#endif
      out.print_raw   ("\"");
      out.print_raw   (cmd);
      out.print_raw_cr("\" ..."); 

      fork_and_exec(cmd);
    }

    // done with OnError
    OnError = NULL;
  }

  static bool skip_bug_url = false;
  if (!skip_bug_url) {
    skip_bug_url = true;

    out.print_raw_cr("#");
    out.print_raw_cr("# If you would like to submit a bug report, please visit:");
    out.print_raw   ("#   ");
    out.print_raw_cr(Arguments::java_vendor_url_bug());
    out.print_raw_cr("#");
  }

  // os::abort() will call abort hooks, try it first.
  static bool skip_os_abort = false;
  if (!skip_os_abort) {
    skip_os_abort = true;
    os::abort();
  }

  // if os::abort() doesn't abort, try os::die();
  os::die();
}

