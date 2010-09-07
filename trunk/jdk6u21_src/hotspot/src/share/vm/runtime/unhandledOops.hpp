/*
 * Copyright (c) 2005, Oracle and/or its affiliates. All rights reserved.
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
#ifdef CHECK_UNHANDLED_OOPS

// Detect unhanded oops in VM code

// The design is that when an oop is declared on the stack as a local
// variable, the oop is actually a C++ struct with constructor and
// destructor.  The constructor adds the oop address on a list
// off each thread and the destructor removes the oop.  At a potential
// safepoint, the stack addresses of the local variable oops are trashed
// with a recognizeable value.  If the local variable is used again, it
// will segfault, indicating an unsafe use of that oop.
// eg:
//    oop o;    //register &o on list
//    funct();  // if potential safepoint - causes clear_naked_oops()
//              // which trashes o above.
//    o->do_something();  // Crashes because o is unsafe.
//
// This code implements the details of the unhandled oop list on the thread.
//

class oop;
class Thread;

class UnhandledOopEntry {
 friend class UnhandledOops;
 private:
  oop* _oop_ptr;
  bool _ok_for_gc;
  address _pc;
 public:
  oop* oop_ptr() { return _oop_ptr; }
  UnhandledOopEntry() : _oop_ptr(NULL), _ok_for_gc(false), _pc(NULL) {}
  UnhandledOopEntry(oop* op, address pc) :
                        _oop_ptr(op),   _ok_for_gc(false), _pc(pc) {}
};


class UnhandledOops {
 friend class Thread;
 private:
  Thread* _thread;
  int _level;
  GrowableArray<UnhandledOopEntry> *_oop_list;
  void allow_unhandled_oop(oop* op);
  void clear_unhandled_oops();
  UnhandledOops(Thread* thread);
  ~UnhandledOops();

 public:
  static void dump_oops(UnhandledOops* list);
  void register_unhandled_oop(oop* op, address pc);
  void unregister_unhandled_oop(oop* op);
};

#ifdef _LP64
const intptr_t BAD_OOP_ADDR =  0xfffffffffffffff1;
#else
const intptr_t BAD_OOP_ADDR =  0xfffffff1;
#endif // _LP64
#endif // CHECK_UNHANDLED_OOPS
