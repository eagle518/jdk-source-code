#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_ValueStack.hpp	1.42 04/04/14 17:27:22 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

class ValueStack: public CompilationResourceObj {
 private:
  IRScope* _scope;                               // the enclosing scope
  bool     _lock_stack;                          // indicates that this ValueStack is for an exception site    
  Values   _locals;                              // the locals
  Values   _stores;                              // the recent StoreLocals, for store elimination
  Values   _stack;                               // the expression stack
  Values   _locks;                               // the monitor stack (holding the locked values)

  Value check(ValueTag tag, Value t) {
    assert(tag == t->type()->tag() || tag == objectTag && t->type()->tag() == addressTag, "types must correspond");
    return t;
  }

  Value check(ValueTag tag, Value t, Value h) {
    assert(h->as_HiWord()->lo_word() == t, "incorrect stack pair");
    return check(tag, t);
  }

 public:
  // creation
  ValueStack(IRScope* scope, int locals_size, int max_stack_size);
  ValueStack(ValueStack* s);

  // merging
  ValueStack* copy();                            // returns a copy of this w/ cleared locals
  ValueStack* copy_locks();                      // returns a copy of this w/ cleared locals and stack
                                                 // Note that when inlining of methods with exception
                                                 // handlers is enabled, this stack may have a
                                                 // non-empty expression stack (size defined by
                                                 // scope()->lock_stack_size())
  void init(ValueStack* s);                      // initializes this w/ cleared locals & phis for stack entries of s
  bool is_same(ValueStack* s);                   // returns true if this & s's types match (w/o checking locals)
  bool is_same_across_scopes(ValueStack* s);     // same as is_same but returns true even if stacks are in different scopes (used for block merging w/inlining)

  // accessors
  IRScope* scope() const                         { return _scope; }
  bool is_lock_stack() const                     { return _lock_stack; }
  Values locks() const                           { return _locks; }
  int locals_size() const                        { return _locals.length(); }
  int stack_size() const                         { return _stack.length(); }
  int locks_size() const                         { return _locks.length(); }
  int max_stack_size() const                     { return _stack.capacity(); }
  bool stack_is_empty() const                    { return _stack.is_empty(); }
  bool no_active_locks() const                   { return _locks.is_empty(); }
  ValueStack* caller_state() const;

  // locals access
  void clear_locals();                           // sets all locals to NULL;
                                                 // clears store elimination table as well

  void kill_local(int i) {
#ifdef ASSERT
    // in debug mode, make sure to erase potentially 'overlapping'
    // entries to make sure we always use the locals array properly
    Value x = _locals.at(i);
    if (x != NULL) {
      if (x->type()->is_double_word()) {
        HiWord* h = x->as_HiWord();
        if (h == NULL) {
          assert(_locals.at(i+1)->as_HiWord()->lo_word() == x, "locals inconsistent");
          _locals.at_put(i+1, NULL);
        } else {
          assert(_locals.at(i-1) == h->lo_word(), "stack inconsistent");
          _locals.at_put(i-1, NULL);
        }
      }
      _locals.at_put(i, NULL);
    }
#else
    // in optimized mode, only kill the specified entry - note that
    // in case of double word entries, the HiWord is undefined
    // and never accessed (unless the program didn't pass the
    // bytecode verifier)
    _locals.at_put(i, NULL);
#endif // ASSERT
  }

  Value load_local(int i) const {
    Value x = _locals.at(i);
    assert(x == NULL || x->as_HiWord() == NULL, "index points to hi word");
    assert(x == NULL || x->type()->is_single_word() || x == _locals.at(i+1)->as_HiWord()->lo_word(), "locals inconsistent");
    return x;
  }

  void store_local(int i, Value x) {
#ifdef ASSERT
    // in debug mode, kill the specified local entry and
    // set the new local including HiWord if necessary
    assert(x->as_HiWord() == NULL, "x must not be HiWord");
    kill_local(i);
    if (x->type()->is_single_word()) {
      _locals.at_put(i, x);
    } else {
      kill_local(i+1);
      _locals.at_put(i, x);
      _locals.at_put(i+1, new HiWord(x));
    }
#else
    // in optimized mode, only set the specified entry - note that
    // in case of double word entries, the HiWord is undefined
    // and never accessed (unless the program didn't pass the
    // bytecode verifier)
    _locals.at_put(i, x);
#endif // ASSERT
  }

  // Support for store elimination; currently separate from the above
  // store_local because of structure of using code.
  void store_local(StoreLocal* x, int bci);
  void clear_store(int index);
  void clear_stores();                           // clears only store elimination table
  void eliminate_stores(int bci);                // eliminates outstanding stores of only current scope
  void eliminate_all_scope_stores(int bci);      // eliminates outstanding stores of this and parent scopes

  // stack access
  Value stack_at_inc(int& i) const {
    Value x = _stack.at(i);
    assert(x->as_HiWord() == NULL, "index points to hi word");
    assert(x->type()->is_single_word() || x == _stack.at(i+1)->as_HiWord()->lo_word(), "stack inconsistent");
    i += x->type()->size();
    return x;
  }

  Value stack_at_dec(int& i) const {
    Value x = _stack.at(--i);
#ifdef ASSERT
    // in debug mode, check that a potential HiWord is setup
    // properly and decrement once more if there is one
    HiWord* h = x->as_HiWord();
    if (h != NULL) {
      x = _stack.at(--i);
      assert(x == h->lo_word(), "stack inconsistent");
    }
#else
    // in optimized mode, no HiWords but NULL is used instead -
    // a NULL entry indicates a high word => decrement once more
    if (x == NULL) x = _stack.at(--i);
#endif // ASSERT
    return x;
  }

  // pinning support
  void pin_stack_locals(int index);
  void pin_stack_fields(ciField* field);
  void pin_stack_indexed(ValueType* type);
  void pin_stack_all(Instruction::PinReason reason = Instruction::PinUnknown);
  void pin_stack_for_state_split();

  // iteration
  void values_do(void f(Value*));

  // untyped manipulation (for dup_x1, etc.)
  void clear_stack()                             { _stack.clear(); }
  void truncate_stack(int size)                  { _stack.trunc_to(size); }
  void raw_push(Value t)                         { _stack.push(t); }
  Value raw_pop()                                { return _stack.pop(); }

  // typed manipulation
  void ipush(Value t)                            { _stack.push(check(intTag    , t)); }
  void fpush(Value t)                            { _stack.push(check(floatTag  , t)); }
  void apush(Value t)                            { _stack.push(check(objectTag , t)); }
  void rpush(Value t)                            { _stack.push(check(addressTag, t)); }
#ifdef ASSERT
  // in debug mode, use HiWord for 2-word values
  void lpush(Value t)                            { _stack.push(check(longTag   , t)); _stack.push(new HiWord(t)); }
  void dpush(Value t)                            { _stack.push(check(doubleTag , t)); _stack.push(new HiWord(t)); }
#else
  // in optimized mode, use NULL for 2-word values
  void lpush(Value t)                            { _stack.push(check(longTag   , t)); _stack.push(NULL); }
  void dpush(Value t)                            { _stack.push(check(doubleTag , t)); _stack.push(NULL); }
#endif // ASSERT

  void push(ValueType* type, Value t) {
    switch (type->tag()) {
      case intTag    : ipush(t); return;
      case longTag   : lpush(t); return;
      case floatTag  : fpush(t); return;
      case doubleTag : dpush(t); return;
      case objectTag : apush(t); return;
      case addressTag: rpush(t); return;
    }
    ShouldNotReachHere();
  }

  Value ipop()                                   { return check(intTag    , _stack.pop()); }
  Value fpop()                                   { return check(floatTag  , _stack.pop()); }
  Value apop()                                   { return check(objectTag , _stack.pop()); }
  Value rpop()                                   { return check(addressTag, _stack.pop()); }
#ifdef ASSERT
  // in debug mode, check for HiWord consistency
  Value lpop()                                   { Value h = _stack.pop(); return check(longTag  , _stack.pop(), h); }
  Value dpop()                                   { Value h = _stack.pop(); return check(doubleTag, _stack.pop(), h); }
#else
  // in optimized mode, ignore HiWord since it is NULL
  Value lpop()                                   { _stack.pop(); return check(longTag  , _stack.pop()); }
  Value dpop()                                   { _stack.pop(); return check(doubleTag, _stack.pop()); }
#endif // ASSERT

  Value pop(ValueType* type) {
    switch (type->tag()) {
      case intTag    : return ipop();
      case longTag   : return lpop();
      case floatTag  : return fpop();
      case doubleTag : return dpop();
      case objectTag : return apop();
      case addressTag: return rpop();
    }
    ShouldNotReachHere();
    return NULL;
  }

  Values* pop_arguments(int argument_size);

  // locks access
  int lock  (IRScope* scope, Value obj);
  int unlock();

  // Inlining support
  ValueStack* push_scope(IRScope* scope);         // "Push" new scope, returning new resulting stack
                                                  // Preserves stack and locks, destroys locals
  ValueStack* pop_scope(bool should_eliminate_stores,
                        int bci);                 // "Pop" topmost scope, returning new resulting stack
                                                  // Preserves stack and locks, destroys locals

  // debugging
  void print()  PRODUCT_RETURN;
  void verify() PRODUCT_RETURN;
};

