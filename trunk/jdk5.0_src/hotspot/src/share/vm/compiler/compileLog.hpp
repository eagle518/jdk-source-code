#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)compileLog.hpp	1.6 04/03/02 17:16:29 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class ciObject;
class ciSymbol;

// CompileLog
//
// An open stream for logging information about activities in a
// compiler thread.  There is exactly one per CompilerThread,
// if the +LogCompilation switch is enabled.
class CompileLog : public xmlStream {
 private:
  const char*   _file;           // name of file where XML goes
  julong        _file_end;       // last good end of file
  intx          _thread_id;      // which compile thread

  stringStream  _context;        // optional, killable context marker
  char          _context_buffer[100];

  char*         _identities;     // array of boolean
  int           _identities_limit;
  int           _identities_capacity;

  CompileLog*   _next;           // static chain of all logs

  static CompileLog* _first;     // head of static chain

  void va_tag(bool push, const char* format, va_list ap);

 public:
  CompileLog(const char* file, FILE* fp, intx thread_id);
  ~CompileLog();

  intx          thread_id()                      { return _thread_id; }
  const char*   file()                           { return _file; }
  stringStream* context()                        { return &_context; }

  void          name(ciSymbol* s);               // name='s'
  void          name(symbolHandle s)             { xmlStream::name(s); }

  // Output an object description, return obj->ident().
  int           identify(ciObject* obj);
  void          clear_identities();

  // virtuals
  virtual void see_tag(const char* tag, bool push);
  virtual void pop_tag(const char* tag);

  // make a provisional end of log mark
  void mark_file_end() { _file_end = out()->count(); }

  // copy all logs to the given stream
  static void finish_log(outputStream* out);
  static void finish_log_on_error(outputStream* out, char *buf, int buflen);
};
