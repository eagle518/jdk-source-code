/*
 * Copyright (c) 2005, 2007, Oracle and/or its affiliates. All rights reserved.
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

// HeapDumper is used to dump the java heap to file in HPROF binary format:
//
//  { HeapDumper dumper(true /* full GC before heap dump */);
//    if (dumper.dump("/export/java.hprof")) {
//      ResourceMark rm;
//      tty->print_cr("Dump failed: %s", dumper.error_as_C_string());
//    } else {
//      // dump succeeded
//    }
//  }
//

class HeapDumper : public StackObj {
 private:
  char* _error;
  bool _print_to_tty;
  bool _gc_before_heap_dump;
  elapsedTimer _t;

  // string representation of error
  char* error() const                   { return _error; }
  void set_error(char* error);

  // indicates if progress messages can be sent to tty
  bool print_to_tty() const             { return _print_to_tty; }

  // internal timer.
  elapsedTimer* timer()                 { return &_t; }

 public:
  HeapDumper(bool gc_before_heap_dump) :
    _gc_before_heap_dump(gc_before_heap_dump), _error(NULL), _print_to_tty(false) { }
  HeapDumper(bool gc_before_heap_dump, bool print_to_tty) :
    _gc_before_heap_dump(gc_before_heap_dump), _error(NULL), _print_to_tty(print_to_tty) { }

  ~HeapDumper();

  // dumps the heap to the specified file, returns 0 if success.
  int dump(const char* path);

  // returns error message (resource allocated), or NULL if no error
  char* error_as_C_string() const;

  static void dump_heap()    KERNEL_RETURN;
};
