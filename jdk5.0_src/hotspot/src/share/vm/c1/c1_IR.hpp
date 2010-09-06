#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_IR.hpp	1.89 03/12/23 16:39:06 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// An XHandler is a C1 internal description for an exception handler

class XHandler: public CompilationResourceObj {
 private:
  ciExceptionHandler* _desc;
  BlockBegin*         _entry;

 public:
  // creation
  XHandler(ciExceptionHandler* desc) : _desc(desc), _entry(NULL) {}
  
  void set_entry(BlockBegin* entry) {
    assert(entry->is_set(BlockBegin::exception_entry_flag), "must be an exception handler entry");
    assert(entry->bci() == handler_bci(), "bci's must correspond");
    _entry = entry;
  }

  // accessors
  int beg_bci() const                            { return _desc->start(); }
  int end_bci() const                            { return _desc->limit(); }
  int handler_bci() const                        { return _desc->handler_bci(); }
  int catch_type() const                         { return _desc->catch_klass_index(); }
  ciInstanceKlass* catch_klass() const           { return _desc->catch_klass(); }
  BlockBegin* entry() const                      { return _entry; }
  bool covers(int bci) const                     { return beg_bci() <= bci && bci < end_bci(); }
  int start_pco() const                          { assert(entry()->exception_handler_pco() >= 0,
                                                          "must have generated code for this exception handler");
                                                   return entry()->exception_handler_pco(); }
};


define_array(XHandlerArray, XHandler*)
define_stack(XHandlerList, XHandlerArray)


// XHandlers is the C1 internal list of exception handlers for a method

class XHandlers: public CompilationResourceObj {
 private:
  XHandlerList _list;

 public:
  // creation
  XHandlers(ciMethod* method);

  // accessors
  int number_of_handlers() const                 { return _list.length(); }
  XHandler* handler_at(int i) const              { return _list.at(i); }
  bool has_handlers() const                      { return number_of_handlers() > 0; }
};


// A LocalSlot is a descriptor of a (interpreter) stack slot representing
// a method local variable. A stack slot may be used to hold locals of
// different types at different times, and a LocalSlot keeps track of
// all the different types used for one slot. 

define_array(LocalArray, Local*)
define_stack(LocalList, LocalArray)

class LocalSlot: public CompilationResourceObj {
 private:
  Local* _local[number_of_legal_tags];

 public:
  LocalSlot();

  // accessors
  Local* for_type(ValueType* type, int i, bool create = false) {
    ValueTag tag = type->tag();
    assert(0 <= type->tag() && type->tag() < number_of_legal_tags, "type not valid");
    Local* x = _local[tag];
    if (x == NULL && create) {
      x = new Local(type, i);
      _local[tag] = x;
    }
    assert(x == NULL || x->java_index() == i, "indeces must correspond");
    return x;
  }

  void collect_locals(LocalList* collection);
  void collect_argument_locals(LocalList* collection);
};


define_array(LocalSlotArray, LocalSlot*)
define_stack(LocalSlotList, LocalSlotArray)


class IRScope;
define_array(IRScopeArray, IRScope*)
define_stack(IRScopeList, IRScopeArray)


// An ExceptionScope models the set of exception handlers covering the
// given instruction. It is modeled separately from the IRScope and
// ValueStack since it is copied into each instruction for which
// can_trap() returns true.
class ExceptionScope: public CompilationResourceObj {
 private:
  ExceptionScope*  _caller_scope;                // Exception handlers in parent scope
  XHandlerList*    _handlers;                    // List of exception handlers covering current BCI
  int              _scope_id;                    // Unique ID for each scope, used when building
                                                 // ranges of PCs covered by a given handler
  static int       _exception_scope_count;       

  void init();
  ExceptionScope(ExceptionScope* parent);

 public:
  ExceptionScope();

  // Management of topmost scope
  void      clear();                             // Clear exception handler list in topmost scope
  void      add_handler(XHandler* handler);
  int       length() const;
  XHandler* handler_at(int i) const;
  int       id() const;                                // Unique ID of this ExceptionScope. Used when constructing
                                                 // PC ranges.
  static int invalid_id();                       // An ID that no ExceptionScope has

  // Management of scopes
  ExceptionScope* push_scope();                  // Returns newly-created scope whose parent is this one
  ExceptionScope* pop_scope();                   // Returns parent scope of this one; current scope is
                                                 // unlinked and can be discarded
  ExceptionScope* caller_scope() const;

  bool            equals(ExceptionScope* other) const;

  // Returns whether a particular exception type can be caught.  Also
  // returns true if klass is unloaded or any exception handler
  // classes are unloaded.  type_is_exact indicates whether the throw
  // is known to be exactly that class or it might throw a subtype.
  bool            could_catch(ciInstanceKlass* klass, bool type_is_exact) const;

  // Copying out into instruction
  ExceptionScope* copy();
};

define_array(WordSizeArray, WordSize)
define_stack(WordSizeList,  WordSizeArray)

class Compilation;
class IRScope: public CompilationResourceObj {
 private:
  // hierarchy
  Compilation*  _compilation;                    // the current compilation
  IRScope*      _caller;                         // the caller scope, or NULL
  int           _caller_bci;                     // the caller bci of the corresponding (inlined) invoke, or < 0
  ValueStack*   _caller_state;                   // the caller state, or NULL
  int           _level;                          // the inlining level
  ciMethod*     _method;                         // the corresponding method
  ciLocalMap*   _oop_map;                        // its oop map
  IRScopeList   _callees;                        // the inlined method scopes

  // graph
  XHandlers*    _xhandlers;                      // the exception handlers
  int           _number_of_locks;                // the number of monitor lock slots needed
  bool          _monitor_pairing_ok;             // the monitor pairing info
  BlockBegin*   _start;                          // the start block, successsors are method entries

  // locals
  LocalSlotList _locals;                         // the locals used within this scope
  BitMap        _local_oop_map;                  // set of indices of local oops at a bci (for debug info)
  int           _first_local_name;               // first (lowest) local name allocated in this scope
  int           _last_local_name;                // last (highest) local name allocated in this scope
                                                 // The above two provide an approximation to liveness
                                                 // currently needed in LIR oop map generation
  WordSizeList  _offset_for_local_index;         // Map from index to offset in frame.

  // lock stack management
  int           _lock_stack_size;                // number of expression stack elements which, if present,
                                                 // must be spilled to the stack because of exception
                                                 // handling inside inlined methods

  // deoptimization
  DebugInformationRecorder* _debug_info_recorder;

  // helper functions
  BlockBegin* header_block(BlockBegin* entry, BlockBegin::Flag f);
  BlockBegin* build_graph(Compilation* compilation, int osr_bci);

  LocalList*    argument_locals();               // Provides only argument locals, in increasing order of java index
                                                 // Must only be called for top scope
 public:
  // creation
  IRScope(Compilation* compilation, IRScope* caller, int caller_bci, ciMethod* method, int osr_bci, bool create_graph = false);

  // accessors
  Compilation*  compilation() const              { return _compilation; }
  IRScope*      caller() const                   { return _caller; }
  int           caller_bci() const               { return _caller_bci; }
  ValueStack*   caller_state() const             { return _caller_state; }
  int           level() const                    { return _level; }
  ciMethod*     method() const                   { return _method; }
  ciLocalMap*   oop_map() const                  { return _oop_map; }
  int           max_stack() const;               // NOTE: expensive
  int           lock_stack_size() const          { return _lock_stack_size; }

  // mutators
  // Needed because caller state is not ready at time of IRScope construction
  void          set_caller_state(ValueStack* state) { _caller_state = state; }
  // Needed because caller state changes after IRScope construction.
  // Computes number of expression stack elements whose state must be
  // preserved in the case of an exception; these may be seen by
  // caller scopes. Zero when inlining of methods containing exception
  // handlers is disabled, otherwise a conservative approximation.
  void          compute_lock_stack_size();

  // hierarchy
  bool          is_top_scope() const             { return _caller == NULL; }
  void          add_callee(IRScope* callee)      { _callees.append(callee); }
  int           number_of_callees() const        { return _callees.length(); }
  IRScope*      callee_no(int i) const           { return _callees.at(i); }
  int           top_scope_bci() const;

  // accessors, graph
  bool          is_valid() const                 { return start() != NULL; }
  XHandlers*    xhandlers() const                { return _xhandlers; }
  int           number_of_locks() const          { return _number_of_locks; }
  void          set_min_number_of_locks(int n)   { if (n > _number_of_locks) _number_of_locks = n; }
  bool          monitor_pairing_ok() const       { return _monitor_pairing_ok; }
  BlockBegin*   start() const                    { return _start; }

  // accessors, locals

  Local* local_at(ValueType* type, int i, bool create = false) {
    LocalSlot* s = _locals[i];
    if (s == NULL) {
      if (!create) return NULL;
      s = new LocalSlot();
      _locals[i] = s;
    }
    return s->for_type(type, i, create);
  }

  LocalList*    locals();
  WordSize      allocate_locals(WordSize start_offset,
                                WordSizeList* local_name_to_offset_map);
  // returns the offset the L<index>
  WordSize      offset_for_local_index(int index) {
    return _offset_for_local_index.at(index);
  }
  enum { BAD_LOCAL_OFFSET = 0xbaad0001 };

  // Provides an approximation to liveness needed by the LIR oop map
  // generator. Can go away when the new register allocator is in place.
  bool          local_name_is_live_in_scope(int local_name);
  // Again only needed by the LIR oop map generator; can go away once
  // new register allocator is in place. Provides names of first and
  // last valid locals (inclusive, i.e., first_local_name() <= i <=
  // last_local_name()), or -1 for both if there are no locals in this
  // scope.
  int           first_local_name()               { return _first_local_name;  }
  int           last_local_name()                { return _last_local_name;   }

  void          clear_local_oop_map()            { _local_oop_map.clear(); }
  void          set_local_is_oop(int index)      { _local_oop_map.set_bit(index); }
  bool          local_is_oop(int index)          { return _local_oop_map.at(index); }

  // deoptimization
  void add_dependent(ciInstanceKlass* klass, ciMethod* method) { 
    assert(DeoptC1, "need debug information");
    _debug_info_recorder->add_dependent(klass, method); 
  }
};


class Loop;
class LoopList;


class IR: public CompilationResourceObj {
 private:
  Compilation*     _compilation;                 // the current compilation
  IRScope*         _top_scope;                   // the root of the scope hierarchy
  WordSize         _locals_size;                 // the space required for all locals
  LoopList*        _loops;                       // the 'relevant' loops of the IR
  BlockList*       _code;                        // the blocks in code generation order w/ use counts

  int              _local_name_base;             // current name if allocating block of locals for new inlined scope
  WordSizeList*    _local_name_to_offset_map;    // name-to-offset map, computed in compute_locals_size

  WordSize         _highest_used_offset;         // highest-used offset in frame, needed only for caching of locals

  void iterate_and_set_weight(boolArray& mark, BlockBegin* b, BlockList* blocks, int d) const;

 public:
  // creation
  IR(Compilation* compilation, ciMethod* method, int osr_bci);

  // accessors
  bool             is_valid() const              { return top_scope()->is_valid(); }
  Compilation*     compilation() const           { return _compilation; }
  IRScope*         top_scope() const             { return _top_scope; }
  int              number_of_locks() const       { return top_scope()->number_of_locks(); }
  ciMethod*        method() const                { return top_scope()->method(); }
  BlockBegin*      start() const                 { return top_scope()->start(); }
  BlockBegin*      std_entry() const             { return start()->end()->as_Base()->std_entry(); }
  BlockBegin*      osr_entry() const             { return start()->end()->as_Base()->osr_entry(); }
  WordSize         locals_size() const           { return _locals_size; }
  int              locals_size_in_words() const  { return in_words(_locals_size); }
  LoopList*        loops() const                 { return _loops; }
  BlockList*       code() const                  { return _code; }
  int              max_stack() const             { return top_scope()->max_stack(); } // expensive
  // Only valid after compute_locals_size() has been called.
  // This routine is needed for caching of locals as well as LIR oop
  // map generation, but can go away once the new register allocator
  // is in place.
  WordSizeList* local_name_to_offset_map() const { return _local_name_to_offset_map; }
  // Other routines needed only for caching of locals
  void             notice_used_offset(WordSize offset);
  WordSize         highest_used_offset() const;

  // ir manipulation
  void optimize();
  // Note: there are currently some dependencies in the code
  // generators that this allocates local names in increasing order
  // starting at index 0, for the passing and referencing of
  // parameters in native methods, OSR methods, etc.
  int  allocate_local_name();
  int  local_name_base() const                   { return _local_name_base; }
  void compute_locals_size();
  void compute_loops();
  void compute_code();

  // iteration
  void iterate_preorder   (BlockClosure* closure);
  void iterate_postorder  (BlockClosure* closure);

  // debugging
  void print(bool cfg_only, bool live_only = false) PRODUCT_RETURN;
  void verify()                                  PRODUCT_RETURN;
};
