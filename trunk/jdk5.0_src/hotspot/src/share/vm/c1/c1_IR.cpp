#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_IR.cpp	1.146 04/04/14 17:27:26 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_c1_IR.cpp.incl"


// Implementation of XHandlers
//
// Note: This code could eventually go away if we are
//       just using the ciExceptionHandlerStream.

XHandlers::XHandlers(ciMethod* method) : _list(method->exception_table_length()) {
  ciExceptionHandlerStream s(method);
  while (!s.is_done()) {
    _list.append(new XHandler(s.handler()));
    s.next();
  }
  assert(s.count() == method->exception_table_length(), "exception table lengths inconsistent");
}


// Implementation of LocalSlot

LocalSlot::LocalSlot() {
  for (int i = 0; i < number_of_legal_tags; i++) _local[i] = NULL;
}


void LocalSlot::collect_locals(LocalList* collection) {
  for (int i = 0; i < number_of_legal_tags; i++) {
    Local* x = _local[i];
    if (x != NULL) collection->push(x);
  }
}


void LocalSlot::collect_argument_locals(LocalList* collection) {
  for (int i = 0; i < number_of_legal_tags; i++) {
    Local* x = _local[i];
    if (x != NULL && x->is_incoming_argument()) collection->push(x);
  }
}


// Implementation of ExceptionScope
int ExceptionScope::_exception_scope_count = -1;

void ExceptionScope::init() {
  _caller_scope = NULL;
  _handlers     = new XHandlerList(5);
  _scope_id     = ++_exception_scope_count;
}


ExceptionScope::ExceptionScope(ExceptionScope* parent) {
  init();
  _caller_scope = parent;
}


ExceptionScope::ExceptionScope() {
  init();
}


void ExceptionScope::clear() {
  _handlers->clear();
}


void ExceptionScope::add_handler(XHandler* handler) {
  _handlers->push(handler);
}


int ExceptionScope::length() const {
  return _handlers->length();
}


XHandler* ExceptionScope::handler_at(int i) const {
  return (*_handlers)[i];
}


int ExceptionScope::id() const {
  return _scope_id;
}


int ExceptionScope::invalid_id() {
  return -1;
}


ExceptionScope* ExceptionScope::push_scope() {
  return new ExceptionScope(this);
}


ExceptionScope* ExceptionScope::pop_scope() {
  assert(_caller_scope != NULL, "must have parent");
  return _caller_scope;
}


ExceptionScope* ExceptionScope::caller_scope() const {
  return _caller_scope;
}


bool ExceptionScope::equals(ExceptionScope* other) const {
  if (other == NULL) return false;

  if (id() != other->id()) {
    return false;
  }

  if (length() != other->length()) {
    return false;
  }
  
  for (int i = 0; i < length(); i++) {
    if (handler_at(i) != other->handler_at(i)) {
      return false;
    }
  }
  
  return true;
}


bool ExceptionScope::could_catch(ciInstanceKlass* klass, bool type_is_exact) const {
  // the type is unknown so be conservative
  if (!klass->is_loaded()) {
    return true;
  }

  for (int i = 0; i < length(); i++) {
    XHandler* handler = handler_at(i);
    if (handler->catch_type() == 0) {
      // catch of ANY
      return true;
    }
    ciInstanceKlass* handler_klass = handler->catch_klass();
    // if it's unknown it might be catchable
    if (!handler_klass->is_loaded()) {
      return true;
    }
    // if the throw type is definitely a subtype of the catch type
    // then it can be caught.
    if (klass->is_subtype_of(handler_klass)) {
      return true;
    }
    if (!type_is_exact) {
      // If the type isn't exactly known then it can also be caught by
      // catch statements where the inexact type is a subtype of the
      // catch type.
      // given: foo extends bar extends Exception
      // throw bar can be caught by catch foo, catch bar, and catch
      // Exception, however it can't be caught by any handlers without
      // bar in its type hierarchy.
      if (handler_klass->is_subtype_of(klass)) {
        return true;
      }
    }
  }
  if (caller_scope() != NULL) {
    return caller_scope()->could_catch(klass, type_is_exact);
  } else {
    return false;
  }
}


ExceptionScope* ExceptionScope::copy() {
  ExceptionScope* cur;
  if (_caller_scope != NULL) {
    cur = _caller_scope->copy()->push_scope();
  } else {
    cur = new ExceptionScope();
  }
  cur->_scope_id = _scope_id;
  for (int i = 0; i < length(); i++) {
    cur->add_handler(handler_at(i));
  }
  return cur;
}


// Implementation of IRScope

BlockBegin* IRScope::header_block(BlockBegin* entry, BlockBegin::Flag f) {
  if (entry == NULL) return NULL;
  assert(entry->is_set(f), "entry/flag mismatch");
  // create header block
  BlockBegin* h = new BlockBegin(entry->bci());
  BlockEnd* g = new Goto(entry, false);
  h->set_next(g, entry->bci());
  h->set_end(g);
  h->set(f);
  // setup header block end state
  ValueStack* s = entry->state()->copy(); // can use copy since stack is empty (=> no phis)
  assert(s->stack_is_empty(), "must have empty stack at entry point");
  g->set_state(s);
  return h;
}


BlockBegin* IRScope::build_graph(Compilation* compilation, int osr_bci) {
  // determine entry points and bci2block mapping
  BlockListBuilder blb(this, osr_bci);
  assert(osr_bci < 0 || blb.osr_entry() != NULL, "osr entry must exist for osr compile");
  // create & setup initial state for std entry
  { ValueStack* state = new ValueStack(this, method()->max_locals(), method()->max_stack());
    if (method()->is_synchronized()) {
      state->lock(this, NULL);
    }
    if (!method()->is_static()) {
      // we should always see the receiver
      local_at(objectType, 0, true)->set_is_incoming_argument();
    }
    {
      // Currently, in order to properly allocate local names and
      // offsets (in allocate_locals()) we need to ensure that all of
      // the incoming arguments, including the receiver, in the top
      // scope are assigned names congruent to their Java indices. We
      // do this by touching and marking the arguments once here, then
      // testing the is_incoming_argument() flag in a couple of places
      // later.
      int idx = (method()->is_static() ? 0 : 1);
      ciSignature* sig = method()->signature();
      for (int i = 0; i < sig->count(); i++) {
        ciType* type = sig->type_at(i);
        BasicType basic_type = type->basic_type();

        // don't allow T_ARRAY to propagate into local types
        if (basic_type == T_ARRAY) basic_type = T_OBJECT;
        ValueType* vt = as_ValueType(basic_type);
        local_at(vt, idx, true)->set_is_incoming_argument();
        idx += type->size();
      }
    }
    blb.std_entry()->set_state(state);
  }
  // complete graph
  GraphBuilder gb(compilation, this, blb.bci2block(), blb.std_entry());
  NOT_PRODUCT(if (PrintValueNumbering && Verbose) gb.print_stats());
  if (gb.bailed_out()) return NULL;
  assert(osr_bci < 0 || blb.osr_entry()->is_set(BlockBegin::was_visited_flag), "osr entry must have been visited for osr compile");
  // for osr compile, bailout if some requirements are not fulfilled
  if (osr_bci >= 0) {
    // check if osr entry point has empty stack - we cannot handle non-empty stacks at osr entry points
    if (!blb.osr_entry()->state()->stack_is_empty()) return NULL; // bailout
  }
  // setup start block (root for the IR graph)
  BlockBegin* start = new BlockBegin(0);
  { Base* base =
      new Base(
        header_block(blb.std_entry(), BlockBegin::std_entry_flag),
        header_block(blb.osr_entry(), BlockBegin::osr_entry_flag)
      );
    start->set_next(base, 0);
    start->set_end(base);
    // create & setup state for start block
    ValueStack* state = new ValueStack(this, method()->max_locals(), method()->max_stack());
    if (method()->is_synchronized()) {
      state->lock(this, NULL);
    }
    start->set_state(state);
    base->set_state(state->copy());
    // setup states for header blocks
    base->std_entry()->join(state);
    if (osr_bci >= 0) base->osr_entry()->join(state);
  }
  // done
  return start;
}


IRScope::IRScope(Compilation* compilation, IRScope* caller, int caller_bci, ciMethod* method, int osr_bci, bool create_graph)
: _callees(2)
, _compilation(compilation)
, _locals(method->max_locals(), NULL)
, _offset_for_local_index(method->max_locals(), in_WordSize(BAD_LOCAL_OFFSET))
, _local_oop_map(MAX2(method->max_locals(), method->arg_size()))
, _lock_stack_size(0)
{
  _caller             = caller;
  _caller_bci         = caller == NULL ? -1 : caller_bci;
  _caller_state       = NULL; // Must be set later if needed
  _level              = caller == NULL ?  0 : caller->level() + 1;
  _method             = method;
  _xhandlers          = new XHandlers(method);
  _number_of_locks    = 0;
  _monitor_pairing_ok = method->has_balanced_monitors();
  _start              = NULL;
  _debug_info_recorder = compilation->debug_info_recorder();
  if (method->is_abstract() || method->is_native()) {
    _oop_map = NULL;
  } else {
    _oop_map = method->all_oop_maps();
  }

  assert(method->holder()->is_loaded() , "method holder must be loaded");

  // build graph if monitor pairing is ok
  if (create_graph && monitor_pairing_ok()) _start = build_graph(compilation, osr_bci);
}


int IRScope::max_stack() const {
  int my_max = method()->max_stack();
  int callee_max = 0;
  for (int i = 0; i < number_of_callees(); i++) {
    callee_max = MAX2(callee_max, callee_no(i)->max_stack());
  }
  return my_max + callee_max;
}


void IRScope::compute_lock_stack_size() {
  if (!InlineMethodsWithExceptionHandlers) {
    _lock_stack_size = 0;
    return;
  }

  // Figure out whether we have to preserve expression stack elements
  // for parent scopes, and if so, how many
  IRScope* cur_scope = this;
  while (cur_scope != NULL && !cur_scope->xhandlers()->has_handlers()) {
    cur_scope = cur_scope->caller();
  }
  _lock_stack_size = (cur_scope == NULL ? 0 :
                      (cur_scope->caller_state() == NULL ? 0 :
                       cur_scope->caller_state()->stack_size()));
}


LocalList* IRScope::argument_locals() {
  assert(is_top_scope(), "should only call for top scope");
  int sz = method()->arg_size();
  LocalList* collection = new LocalList(sz);
  assert(_locals.length() >= sz, "check");
  for (int i = 0; i < sz; i++) {
    LocalSlot* s = _locals[i];
    // High words of non-overlapping doubleword arguments will still be null
    if (s != NULL) s->collect_argument_locals(collection);
  }
  return collection;
}


LocalList* IRScope::locals() {
  LocalList* collection = new LocalList(method()->max_locals() + 4); // add some extra, in case we have overlap
  const int n = _locals.length();
  for (int i = 0; i < n; i++) {
    LocalSlot* s = _locals[i];
    if (s != NULL) s->collect_locals(collection);
  }
  return collection;
}


WordSize IRScope::allocate_locals(WordSize start_offset, WordSizeList* local_name_to_offset_map) {
  // collect all locals of this scope (w/o subscopes)
  // note that one local slot may hold more than one local (overlap)
  // if the slot has been used to store locals of different type
  LocalList collection = *locals();
  const int n = collection.length();

  // make sure local names have not been set yet
  debug_only({ for (int i = 0; i < n; i++) assert(!collection[i]->has_local_name(), "local name must not be set yet"); })

  WordSize max_offset = is_top_scope() ? in_WordSize(method()->arg_size()) : start_offset;

  const WordSize bad_offset = in_WordSize(BAD_LOCAL_OFFSET);
  const WordSize one_word = in_WordSize(1);
  const WordSize two_words = in_WordSize(2);

  int first_local_name = compilation()->hir()->local_name_base();
  int last_local_name = first_local_name - 1;
  bool first = true;

  // Note: this code is duplicated and it would be nice to refactor it
  // but it's pretty confusing as it is and putting it in a
  // separate method only makes it worse.
  if (is_top_scope()) {
    // Assign names to argument locals first. These must always be
    // present in the local collection and must match the Java indices.
    LocalList args = *argument_locals();
    int sz = args.length();
    for (int i = 0; i < sz; i++) {
      Local* x = args[i];
      assert(x->is_incoming_argument(), "check");
      WordSize cur_offset = start_offset + in_WordSize(x->java_index());
      x->set_local_name(compilation()->hir()->allocate_local_name());
      assert(x->local_name() == x->java_index(), "must allocate names for arguments before any other locals");
      last_local_name = x->local_name();
      x->set_offset(cur_offset);
      local_name_to_offset_map->at_put_grow(x->local_name(), x->offset(), bad_offset);
      if (x->type()->is_double_word()) {
        x->set_hi_word_local_name(compilation()->hir()->allocate_local_name());
        assert(x->hi_word_local_name() == 1 + x->java_index(), "must allocate names for arguments before any other locals");
        // Indicate for assertion purposes that offset is not set for
        // hi word, to hopefully have better flexibility in
        // implementing LP64 more cleanly in the future
        local_name_to_offset_map->at_put_grow(x->hi_word_local_name(), in_WordSize(-1), bad_offset);
      }
      _offset_for_local_index.at_put(x->java_index(), cur_offset);
      if (x->type()->is_double_word()) {
        _offset_for_local_index.at_put(x->java_index() + 1, cur_offset + one_word);
      }
    }
  }

  if (ForceStackAlignment) {
    // make sure to start on an aligned boundary
    // the first non-parameter is properly aligned
    WordSize unused_offset = bad_offset;
    WordSize offset = max_offset;
    if ((in_words(max_offset) - compilation()->method()->arg_size()) % 2 == 1) {
      unused_offset = max_offset;
      offset = max_offset + one_word;
    }
    
    // lay out the double word locals
    int i;
    for (i = 0; i < n; i++) {
      Local* x = collection[i];
      assert(0 <= x->java_index() && x->java_index() < method()->max_locals(), "illegal index");
      
      if (!x->is_incoming_argument() && x->type()->is_double_word()) {
        assert(!x->has_offset(), "no");
        WordSize offset_lo = _offset_for_local_index.at(x->java_index());
        WordSize offset_hi = _offset_for_local_index.at(x->java_index() + 1);
        if (offset_lo == bad_offset && offset_hi == bad_offset) {
          // assign double word entries first
          _offset_for_local_index.at_put(x->java_index(), offset);
          _offset_for_local_index.at_put(x->java_index() + 1, offset + one_word);
        } else if (offset_lo == bad_offset) {
          // half of it is initialized.  make the other half match up
          _offset_for_local_index.at_put(x->java_index(), offset_hi - one_word);
        } else if (offset_hi == bad_offset) {
          _offset_for_local_index.at_put(x->java_index() + 1, offset_lo + one_word);
        }
        offset_lo = _offset_for_local_index.at(x->java_index());
        offset_hi = _offset_for_local_index.at(x->java_index() + 1);
        assert(offset_hi == offset_lo + one_word, "must be next to each other");
        offset = MAX2(offset, offset_lo + two_words);
      }
    }

    // lay out the single word locals
    for (i = 0; i < n; i++) {
      Local* x = collection[i];
      assert(0 <= x->java_index() && x->java_index() < method()->max_locals(), "illegal index");
      
      // check if this local position has already been assigned an offset
      if (!x->is_incoming_argument() && x->type()->is_single_word()) {
        assert(!x->has_offset(), "no");
        WordSize o = _offset_for_local_index.at(x->java_index());
        if (o == bad_offset) {
          if (unused_offset == bad_offset) {
            _offset_for_local_index.at_put(x->java_index(), offset);
          } else {
            _offset_for_local_index.at_put(x->java_index(), unused_offset);
            unused_offset = bad_offset;
          }
          offset = MAX2(offset, _offset_for_local_index.at(x->java_index()) + one_word);
        }
      }
    }
    max_offset = offset;
  } else {
    for (int i = 0; i < n; i++) {
      Local* x = collection[i];
      if (!x->is_incoming_argument()) {
        WordSize cur_offset = start_offset + in_WordSize(x->java_index());
        _offset_for_local_index.at_put(x->java_index(), cur_offset);
        if (x->type()->is_double_word()) {
          _offset_for_local_index.at_put(x->java_index() + 1, cur_offset + one_word);
        }
      }
    }
  }

  // assign offsets corresponding to indices
  {
    for (int i = 0; i < n; i++) {
      Local* x = collection[i];
      assert(!x->is_incoming_argument() || is_top_scope(), "should only see incoming argument locals in top scope");
      if (!x->is_incoming_argument()) {
        x->set_local_name(compilation()->hir()->allocate_local_name());
        last_local_name = x->local_name();
        x->set_offset(_offset_for_local_index.at(x->java_index()));
        local_name_to_offset_map->at_put_grow(x->local_name(), x->offset(), bad_offset);
        if (x->type()->is_double_word()) {
          assert(x->offset() + one_word == _offset_for_local_index.at(x->java_index() + 1),
                 "two halves should be contiguous");
          x->set_hi_word_local_name(compilation()->hir()->allocate_local_name());
          // Indicate for assertion purposes that offset is not set for
          // hi word, to hopefully have better flexibility in
          // implementing LP64 more cleanly in the future
          local_name_to_offset_map->at_put_grow(x->hi_word_local_name(), in_WordSize(-1), bad_offset);
        }
        WordSize local_size = x->type()->is_single_word() ? one_word : two_words;
        max_offset = in_WordSize(MAX2(in_words(max_offset), in_words(x->offset() + local_size)));
      }
    }
  }

  _first_local_name = first_local_name;
  _last_local_name  = last_local_name;

  // make sure all local names have been set now
#ifdef ASSERT
  {
    for (int j = 0; j < method()->max_locals(); j++) {
      offset_for_local_index(j);
    }
    for (int i = 0; i < n; i++) {
      assert(collection[i]->has_local_name(), "local name must be set now");
      assert(collection[i]->offset() == offset_for_local_index(collection[i]->java_index()), "must match");
    }
  }
#endif // ASSERt

  // allocate space for subscopes
  WordSize size = max_offset;
  { const int n = number_of_callees();
    for (int i = 0; i < n; i++) {
      size = in_WordSize(MAX2(in_words(size), in_words(callee_no(i)->allocate_locals(max_offset, local_name_to_offset_map))));
    }
  }
  compilation()->hir()->notice_used_offset(size);
  return size;
}


bool IRScope::local_name_is_live_in_scope(int local_name) {
  return (_first_local_name <= local_name && local_name <= _last_local_name);
}


int IRScope::top_scope_bci() const {
  assert(!is_top_scope(), "no correct answer for top scope possible");
  const IRScope* scope = this;
  while (!scope->caller()->is_top_scope()) {
    scope = scope->caller();
  }
  return scope->caller_bci();
}


// Implementation of IR

IR::IR(Compilation* compilation, ciMethod* method, int osr_bci) : _locals_size(in_WordSize(-1)), _highest_used_offset(in_WordSize(-1)) {
  // initialize data structures
  ValueType::initialize();
  Instruction::initialize();
  BlockBegin::initialize();
  GraphBuilder::initialize();
  // setup IR fields
  _compilation = compilation;
  _top_scope   = new IRScope(compilation, NULL, -1, method, osr_bci, true);
  _loops       = NULL;
  _code        = NULL;
  _local_name_base = 0;
  _local_name_to_offset_map = NULL;
}


void IR::optimize() {
  Optimizer opt(this);
  if (DoCEE) {
    opt.eliminate_conditional_expressions();
    if (PrintCFG || PrintCFG1) { tty->print_cr("CFG after CEE"); print(true); }
    if (PrintIR  || PrintIR1 ) { tty->print_cr("IR after CEE"); print(false); }
  }
  if (EliminateBlocks) {
    opt.eliminate_blocks();
    if (PrintCFG || PrintCFG1) { tty->print_cr("CFG after block elimination"); print(true); }
    if (PrintIR  || PrintIR1 ) { tty->print_cr("IR after block elimination"); print(false); }
  }
  if (EliminateNullChecks) {
    opt.eliminate_null_checks();
    if (PrintCFG || PrintCFG1) { tty->print_cr("CFG after null check elimination"); print(true); }
    if (PrintIR  || PrintIR1 ) { tty->print_cr("IR after null check elimination"); print(false); }
  }
}


void IR::notice_used_offset(WordSize offset) {
  if (offset > _highest_used_offset) {
    _highest_used_offset = offset;
  }
}


WordSize IR::highest_used_offset() const {
  return _highest_used_offset;
}


int IR::allocate_local_name() {
  int cur = _local_name_base;
  ++_local_name_base;
  return cur;
}


void IR::compute_locals_size() {
  WordSizeList* local_name_to_offset_map = new WordSizeList();
  _locals_size = top_scope()->allocate_locals(in_WordSize(0), local_name_to_offset_map);
  _local_name_to_offset_map = local_name_to_offset_map;
}


void IR::compute_loops() {
  if (ComputeLoops && method()->has_loops()) {
    LoopFinder l(this, BlockBegin::number_of_blocks());
    _loops = l.compute_loops(CacheCallFreeLoopsOnly);
  }
}


void IR::iterate_and_set_weight(boolArray& mark, BlockBegin* b, BlockList* blocks, int d) const {
  if (!mark.at(b->block_id())) {
    mark.at_put(b->block_id(), true);
    b->set_weight(d);
    blocks->append(b);
    BlockEnd* e = b->end();
    int i;
    for (i = b->number_of_exception_handlers() - 1; i >= 0; i--) iterate_and_set_weight(mark, b->exception_handler_at(i), blocks, d + 1); 
    for (i = e->number_of_sux               () - 1; i >= 0; i--) iterate_and_set_weight(mark, e->sux_at              (i), blocks, d + 1); 
  }
}


class UseCountComputer: public AllStatic {
 private:
  static void update_use_count(Value* n) {
    // Local instructions and Phis for expression stack values at the
    // start of basic blocks are not added to the instruction list
    if ((*n)->bci() == -1 && (*n)->as_Local() == NULL && (*n)->as_Phi() == NULL) {
      assert(false, "a node was not appended to the graph");
      Compilation::current_compilation()->bailout("a node was not appended to the graph");
    }
    // use n's input if not visited before
    if (!(*n)->is_pinned() && !(*n)->has_uses()) {
      // note: a) if the instruction is pinned, it will be handled by compute_use_count
      //       b) if the instruction has uses, it was touched before
      //       => in both cases we don't need to update n's values
      uses_do(n);
    }
    // use n
    (*n)->_use_count++;
  }

  static void uses_do(Value* n) {
    (*n)->input_values_do(update_use_count);
    // special handling for some instructions
    if ((*n)->as_BlockEnd() != NULL) {
      // note on BlockEnd:
      //   must 'use' the stack only if the method doesn't
      //   terminate, however, in those cases stack is empty
      (*n)->state_values_do(update_use_count);
    }
  }

  static void basic_compute_use_count(BlockBegin* b) {
    for (Instruction* n = b; n != NULL; n = n->next()) {
      if (n->is_pinned()) uses_do(&n);
    }
  }

  static bool updated_pinning(BlockBegin* b) {
    bool updated = false;
    for (Instruction* n = b; n != NULL; n = n->next()) {
      if (!n->is_pinned() && !n->has_uses() && n->can_trap()) { n->pin(); updated = true; }
    }
    return updated;
  }

  static void clear_use_count(BlockBegin* b) {
    // clear use counts of phi instructions
    ValueStack* s = b->state();
    for (int i = 0; i < s->stack_size();) s->stack_at_inc(i)->_use_count = 0;
    // clear use counts of all other instructions
    for (Instruction* n = b; n != NULL; n = n->next()) n->_use_count = 0;
  }

 public:
  static void compute_use_count(BlockBegin* b) {
    // determine use count given the current pinning information
    // note: all use counts are cleared in the beginning
    basic_compute_use_count(b);
    // instructions that are never used but that can trap must
    // not be eliminated => make sure code is generated for'em
    // by pinning them explicitly (was bug - gri 11/10/99)
    if (updated_pinning(b)) {
      // set of pinned instructions was updated
      // => need to recompute use count information
      // note: must clear use counts first
      clear_use_count(b);
      basic_compute_use_count(b);
    }
  }
};


static int cmp(BlockBegin** a, BlockBegin** b) {
  return (*a)->weight() - (*b)->weight();
}


class SuxAndWeightAdjuster: public BlockClosure {
 private:
  BlockBegin* _prev;

 public:
  SuxAndWeightAdjuster()                          { _prev = NULL; }

  virtual void block_do(BlockBegin* block) {
    if (_prev == NULL) {
      // set weight for first block
      block->_weight = 0;
    } else {
      // set weight for current block
      block->_weight = _prev->weight() + 1;
      // swap successors if default successor is not the block immediately following
      BlockEnd* end = _prev->end();
      if (end->number_of_sux() == 2 && end->default_sux() != block && end->sux_index(block) >= 0) {
             if (end->as_If          () != NULL) end->as_If          ()->swap_sux();
        else if (end->as_IfInstanceOf() != NULL) end->as_IfInstanceOf()->swap_sux();
      }
    }
    _prev = block;
  }
};

void IR::compute_code() {
  assert(is_valid(), "IR must be valid");
  const int n = BlockBegin::number_of_blocks();
  // collect all blocks and set weight depending on distance from start
  // note: we need a special iterator because the standard ones don't
  //       compute the distance from start.
  BlockList* blocks = new BlockList(n);
  { boolArray mark(n, false);
    iterate_and_set_weight(mark, start(), blocks, 0);
  }
  // now we have all blocks => sort them according to weight
  blocks->sort(cmp);
  // swap successors of If's if default successor is not the block immediately following
  // and adjust weights so they are increasing by 1 and starting with 0 for first block
  { SuxAndWeightAdjuster swa;
    blocks->iterate_forward(&swa);
  }
  // compute use counts
  blocks->blocks_do(UseCountComputer::compute_use_count);
  // done
  _code = blocks;
}


void IR::iterate_preorder(BlockClosure* closure) {
  assert(is_valid(), "IR must be valid");
  start()->iterate_preorder(closure);
}


void IR::iterate_postorder(BlockClosure* closure) {
  assert(is_valid(), "IR must be valid");
  start()->iterate_postorder(closure);
}


#ifndef PRODUCT
class BlockPrinter: public BlockClosure {
 private:
  InstructionPrinter* _ip;
  bool                _cfg_only;
  bool                _live_only;

 public:
  BlockPrinter(InstructionPrinter* ip, bool cfg_only, bool live_only = false) {
    _ip       = ip;
    _cfg_only = cfg_only;
    _live_only = live_only;
  }

  virtual void block_do(BlockBegin* block) {
    if (_cfg_only) {
      _ip->print_instr(block); tty->cr();
    } else {
      block->print_block(*_ip, _live_only);
    }
  }
};


void IR::print(bool cfg_only, bool live_only) {
  if (is_valid()) {
    InstructionPrinter ip;
    BlockPrinter bp(&ip, cfg_only, live_only);
    start()->iterate_preorder(&bp);
  } else {
    tty->print_cr("invalid IR");
  }
  tty->cr();
}


void IR::verify() {
  Unimplemented();
}
#endif // PRODUCT

