#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_Instruction.hpp	1.181 04/06/03 18:15:04 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Predefined classes
class ciField;
class Item;
class ValueStack;
class InstructionPrinter;
class IRScope;
class LocalMapping;
class LIR_OprDesc;
typedef LIR_OprDesc* LIR_Opr;


// Instruction class hierarchy
//
// All leaf classes in the class hierarchy are concrete classes
// (i.e., are instantiated). All other classes are abstract and
// serve factoring.

class Instruction;
#ifdef ASSERT
class   HiWord;
#endif // ASSERT
class   Phi;
class   Local;
class   Constant;
class   AccessLocal;
class     LoadLocal;
class     StoreLocal;
class   AccessField;
class     LoadField;
class     StoreField;
class   AccessArray;
class     ArrayLength;
class     AccessIndexed;
class       LoadIndexed;
class       StoreIndexed;
class   CachingChange;
class   NegateOp;
class   Op2;
class     ArithmeticOp;
class     ShiftOp;
class     LogicOp;
class     CompareOp;
class     IfOp;
class   Convert;
class   NullCheck;
class   StateSplit;
class     Invoke;
class     NewInstance;
class     NewArray;
class       NewTypeArray;
class       NewObjectArray;
class       NewMultiArray;
class     TypeCheck;
class       CheckCast;
class       InstanceOf;
class     AccessMonitor;
class       MonitorEnter;
class       MonitorExit;
class     Intrinsic;
class     BlockBegin;
class     BlockEnd;
class       Goto;
class       If;
class       IfInstanceOf;
class       Switch;
class         TableSwitch;
class         LookupSwitch;
class       Return;
class       Throw;
class       Base;
class   UnsafeOp;
class     UnsafeRawOp;
class       UnsafeGetRaw;
class       UnsafePutRaw;
class     UnsafeObjectOp;
class       UnsafeGetObject;
class       UnsafePutObject;

// A Value is a reference to the instruction creating the value
typedef Instruction* Value;
define_array(ValueArray, Value)
define_stack(Values, ValueArray)


// BlockClosure is the base class for block traversal/iteration.

class BlockClosure: public CompilationResourceObj {
 public:
  virtual void block_do(BlockBegin* block)       = 0;
};


// Some array and list classes
define_array(BasicTypeArray, BasicType)
define_stack(BasicTypeList, BasicTypeArray)

define_array(BlockBeginArray, BlockBegin*)
define_stack(_BlockList, BlockBeginArray)

class BlockList: public _BlockList {
 public:
  BlockList(): _BlockList() {}
  BlockList(const int size): _BlockList(size) {}
  BlockList(const int size, BlockBegin* init): _BlockList(size, init) {}

  void iterate_forward(BlockClosure* closure);
  void iterate_backward(BlockClosure* closure);
  void blocks_do(void f(BlockBegin*));
  void values_do(void f(Value*));
  void print(bool cfg_only = false) PRODUCT_RETURN;
};


// InstructionVisitors provide type-based dispatch for instructions.
// For each concrete Instruction class X, a virtual function do_X is
// provided. Functionality that needs to be implemented for all classes
// (e.g., printing, code generation) is factored out into a specialised
// visitor instead of added to the Instruction classes itself.

class InstructionVisitor: public StackObj {
 public:
#ifdef ASSERT
          void do_HiWord         (HiWord*          x) { ShouldNotReachHere(); }
#endif // ASSERT
  virtual void do_Phi            (Phi*             x) = 0;
  virtual void do_Local          (Local*           x) = 0;
  virtual void do_Constant       (Constant*        x) = 0;
  virtual void do_LoadLocal      (LoadLocal*       x) = 0;
  virtual void do_StoreLocal     (StoreLocal*      x) = 0;
  virtual void do_LoadField      (LoadField*       x) = 0;
  virtual void do_StoreField     (StoreField*      x) = 0;
  virtual void do_ArrayLength    (ArrayLength*     x) = 0;
  virtual void do_LoadIndexed    (LoadIndexed*     x) = 0;
  virtual void do_StoreIndexed   (StoreIndexed*    x) = 0;
  virtual void do_CachingChange  (CachingChange*   x) = 0;
  virtual void do_NegateOp       (NegateOp*        x) = 0;
  virtual void do_ArithmeticOp   (ArithmeticOp*    x) = 0;
  virtual void do_ShiftOp        (ShiftOp*         x) = 0;
  virtual void do_LogicOp        (LogicOp*         x) = 0;
  virtual void do_CompareOp      (CompareOp*       x) = 0;
  virtual void do_IfOp           (IfOp*            x) = 0;
  virtual void do_Convert        (Convert*         x) = 0;
  virtual void do_NullCheck      (NullCheck*       x) = 0;
  virtual void do_Invoke         (Invoke*          x) = 0;
  virtual void do_NewInstance    (NewInstance*     x) = 0;
  virtual void do_NewTypeArray   (NewTypeArray*    x) = 0;
  virtual void do_NewObjectArray (NewObjectArray*  x) = 0;
  virtual void do_NewMultiArray  (NewMultiArray*   x) = 0;
  virtual void do_CheckCast      (CheckCast*       x) = 0;
  virtual void do_InstanceOf     (InstanceOf*      x) = 0;
  virtual void do_MonitorEnter   (MonitorEnter*    x) = 0;
  virtual void do_MonitorExit    (MonitorExit*     x) = 0;
  virtual void do_Intrinsic      (Intrinsic*       x) = 0;
  virtual void do_BlockBegin     (BlockBegin*      x) = 0;
  virtual void do_Goto           (Goto*            x) = 0;
  virtual void do_If             (If*              x) = 0;
  virtual void do_IfInstanceOf   (IfInstanceOf*    x) = 0;
  virtual void do_TableSwitch    (TableSwitch*     x) = 0;
  virtual void do_LookupSwitch   (LookupSwitch*    x) = 0;
  virtual void do_Return         (Return*          x) = 0;
  virtual void do_Throw          (Throw*           x) = 0;
  virtual void do_Base           (Base*            x) = 0;
  virtual void do_UnsafeGetRaw   (UnsafeGetRaw*    x) = 0;
  virtual void do_UnsafePutRaw   (UnsafePutRaw*    x) = 0;
  virtual void do_UnsafeGetObject(UnsafeGetObject* x) = 0;
  virtual void do_UnsafePutObject(UnsafePutObject* x) = 0;
};


// Hashing support
//
// Note: This hash functions affect the performance
//       of ValueMap - make changes carefully!

#define HASH1(x1            )                    ((intx)(x1))
#define HASH2(x1, x2        )                    ((HASH1(x1        ) << 7) ^ HASH1(x2))
#define HASH3(x1, x2, x3    )                    ((HASH2(x1, x2    ) << 7) ^ HASH1(x3))
#define HASH4(x1, x2, x3, x4)                    ((HASH3(x1, x2, x3) << 7) ^ HASH1(x4))


// The following macros are used to implement instruction-specific hashing.
// By default, each instruction implements hash() and is_equal(Value), used
// for value numbering/common subexpression elimination. The default imple-
// mentation disables value numbering. Each instruction which can be value-
// numbered, should define corresponding hash() and is_equal(Value) functions
// via the macros below. The f arguments specify all the values/op codes, etc.
// that need to be identical for two instructions to be identical.
//
// Note: The default implementation of hash() returns 0 in order to indicate
//       that the instruction should not be considered for value numbering.
//       The currently used hash functions do not guarantee that never a 0
//       is produced. While this is still correct, it may be a performance
//       bug (no value numbering for that node). However, this situation is
//       so unlikely, that we are not going to handle it specially.

#define HASHING1(class_name, enabled, f1)             \
  virtual intx hash() const {                         \
    return (enabled) ? HASH2(name(), f1) : 0;         \
  }                                                   \
  virtual bool is_equal(Value v) const {              \
    if (!(enabled)  ) return false;                   \
    class_name* _v = v->as_##class_name();            \
    if (_v == NULL  ) return false;                   \
    if (f1 != _v->f1) return false;                   \
    return true;                                      \
  }                                                   \


#define HASHING2(class_name, enabled, f1, f2)         \
  virtual intx hash() const {                         \
    return (enabled) ? HASH3(name(), f1, f2) : 0;     \
  }                                                   \
  virtual bool is_equal(Value v) const {              \
    if (!(enabled)  ) return false;                   \
    class_name* _v = v->as_##class_name();            \
    if (_v == NULL  ) return false;                   \
    if (f1 != _v->f1) return false;                   \
    if (f2 != _v->f2) return false;                   \
    return true;                                      \
  }                                                   \


#define HASHING3(class_name, enabled, f1, f2, f3)     \
  virtual intx hash() const {                          \
    return (enabled) ? HASH4(name(), f1, f2, f3) : 0; \
  }                                                   \
  virtual bool is_equal(Value v) const {              \
    if (!(enabled)  ) return false;                   \
    class_name* _v = v->as_##class_name();            \
    if (_v == NULL  ) return false;                   \
    if (f1 != _v->f1) return false;                   \
    if (f2 != _v->f2) return false;                   \
    if (f3 != _v->f3) return false;                   \
    return true;                                      \
  }                                                   \


// The mother of all instructions...

class Instruction: public CompilationResourceObj {
 private:
  static int   _next_id;                         // the node counter

  int          _id;                              // the unique instruction id
  int          _bci;                             // the instruction bci
  int          _use_count;                       // the number of instructions refering to this value (w/o prev/next); only roots can have use count = 0 or > 1
  int          _pin_state;                       // set of PinReason describing the reason for pinning
  ValueType*   _type;                            // the instruction value type
  Instruction* _next;                            // the next instruction if any (NULL for BlockEnd instructions)
  Instruction* _subst;                           // the substitution instruction if any
  Item*        _item;                            // the machine specific information
  LIR_Opr      _operand;                         // LIR specific information
  unsigned int _flags;                           // Flag bits

  ExceptionScope* _exception_scope;              // List of exception handlers covering this instruction.
                                                 // Organized in nested fashion. This list can be conservative
                                                 // and contain more exception handlers than can be reached or
                                                 // that actually cover this instruction. Used to build the
                                                 // ExceptionRangeTable.

  friend class UseCountComputer;
  friend class Inliner;

 protected:
  void set_bci(int bci)                          { assert(bci == SynchronizationEntryBCI || bci >= 0, "illegal bci"); _bci = bci; }

 public:
  enum InstructionFlag {
    NeedsNullCheckFlag = 0,
    CanTrapFlag,
    DirectCompareFlag,
    IsEliminatedFlag,
    IsInitializedFlag,
    IsInlineArgumentFlag,
    IsLoadedFlag,
    IsSafepointFlag,
    IsStaticFlag,
    IsStrictfpFlag,
    NeedsRangeCheckFlag,
    NeedsStoreCheckFlag,
    NeedsWriteBarrierFlag,
    PreservesStateFlag,
    TargetIsFinalFlag,
    TargetIsLoadedFlag,
    TargetIsStrictfpFlag,
    UnorderedIsTrueFlag,
    KeepStateBeforeAliveFlag,
    ThrowIncompatibleClassChangeErrorFlag,
    InstructionLastFlag
  };

 public:
  bool check_flag(InstructionFlag id) const      { return _flags & (1 << id);    }
  void set_flag(InstructionFlag id, bool f)      { _flags = f ? (_flags | (1 << id)) : (_flags & ~(1 << id)); };

  // 'globally' used condition values
  enum Condition {
    eql, neq, lss, leq, gtr, geq
  };

  // Instructions may be pinned for many reasons and under certain conditions
  // with enough knowledge it's possible to safely unpin them.
  enum PinReason {
      PinUnknown           = 1 << 0
    , PinInlineReturnValue = 1 << 1
    , PinInlineLocals      = 1 << 2
    , PinExplicitNullCheck = 1 << 3
    , PinStoreIndexed      = 1 << 4
    , PinUninitialized     = 1 << 5
    , PinEndOfBlock        = 1 << 6
    , PinInlineEndOfBlock  = 1 << 7
    , PinStackCEE          = 1 << 8
    , PinStackLocals       = 1 << 9
    , PinStackFields       = 1 << 10
    , PinStackIndexed      = 1 << 11
    , PinStackForStateSplit= 1 << 12
    , PinStateSplitConstructor= 1 << 13
  };

  static Condition mirror(Condition cond);
  static Condition negate(Condition cond);

  // initialization
  static void initialize()                       { _next_id = 0; }
  static int number_of_instructions()            { return _next_id; }

  // creation
  Instruction(ValueType* type)
  : _id(_next_id++)
  , _bci(-1)
  , _use_count(0)
  , _pin_state(0)
  , _type(type)
  , _next(NULL)
  , _subst(NULL)
  , _item(NULL)
  , _flags(0)
  , _operand(LIR_OprFact::illegalOpr)
  , _exception_scope(NULL)
  {
    assert(type != NULL, "type must exist");
  }

  // accessors
  int id() const                                 { return _id; }
  int bci() const                                { return _bci; }
  int use_count() const                          { return _use_count; }
  int pin_state() const                          { return _pin_state; }
  bool is_pinned() const                         { return _pin_state != 0 || PinAllInstructions; }
  ValueType* type() const                        { return _type; }
  Instruction* prev(BlockBegin* block);          // use carefully, expensive operation
  Instruction* next() const                      { return _next; }
  Instruction* subst()                           { return _subst == NULL ? this : _subst->subst(); }
  Item* item() const                             { return _item; }
  LIR_Opr operand() const                        { return _operand; }

  void set_needs_null_check(bool f)              { set_flag(NeedsNullCheckFlag, f); }
  bool needs_null_check() const                  { return check_flag(NeedsNullCheckFlag); }

  bool has_uses() const                          { return use_count() > 0; }
  bool is_root() const                           { return is_pinned() || use_count() > 1; }
  ExceptionScope* exception_scope() const        { return _exception_scope; }

  // manipulation
  void pin(PinReason reason)                     { _pin_state |= reason; }
  void pin()                                     { _pin_state |= PinUnknown; }
  // DANGEROUS: only used by EliminateStores
  void unpin(PinReason reason)                   { assert((reason & PinUnknown) == 0, "can't unpin unknown state"); _pin_state &= ~reason; }
  virtual void set_lock_stack(ValueStack* l)     { /* do nothing*/ }
  virtual ValueStack* lock_stack() const         { return NULL; }

  Instruction* set_next(Instruction* next, int bci) {
    if (next != NULL) {
      assert(as_BlockEnd() == NULL, "BlockEnd instructions must have no next");
      next->set_bci(bci);
    }
    _next = next;
    return next;
  }

  void set_subst(Instruction* subst)             { _subst = subst; }
  void set_exception_scope(ExceptionScope *s)    { _exception_scope = s; }

  // machine-specifics
  void set_item(Item* item)                      { assert(item != NULL, "item must exist"); _item = item; }
  void clear_item()                              { _item = NULL; }
  void set_operand(LIR_Opr operand)              { assert(operand != LIR_OprFact::illegalOpr, "operand must exist"); _operand = operand; }
  void clear_operand()                           { _operand = LIR_OprFact::illegalOpr; }

  // generic
  virtual Instruction*      as_Instruction()     { return this; } // to satisfy HASHING1 macro
#ifdef ASSERT
  virtual HiWord*           as_HiWord()          { return NULL; }
#endif // ASSERT
  virtual Phi*              as_Phi()             { return NULL; }
  virtual Local*            as_Local()           { return NULL; }
  virtual Constant*         as_Constant()        { return NULL; }
  virtual AccessLocal*      as_AccessLocal()     { return NULL; }
  virtual LoadLocal*        as_LoadLocal()       { return NULL; }
  virtual StoreLocal*       as_StoreLocal()      { return NULL; }
  virtual AccessField*      as_AccessField()     { return NULL; }
  virtual LoadField*        as_LoadField()       { return NULL; }
  virtual StoreField*       as_StoreField()      { return NULL; }
  virtual ArrayLength*      as_ArrayLength()     { return NULL; }
  virtual AccessIndexed*    as_AccessIndexed()   { return NULL; }
  virtual LoadIndexed*      as_LoadIndexed()     { return NULL; }
  virtual StoreIndexed*     as_StoreIndexed()    { return NULL; }
  virtual CachingChange*    as_CachingChange()   { return NULL; }
  virtual NegateOp*         as_NegateOp()        { return NULL; }
  virtual Op2*              as_Op2()             { return NULL; }
  virtual ArithmeticOp*     as_ArithmeticOp()    { return NULL; }
  virtual ShiftOp*          as_ShiftOp()         { return NULL; }
  virtual LogicOp*          as_LogicOp()         { return NULL; }
  virtual CompareOp*        as_CompareOp()       { return NULL; }
  virtual IfOp*             as_IfOp()            { return NULL; }
  virtual Convert*          as_Convert()         { return NULL; }
  virtual NullCheck*        as_NullCheck()       { return NULL; }
  virtual StateSplit*       as_StateSplit()      { return NULL; }
  virtual Invoke*           as_Invoke()          { return NULL; }
  virtual NewInstance*      as_NewInstance()     { return NULL; }
  virtual NewArray*         as_NewArray()        { return NULL; }
  virtual NewTypeArray*     as_NewTypeArray()    { return NULL; }
  virtual NewObjectArray*   as_NewObjectArray()  { return NULL; }
  virtual NewMultiArray*    as_NewMultiArray()   { return NULL; }
  virtual TypeCheck*        as_TypeCheck()       { return NULL; }
  virtual CheckCast*        as_CheckCast()       { return NULL; }
  virtual InstanceOf*       as_InstanceOf()      { return NULL; }
  virtual AccessMonitor*    as_AccessMonitor()   { return NULL; }
  virtual MonitorEnter*     as_MonitorEnter()    { return NULL; }
  virtual MonitorExit*      as_MonitorExit()     { return NULL; }
  virtual Intrinsic*        as_Intrinsic()       { return NULL; }
  virtual BlockBegin*       as_BlockBegin()      { return NULL; }
  virtual BlockEnd*         as_BlockEnd()        { return NULL; }
  virtual Goto*             as_Goto()            { return NULL; }
  virtual If*               as_If()              { return NULL; }
  virtual IfInstanceOf*     as_IfInstanceOf()    { return NULL; }
  virtual TableSwitch*      as_TableSwitch()     { return NULL; }
  virtual LookupSwitch*     as_LookupSwitch()    { return NULL; }
  virtual Return*           as_Return()          { return NULL; }
  virtual Throw*            as_Throw()           { return NULL; }
  virtual Base*             as_Base()            { return NULL; }
  virtual UnsafeOp*         as_UnsafeOp()        { return NULL; }

  virtual void visit(InstructionVisitor* v)      = 0;

  virtual bool can_trap() const                  { return false; }

  virtual void input_values_do(void f(Value*))   = 0;
  virtual void state_values_do(void f(Value*))   { /* usually no state - override on demand */ }
  virtual void other_values_do(void f(Value*))   { /* usually no other - override on demand */ }
          void       values_do(void f(Value*))   { input_values_do(f); state_values_do(f); other_values_do(f); }

  virtual ciType* exact_type() const             { return NULL; }
  virtual ciType* declared_type() const          { return NULL; }

  // hashing
  virtual const char* name() const               = 0;
  HASHING1(Instruction, false, id())             // hashing disabled by default

  // debugging
  void print()                                   PRODUCT_RETURN;
  void print(InstructionPrinter& ip)             PRODUCT_RETURN;
};


// The following macros are used to define base (i.e., non-leaf)
// and leaf instruction classes. They define class-name related
// generic functionality in one place.

#define BASE(class_name, super_class_name)       \
  class class_name: public super_class_name {    \
   public:                                       \
    virtual class_name* as_##class_name()        { return this; }              \


#define LEAF(class_name, super_class_name)       \
  BASE(class_name, super_class_name)             \
   public:                                       \
    virtual const char* name() const             { return #class_name; }       \
    virtual void visit(InstructionVisitor* v)    { v->do_##class_name(this); } \


// Debugging support

#ifdef ASSERT
  static void assert_value(Value* x)             { assert((*x) != NULL, "value must exist"); }
  #define ASSERT_VALUES                          values_do(assert_value);
#else
  #define ASSERT_VALUES
#endif // ASSERT


// A HiWord occupies the 'high word' of a 2-word
// expression stack entry. Hi & lo words must be
// paired on the expression stack (otherwise the
// bytecode sequence is illegal). Note that 'hi'
// refers to the IR expression stack format and
// does *not* imply a machine word ordering. No
// HiWords are used in optimized mode for speed,
// but NULL pointers are used instead.

#ifdef ASSERT
LEAF(HiWord, Instruction)
 private:
  Value _lo_word;

 public:
  // creation
  HiWord(Value lo_word) : Instruction(lo_word->type()), _lo_word(lo_word) {
    assert(lo_word->type()->is_double_word(), "HiWord must be used for 2-word values only");
  }

  // accessors
  Value lo_word() const                          { return _lo_word; }

  // generic
  virtual void input_values_do(void f(Value*))   { ShouldNotReachHere(); }
};
#endif // ASSERT


// A Phi instruction is a placeholder for a value
// coming in from one or more predecessor blocks.
// Phi instructions can only appear in the state
// array of a BlockBegin. Currently Phis are used
// only for stack values.

LEAF(Phi, Instruction)
 public:
  // creation
  Phi(ValueType* type) : Instruction(type->base()) {
    // Note: If the type of a phi is a constant type, the phi's constant
    //       value may be used by the canonicalizer. However, currently,
    //       we do not correctly compute the merged phi type, thus we
    //       always use the non-constant base type for phis for now -
    //       was bug - gri 6/23/99.
    pin();
  }

  // generic
  virtual void input_values_do(void f(Value*))   { /* no values */ }
  HASHING1(Phi, true, id());
};


LEAF(Local, Instruction)
 private:
  int      _java_index;                          // the local index within the method to which the local belongs
  int      _local_name;                          // the name of this local, unique across inlined scopes
  int      _hi_word_local_name;                  // for doubleword locals, the name of the hi-word half
  int      _offset;                              // now needed only for caching of locals.
                                                 // can go away after new register allocator is in place.
                                                 // because of how LP64 behaves currently, we don't allocate
                                                 // an offset for the hi word of doubleword locals in shared
                                                 // code, instead leaving this up to the FrameMap.
  bool     _is_incoming_argument;                // Indicates that this local is an incoming argument to the top scope
                                                 // (includes the receiver)

 public:
  // creation
  Local(ValueType* type, int index)
    : Instruction(type)
    , _java_index(index)
    , _local_name(-1)
    , _hi_word_local_name(-1)
    , _offset(-1)
    , _is_incoming_argument(false)
  {}

  // accessors
  int java_index() const                         { return _java_index; }
  int has_local_name() const                     { return _local_name >= 0; }
  int local_name() const                         { return _local_name; }
  int has_hi_word_local_name() const             { return _hi_word_local_name >= 0; }
  int hi_word_local_name() const                 { return _hi_word_local_name; }
  int has_offset() const                         { return _offset >= 0; }
  WordSize offset() const                        { return in_WordSize(_offset); }
  bool is_incoming_argument() const              { return _is_incoming_argument; }

  // setters
  void set_local_name(int name )                 { _local_name = name; assert(has_local_name(), "illegal name"); }
  void set_hi_word_local_name(int name )         { assert(type()->is_double_word(), "wrong type");
                                                   _hi_word_local_name = name;
                                                   assert(has_hi_word_local_name(), "illegal hi word name"); }
  void set_offset(WordSize offset)               { _offset = in_words(offset); assert(has_offset(), "illegal offset"); }
  void set_is_incoming_argument()                { _is_incoming_argument = true; }

  // generic
  virtual void input_values_do(void f(Value*))   { /* no values */ }
};


LEAF(Constant, Instruction)
  bool _may_be_commoned;
  ValueStack* _state;

 public:
  // creation
  Constant(ValueType* type, bool may_be_commoned = true):
      Instruction(type)
  , _may_be_commoned(may_be_commoned)
  , _state(NULL) {
    assert(type->is_constant(), "must be a constant");
  }

  Constant(ValueType* type, ValueStack* state):
    Instruction(type)
  , _may_be_commoned(false)
  , _state(state) {
    assert(state != NULL, "only used for constants which need patching");
    assert(type->is_constant(), "must be a constant");
  }

  ValueStack* state() const               { return _state; }

  virtual bool can_trap() const           { return (_state != NULL); }

  // generic
  virtual void input_values_do(void f(Value*))   { /* no values */ }
  virtual void other_values_do(void f(Value*));

  virtual intx hash() const;
  virtual bool is_equal(Value v) const;
};


BASE(AccessLocal, Instruction)
 private:
  Local* _local;

 public:
  // creation
  AccessLocal(Local* local) : Instruction(local->type()), _local(local) {}

  // accessors
  Local* local() const                           { return _local; }
  int java_index() const                         { return local()->java_index(); }
  int has_local_name() const                     { return local()->has_local_name(); }
  int local_name() const                         { return local()->local_name(); }
  int has_offset() const                         { return local()->has_offset(); }
  WordSize offset() const                        { return local()->offset(); }
  bool is_receiver() const                       { return java_index() == 0; }

  // generic
  virtual void input_values_do(void f(Value*))   { f((Value*)&_local); }
};


LEAF(LoadLocal, AccessLocal)
 public:
  // creation
  LoadLocal(Local* local) : AccessLocal(local) {
    assert(!local->type()->is_constant(), "must not be constant type");
  }

  // Note: LoadLocal is currently not used with value numbering as
  //       load elimination is taking care of it - eventually, value
  //       numbering could do this as well.
};


LEAF(StoreLocal, AccessLocal)
 private:
  Value _value;

 public:
  // creation
  StoreLocal(Local* local, Value value, bool is_inline_argument)
  : AccessLocal(local)
  , _value(value) {
    set_flag(IsInlineArgumentFlag, is_inline_argument);
    set_eliminated(false);
    ASSERT_VALUES
    if (is_inline_argument) {
      pin(Instruction::PinInlineLocals);
    } else {
      pin();
    }
  }

  // accessors
  Value value() const                            { return _value; }
  bool  is_inline_argument() const               { return check_flag(IsInlineArgumentFlag); }
  // NOTE: pin_state test is conservative to preserve the same semantics
  // as the original store elimination optimization.
  bool  is_eliminated() const                    { return check_flag(IsEliminatedFlag) && (pin_state() == PinInlineLocals); }

  void  set_eliminated(bool val)                 { set_flag(IsEliminatedFlag, val); }

  // generic
  // NOTE: always traverse the value because its item is loaded.
  // Elimination decrements use count of that local, however.
  virtual void input_values_do(void f(Value*))   { if (!is_eliminated()) { AccessLocal::input_values_do(f); } f(&_value); }
};


BASE(AccessField, Instruction)
 private:
  Value       _obj;
  int         _offset;
  ciField*    _field;
  ValueStack* _state_before;                     // state is set only for unloaded or uninitialized fields
  ValueStack* _lock_stack;                       // contains lock and scope information
  NullCheck*  _explicit_null_check;              // For explicit null check elimination

 public:
  // creation
  AccessField(Value obj, int offset, ciField* field, bool is_static, ValueStack* lock_stack, bool is_loaded, bool is_initialized)
  : Instruction(as_ValueType(field->type()->basic_type()))
  , _obj(obj)
  , _offset(offset)
  , _field(field)
  , _lock_stack(lock_stack)
  , _state_before(NULL)
  , _explicit_null_check(NULL)
  {
    set_needs_null_check(!is_static);
    set_flag(IsLoadedFlag, is_loaded);
    set_flag(IsInitializedFlag, is_initialized);
    set_flag(IsStaticFlag, is_static);
    ASSERT_VALUES
    if (!is_loaded || !is_initialized || field->is_volatile()) pin(); // in this case we have patching at runtime => needs pinning
  }

  // accessors
  Value obj() const                              { return _obj; }
  int offset() const                             { return _offset; }
  ciField* field() const                         { return _field; }
  BasicType field_type() const                   { return _field->type()->basic_type(); }
  bool is_static() const                         { return check_flag(IsStaticFlag); }
  bool is_loaded() const                         { return check_flag(IsLoadedFlag); }
  bool is_initialized() const                    { return check_flag(IsInitializedFlag); }
  ValueStack* state_before() const               { return _state_before; }
  ValueStack* lock_stack() const                 { return _lock_stack; }
  NullCheck* explicit_null_check() const         { return _explicit_null_check; }
  bool needs_patching() const                    { return !is_initialized() || !is_loaded() || PatchALot; }

  // manipulation
  void set_state_before(ValueStack* state)       { _state_before = state; }
  void set_lock_stack(ValueStack* l)             { _lock_stack = l; }
  // Under certain circumstances, if a previous NullCheck instruction
  // proved the target object non-null, we can eliminate the explicit
  // null check and do an implicit one, simply specifying the debug
  // information from the NullCheck. This field should only be consulted
  // if needs_null_check() is true.
  void set_explicit_null_check(NullCheck* check) { _explicit_null_check = check; }

  // generic
  virtual bool can_trap() const                  { return true; }
  virtual void input_values_do(void f(Value*))   { f(&_obj); }
  virtual void other_values_do(void f(Value*));
};


LEAF(LoadField, AccessField)
 public:
  // creation
  LoadField(Value obj, int offset, ciField* field, bool is_static, ValueStack* lock_stack, bool is_loaded, bool is_initialized)
  : AccessField(obj, offset, field, is_static, lock_stack, is_loaded, is_initialized)
  {}

  ciType* declared_type() const;
  ciType* exact_type() const;

  // generic
  HASHING2(LoadField, is_loaded(), obj(), offset())  // cannot be eliminated if not yet loaded
};


LEAF(StoreField, AccessField)
 private:
  Value _value;

 public:
  // creation
  StoreField(Value obj, int offset, ciField* field, Value value, bool is_static, ValueStack* lock_stack, bool is_loaded, bool is_initialized)
  : AccessField(obj, offset, field, is_static, lock_stack, is_loaded, is_initialized)
  , _value(value)
  {
    set_flag(NeedsWriteBarrierFlag, as_ValueType(field_type())->is_object());
    ASSERT_VALUES
    pin();
  }

  // accessors
  Value value() const                            { return _value; }
  bool needs_write_barrier() const               { return check_flag(NeedsWriteBarrierFlag); }

  // generic
  virtual void input_values_do(void f(Value*))   { AccessField::input_values_do(f); f(&_value); }
};


BASE(AccessArray, Instruction)
 private:
  Value       _array;
  ValueStack* _lock_stack;

 public:
  // creation
  AccessArray(ValueType* type, Value array, ValueStack* lock_stack)
  : Instruction(type)
  , _array(array)
  , _lock_stack(lock_stack) {
    set_needs_null_check(true);
    ASSERT_VALUES
    pin(); // instruction with side effect (null exception or range check throwing)
  }

  Value array() const                            { return _array; }
  ValueStack* lock_stack() const                 { return _lock_stack; }

  // setters
  void set_lock_stack(ValueStack* l)             { _lock_stack = l; }

  // generic
  virtual bool can_trap() const                  { return true; }
  virtual void input_values_do(void f(Value*))   { f(&_array); }
};


LEAF(ArrayLength, AccessArray)
 private:
  NullCheck*  _explicit_null_check;              // For explicit null check elimination

 public:
  // creation
  ArrayLength(Value array, ValueStack* lock_stack)
  : AccessArray(intType, array, lock_stack)
  , _explicit_null_check(NULL) {}

  // accessors
  NullCheck* explicit_null_check() const         { return _explicit_null_check; }

  // setters
  // See LoadField::set_explicit_null_check for documentation
  void set_explicit_null_check(NullCheck* check) { _explicit_null_check = check; }

  // generic
  HASHING1(ArrayLength, true, array())
};


BASE(AccessIndexed, AccessArray)
 private:
  Value     _index;
  Value     _length;
  BasicType _elt_type;

 public:
  // creation
  AccessIndexed(Value array, Value index, Value length, BasicType elt_type, ValueStack* lock_stack)
  : AccessArray(as_ValueType(elt_type), array, lock_stack)
  , _index(index)
  , _length(length)
  , _elt_type(elt_type)
  {
    set_flag(NeedsRangeCheckFlag, true);
    ASSERT_VALUES
  }

  // accessors
  Value index() const                            { return _index; }
  Value length() const                           { return _length; }
  BasicType elt_type() const                     { return _elt_type; }
  bool needs_range_check() const                 { return check_flag(NeedsRangeCheckFlag); }

  // generic
  virtual void input_values_do(void f(Value*))   { AccessArray::input_values_do(f); f(&_index); if (_length != NULL) f(&_length); }
};


LEAF(LoadIndexed, AccessIndexed)
 private:
  NullCheck*  _explicit_null_check;              // For explicit null check elimination

 public:
  // creation
  LoadIndexed(Value array, Value index, Value length, BasicType elt_type, ValueStack* lock_stack)
  : AccessIndexed(array, index, length, elt_type, lock_stack)
  , _explicit_null_check(NULL) {}

  // accessors
  NullCheck* explicit_null_check() const         { return _explicit_null_check; }

  // setters
  // See LoadField::set_explicit_null_check for documentation
  void set_explicit_null_check(NullCheck* check) { _explicit_null_check = check; }

  ciType* exact_type() const;
  ciType* declared_type() const;

  // generic
  HASHING2(LoadIndexed, true, array(), index())
};


LEAF(StoreIndexed, AccessIndexed)
 private:
  Value       _value;

 public:
  // creation
  StoreIndexed(Value array, Value index, Value length, BasicType elt_type, Value value, ValueStack* lock_stack)
  : AccessIndexed(array, index, length, elt_type, lock_stack)
  , _value(value)
  {
    set_flag(NeedsWriteBarrierFlag, (as_ValueType(elt_type)->is_object()));
    set_flag(NeedsStoreCheckFlag, (as_ValueType(elt_type)->is_object()));
    ASSERT_VALUES
    pin();
  }

  // accessors
  Value value() const                            { return _value; }
  IRScope* scope() const;                        // the state's scope
  bool needs_write_barrier() const               { return check_flag(NeedsWriteBarrierFlag); }
  bool needs_store_check() const                 { return check_flag(NeedsStoreCheckFlag); }

  // generic
  virtual void input_values_do(void f(Value*))   { AccessIndexed::input_values_do(f); f(&_value); }
  virtual void other_values_do(void f(Value*))   { }
};


LEAF(CachingChange, Instruction)
 private:
  BlockBegin*   _pred;
  BlockBegin*   _sux;
  LocalMapping* _pred_mapping;
  LocalMapping* _sux_mapping;
 public:
  // creation
  CachingChange(BlockBegin* pred, BlockBegin* sux)
  : Instruction(illegalType)
  , _pred_mapping(NULL)
  , _sux_mapping(NULL)
  , _pred(pred)
  , _sux(sux)
  {
    pin();
  }


  BlockBegin* pred_block() const                 { return _pred; }
  BlockBegin* sux_block() const                  { return _sux; }

  LocalMapping* pred_mapping() const             { return _pred_mapping; }
  void set_pred_mapping(LocalMapping* mapping)   { _pred_mapping = mapping; }

  LocalMapping* sux_mapping() const              { return _sux_mapping; }
  void set_sux_mapping(LocalMapping* mapping)    { _sux_mapping = mapping; }

  // generic
  virtual void input_values_do(void f(Value*))   { /* no values */ }
};


LEAF(NegateOp, Instruction)
 private:
  Value _x;

 public:
  // creation
  NegateOp(Value x) : Instruction(x->type()->base()), _x(x) {
    ASSERT_VALUES
  }

  // accessors
  Value x() const                                { return _x; }

  // generic
  virtual void input_values_do(void f(Value*))   { f(&_x); }
};


BASE(Op2, Instruction)
 private:
  Bytecodes::Code _op;
  Value           _x;
  Value           _y;

 public:
  // creation
  Op2(ValueType* type, Bytecodes::Code op, Value x, Value y) : Instruction(type), _op(op), _x(x), _y(y) {
    ASSERT_VALUES
  }

  // accessors
  Bytecodes::Code op() const                     { return _op; }
  Value x() const                                { return _x; }
  Value y() const                                { return _y; }

  // manipulators
  void swap_operands() {
    assert(is_commutative(), "operation must be commutative");
    Value t = _x; _x = _y; _y = t;
  }

  // generic
  virtual bool is_commutative() const            { return false; }
  virtual void input_values_do(void f(Value*))   { f(&_x); f(&_y); }
};


LEAF(ArithmeticOp, Op2)
 private:
  ValueStack* _lock_stack;                       // used only for division operations
 public:
  // creation
  ArithmeticOp(Bytecodes::Code op, Value x, Value y, bool is_strictfp, ValueStack* lock_stack)
  : Op2(x->type()->meet(y->type()), op, x, y)
  ,  _lock_stack(lock_stack) {
    set_flag(IsStrictfpFlag, is_strictfp);
    if (can_trap()) pin();
  }

  // accessors
  ValueStack* lock_stack() const                 { return _lock_stack; }
  bool        is_strictfp() const                { return check_flag(IsStrictfpFlag); }

  // setters
  void set_lock_stack(ValueStack* l)             { _lock_stack = l; }

  // generic
  virtual bool is_commutative() const;
  virtual bool can_trap() const;
  HASHING3(Op2, true, op(), x(), y())
};


LEAF(ShiftOp, Op2)
 public:
  // creation
  ShiftOp(Bytecodes::Code op, Value x, Value s) : Op2(x->type()->base(), op, x, s) {}

  // generic
  HASHING3(Op2, true, op(), x(), y())
};


LEAF(LogicOp, Op2)
 public:
  // creation
  LogicOp(Bytecodes::Code op, Value x, Value y) : Op2(x->type()->meet(y->type()), op, x, y) {}

  // generic
  virtual bool is_commutative() const;
  HASHING3(Op2, true, op(), x(), y())
};


LEAF(CompareOp, Op2)
 private:
  ValueStack* _state_before;                     // for deoptimization, when canonicalizing
 public:
  // creation
  CompareOp(Bytecodes::Code op, Value x, Value y, ValueStack* state_before)
  : Op2(intType, op, x, y)
  , _state_before(state_before)
  {}

  // accessors
  ValueStack* state_before() const               { return _state_before; }

  // manipulation
  void set_state_before(ValueStack* state)       { _state_before = state; }

  // generic
  HASHING3(Op2, true, op(), x(), y())
  virtual void other_values_do(void f(Value*));
};


LEAF(IfOp, Op2)
 private:
  Value _tval;
  Value _fval;

 public:
  // creation
  IfOp(Value x, Condition cond, Value y, Value tval, Value fval)
  : Op2(tval->type()->meet(fval->type()), (Bytecodes::Code)cond, x, y)
  , _tval(tval)
  , _fval(fval)
  {
    ASSERT_VALUES
    assert(tval->type()->tag() == fval->type()->tag(), "types must match");
  }

  // accessors
  virtual bool is_commutative() const;
  Bytecodes::Code op() const                     { ShouldNotCallThis(); return Bytecodes::_illegal; }
  Condition cond() const                         { return (Condition)Op2::op(); }
  Value tval() const                             { return _tval; }
  Value fval() const                             { return _fval; }

  // generic
  virtual void input_values_do(void f(Value*))   { Op2::input_values_do(f); f(&_tval); f(&_fval); }
};


LEAF(Convert, Instruction)
 private:
  Bytecodes::Code _op;
  Value           _value;

 public:
  // creation
  Convert(Bytecodes::Code op, Value value, ValueType* to_type) : Instruction(to_type), _op(op), _value(value) {
    ASSERT_VALUES
  }

  // accessors
  Bytecodes::Code op() const                     { return _op; }
  Value value() const                            { return _value; }

  // generic
  virtual void input_values_do(void f(Value*))   { f(&_value); }
  HASHING2(Convert, true, op(), value())
};


LEAF(NullCheck, Instruction)
 private:
  Value       _obj;
  ValueStack* _lock_stack;

 public:
  // creation
  NullCheck(Value obj, ValueStack* lock_stack) : Instruction(obj->type()), _obj(obj), _lock_stack(lock_stack) {
    ASSERT_VALUES
    set_can_trap(true);
    assert(_obj->type()->is_object(), "null check must be applied to objects only");
    pin(Instruction::PinExplicitNullCheck);
  }

  // accessors
  Value obj() const                              { return _obj; }
  ValueStack* lock_stack() const                 { return _lock_stack; }

  // setters
  void set_lock_stack(ValueStack* l)             { _lock_stack = l; }
  void set_can_trap(bool can_trap)               {     set_flag(CanTrapFlag, can_trap); }

  // generic
  virtual bool can_trap() const                  { return check_flag(CanTrapFlag); /* null-check elimination sets to false */ }
  virtual void input_values_do(void f(Value*))   { f(&_obj); }
};


BASE(StateSplit, Instruction)
 private:
  ValueStack* _state;

 public:
  // creation
  StateSplit(ValueType* type) : Instruction(type), _state(NULL) {
    pin(PinStateSplitConstructor);
  }

  // accessors
  ValueStack* state() const                      { return _state; }
  IRScope* scope() const;                        // the state's scope

  // manipulation
  void set_state(ValueStack* state)              { _state = state; }

  // generic
  virtual void input_values_do(void f(Value*))   { /* no values */ }
  virtual void state_values_do(void f(Value*));
};


LEAF(Invoke, StateSplit)
 private:
  Bytecodes::Code           _code;
  Value                     _recv;
  Values*                   _args;
  BasicTypeList*            _signature;
  int                       _vtable_index;

 public:
  // creation
  Invoke(Bytecodes::Code code, ValueType* result_type, Value recv, Values* args,
         int vtable_index, bool target_is_final, bool target_is_loaded, bool target_is_strictfp);

  // accessors
  Bytecodes::Code code() const                   { return _code; }
  Value receiver() const                         { return _recv; }
  bool has_receiver() const                      { return receiver() != NULL; }
  int number_of_arguments() const                { return _args->length(); }
  int size_of_arguments() const;
  Value argument_at(int i) const                 { return _args->at(i); }
  int vtable_index() const                       { return _vtable_index; }
  BasicTypeList* signature() const               { return _signature; }

  // Returns false if target is not loaded
  bool target_is_final() const                   { return check_flag(TargetIsFinalFlag); }
  bool target_is_loaded() const                  { return check_flag(TargetIsLoadedFlag); }
  // Returns false if target is not loaded
  bool target_is_strictfp() const                { return check_flag(TargetIsStrictfpFlag); }

  // generic
  virtual bool can_trap() const                  { return true; }
  virtual void input_values_do(void f(Value*)) {
    StateSplit::input_values_do(f);
    if (has_receiver()) f(&_recv);
    for (int i = 0; i < _args->length(); i++) f(_args->adr_at(i));
  }
};


LEAF(NewInstance, StateSplit)
 private:
  ciInstanceKlass* _klass;

 public:
  // creation
  NewInstance(ciInstanceKlass* klass) : StateSplit(instanceType), _klass(klass) {}

  // accessors
  ciInstanceKlass* klass() const                 { return _klass; }

  // generic
  virtual bool can_trap() const                  { return true; }
  ciType* exact_type() const;
};


BASE(NewArray, StateSplit)
 private:
  Value       _length;
  ValueStack* _state_before;

 public:
  // creation
  NewArray(Value length) : StateSplit(objectType), _length(length), _state_before(NULL) {
    // Do not ASSERT_VALUES since length is NULL for NewMultiArray
  }

  // accessors
  ValueStack* state_before() const               { return _state_before; }
  Value length() const                           { return _length; }

  // manipulation
  void set_state_before(ValueStack* state)       { _state_before = state; }

  // generic
  virtual bool can_trap() const                  { return true; }
  virtual void input_values_do(void f(Value*))   { StateSplit::input_values_do(f); f(&_length); }
  virtual void other_values_do(void f(Value*));
};


LEAF(NewTypeArray, NewArray)
 private:
  BasicType _elt_type;

 public:
  // creation
  NewTypeArray(Value length, BasicType elt_type) : NewArray(length), _elt_type(elt_type) {}

  // accessors
  BasicType elt_type() const                     { return _elt_type; }
  ciType* exact_type() const;
};


LEAF(NewObjectArray, NewArray)
 private:
  ciKlass* _klass;

 public:
  // creation
  NewObjectArray(ciKlass* klass, Value length) : NewArray(length), _klass(klass) {}

  // accessors
  ciKlass* klass() const                         { return _klass; }
  ciType* exact_type() const;
};


LEAF(NewMultiArray, NewArray)
 private:
  ciKlass* _klass;
  Values*  _dims;

 public:
  // creation
  NewMultiArray(ciKlass* klass, Values* dims) : NewArray(NULL), _klass(klass), _dims(dims) {
    ASSERT_VALUES
  }

  // accessors
  ciKlass* klass() const                         { return _klass; }
  Values* dims() const                           { return _dims; }
  int rank() const                               { return dims()->length(); }

  // generic
  virtual void input_values_do(void f(Value*)) {
    // NOTE: we do not call NewArray::input_values_do since "length"
    // is meaningless for a multi-dimensional array; passing the
    // zeroth element down to NewArray as its length is a bad idea
    // since there will be a copy in the "dims" array which doesn't
    // get updated, and the value must not be traversed twice. Was bug
    // - kbr 4/10/2001
    StateSplit::input_values_do(f);
    for (int i = 0; i < _dims->length(); i++) f(_dims->adr_at(i));
  }
};


BASE(TypeCheck, StateSplit)
 private:
  ciKlass*    _klass;
  Value       _obj;
  ValueStack* _state_before;

 public:
  // creation
  TypeCheck(ciKlass* klass, Value obj, ValueType* type) : StateSplit(type), _klass(klass), _obj(obj), _state_before(NULL) {
    ASSERT_VALUES
    set_direct_compare(false);
  }

  // accessors
  ValueStack* state_before() const               { return _state_before; }
  ciKlass* klass() const                         { return _klass; }
  Value obj() const                              { return _obj; }
  bool is_loaded() const                         { return klass() != NULL; }
  bool direct_compare() const                    { return check_flag(DirectCompareFlag); }

  // manipulation
  void set_state_before(ValueStack* state)       { _state_before = state; }
  void set_direct_compare(bool flag)             { set_flag(DirectCompareFlag, flag); }

  // generic
  virtual bool can_trap() const                  { return true; }
  virtual void input_values_do(void f(Value*))   { StateSplit::input_values_do(f); f(&_obj); }
  virtual void other_values_do(void f(Value*));
};


LEAF(CheckCast, TypeCheck)
 public:
  // creation
  CheckCast(ciKlass* klass, Value obj) : TypeCheck(klass, obj, objectType) {}

  void set_incompatible_class_change_check() {
    set_flag(ThrowIncompatibleClassChangeErrorFlag, true);
  }
  bool is_incompatible_class_change_check() const {
    return check_flag(ThrowIncompatibleClassChangeErrorFlag);
  }
};


LEAF(InstanceOf, TypeCheck)
 public:
  // creation
  InstanceOf(ciKlass* klass, Value obj) : TypeCheck(klass, obj, intType) {}
};


BASE(AccessMonitor, StateSplit)
 private:
  Value       _obj;
  int         _monitor_no;

 public:
  // creation
  AccessMonitor(Value obj, int monitor_no)
  : StateSplit(illegalType)
  , _obj(obj)
  , _monitor_no(monitor_no)
  {
    set_needs_null_check(true);
    ASSERT_VALUES
  }

  // accessors
  Value obj() const                              { return _obj; }
  int monitor_no() const                         { return _monitor_no; }

  // generic
  virtual bool can_trap() const                  { return true; }
  virtual void input_values_do(void f(Value*))   { StateSplit::input_values_do(f); f(&_obj); }
};


LEAF(MonitorEnter, AccessMonitor)
 private:
  ValueStack* _lock_stack_before;

 public:
  // creation
  MonitorEnter(Value obj, int monitor_no, ValueStack* lock_stack_before)
  : AccessMonitor(obj, monitor_no)
  , _lock_stack_before(lock_stack_before)
  {
    ASSERT_VALUES
  }

  // accessors
  ValueStack* lock_stack_before() const          { return _lock_stack_before; }
};


LEAF(MonitorExit, AccessMonitor)
 public:
  // creation
  MonitorExit(Value obj, int monitor_no) : AccessMonitor(obj, monitor_no) {}
};


LEAF(Intrinsic, StateSplit)
 private:
  ciMethod::IntrinsicId _id;
  Values*               _args;
  ValueStack*           _lock_stack;
  Value                 _recv;

 public:
  // preserves_state can be set to true for Intrinsics
  // which are guaranteed to preserve register state across any slow
  // cases; setting it to true does not mean that the Intrinsic can
  // not trap, only that if we continue execution in the same basic
  // block after the Intrinsic, all of the registers are intact. This
  // allows load elimination and common expression elimination to be
  // performed across the Intrinsic.  The default value is false.
  Intrinsic(ValueType* type,
            ciMethod::IntrinsicId id,
            Values* args,
            bool has_receiver,
            ValueStack* lock_stack,
            bool preserves_state = false)
  : StateSplit(type)
  , _id(id)
  , _args(args)
  , _lock_stack(lock_stack)
  , _recv(NULL)
  {
    assert(args != NULL, "args must exist");
    ASSERT_VALUES
    set_flag(PreservesStateFlag, preserves_state);
    if (preserves_state && !has_receiver) 
      unpin(PinStateSplitConstructor);
    if (has_receiver) {
      _recv = argument_at(0);
    }
    set_needs_null_check(has_receiver);
  }

  // accessors
  ciMethod::IntrinsicId id() const               { return _id; }
  int number_of_arguments() const                { return _args->length(); }
  Value argument_at(int i) const                 { return _args->at(i); }
  ValueStack* lock_stack() const                 { return _lock_stack; }

  bool has_receiver() const                      { return (_recv != NULL); }
  Value receiver() const                         { assert(has_receiver(), "must have receiver"); return _recv; }
  bool preserves_state() const                   { return check_flag(PreservesStateFlag); }

  // generic
  virtual bool can_trap() const                  { return true; }
  virtual void input_values_do(void f(Value*)) {
    StateSplit::input_values_do(f);
    for (int i = 0; i < _args->length(); i++) f(_args->adr_at(i));
  }
};


class LIR_List;

LEAF(BlockBegin, StateSplit)
 private:
  static int _next_block_id;                     // the block counter

  int        _block_id;                          // the unique block id
  int        _weight;                            // the block weight, used for block ordering
  int        _flags;                             // the flags associated with this block
  BlockEnd*  _end;                               // the last instruction of this block
  BlockList  _exception_handlers;                // the exception handlers potentially invoked by this block
  int        _exception_handler_pco;             // if this block is the start of an exception handler,
                                                 // this records the PC offset in the assembly code of the
                                                 // first instruction in this block
  LocalMapping* _local_mapping;                  // the mapping between registers and locals for this block
  Label      _label;                             // the label associated with this block
  LIR_List*  _lir;                               // the low level intermediate representation for this block

#ifndef PRODUCT
  BitMap     _lir_oop_map;                       // Stores state used by LIR_OopMapGenerator;
                                                 // initialized to 0-length
#endif

  int        _loop_index;

  void iterate_preorder (boolArray& mark, BlockClosure* closure);
  void iterate_postorder(boolArray& mark, BlockClosure* closure);

  friend class SuxAndWeightAdjuster;

 public:
  // initialization/counting
  static void initialize()                       { _next_block_id = 0; }
  static int  number_of_blocks()                 { return _next_block_id; }

  // creation
  BlockBegin(int bci)
  : StateSplit(illegalType)
  , _block_id(_next_block_id++)
  , _weight(0)
  , _flags(0)
  , _end(NULL)
  , _exception_handlers(1)
  , _exception_handler_pco(-1)
  , _local_mapping(NULL)
  , _lir(NULL)
  , _loop_index(-1)
#ifndef PRODUCT
  , _lir_oop_map(NULL, 0)
#endif
  {
    set_bci(bci);
  }

  // accessors
  int block_id() const                           { return _block_id; }
  int weight() const                             { return _weight; }
  BlockEnd* end() const                          { return _end; }
  LocalMapping* local_mapping() const            { return _local_mapping; }
  Label* label()                                 { return &_label; }
  LIR_List* lir() const                          { return _lir; }
  int exception_handler_pco() const              { return _exception_handler_pco; }
#ifndef PRODUCT
  BitMap* lir_oop_map()                          { return &_lir_oop_map; }
#endif
  bool enqueued_for_oop_map_gen() const;         // Quick test to avoid duplicated work

  // manipulation
  void set_weight(int start_distance);
  void set_end(BlockEnd* end)                    { _end = end; }
  void set_local_mapping(LocalMapping* local_mapping)     { _local_mapping = local_mapping; }
  void set_lir(LIR_List* lir)                    { _lir = lir; }
  void set_exception_handler_pco(int pco)        { _exception_handler_pco = pco; }
  void set_enqueued_for_oop_map_gen(bool value);

  // exception handlers potentially invoked by this block
  void add_exception_handler(BlockBegin* b);
  int number_of_exception_handlers() const       { return _exception_handlers.length(); }
  BlockBegin* exception_handler_at(int i) const  { return _exception_handlers.at(i); }

  // flags
  enum Flag {
    no_flag                      = 0,
    std_entry_flag               = 1 << 0,
    osr_entry_flag               = 1 << 1,
    exception_entry_flag         = 1 << 2,
    subroutine_entry_flag        = 1 << 3,
    backward_branch_target_flag  = 1 << 4,
    is_on_work_list_flag         = 1 << 5,
    was_visited_flag             = 1 << 6,
    single_precision_flag        = 1 << 7,
    enqueued_for_oop_map_gen_flag= 1 << 8,
    lir_oop_map_gen_reachable_flag= 1 << 9
  };

  void set(Flag f)                               { _flags |= f; }
  void clear(Flag f)                             { _flags &= ~f; }
  bool is_set(Flag f) const                      { return (_flags & f) != 0; }
  bool is_entry_block() const {
    const int entry_mask = std_entry_flag | osr_entry_flag | exception_entry_flag | subroutine_entry_flag;
    return (_flags & entry_mask) != 0;
  }

  // block ordering in final block list (before code generation)
  bool is_after_block(BlockBegin* b)             { return weight() >= b->weight(); }

  // iteration
  void iterate_preorder   (BlockClosure* closure);
  void iterate_postorder  (BlockClosure* closure);

  void block_values_do(void f(Value*));

  // loops
  void set_loop_index(int ix)                    { _loop_index = ix;        }
  int  loop_index() const                        { return _loop_index;      }

  // merging
  bool try_join(ValueStack* state);              // same as join, but returns false if join fails
  void join(ValueStack* state)                   { bool b = try_join(state); assert(b, "join failed"); }
  void resolve_substitution();                   // resolve value references to substituted values
                                                 // for all instructions in this block

  // debugging
  void print_block()                             PRODUCT_RETURN;
  void print_block(InstructionPrinter& ip, bool live_only = false) PRODUCT_RETURN;
};


BASE(BlockEnd, StateSplit)
 private:
  BlockList*  _sux;
  ValueStack* _state_before;

 protected:
  BlockList* sux() const                         { return _sux; }

  void set_sux(BlockList* sux) {
#ifdef ASSERT
    assert(sux != NULL, "sux must exist");
    for (int i = sux->length() - 1; i >= 0; i--) assert(sux->at(i) != NULL, "sux must exist");
#endif
    _sux = sux;
  }

 public:
  // creation
  BlockEnd(ValueType* type, bool is_safepoint)
  : StateSplit(type)
  , _sux(NULL)
  , _state_before(NULL) {
    set_flag(IsSafepointFlag, is_safepoint);
  }

  // accessors
  ValueStack* state_before() const               { return _state_before; }
  bool is_safepoint() const                      { return check_flag(IsSafepointFlag); }

  // manipulation
  void set_state_before(ValueStack* state)       { _state_before = state; }

  // generic
  virtual void other_values_do(void f(Value*));

  // successors
  int number_of_sux() const                      { return _sux != NULL ? _sux->length() : 0; }
  BlockBegin* sux_at(int i) const                { return _sux->at(i); }
  BlockBegin* default_sux() const                { return sux_at(number_of_sux() - 1); }
  BlockBegin** addr_sux_at(int i) const          { return _sux->adr_at(i); }
  int sux_index(BlockBegin* sux) const           { return _sux->find(sux); }
  void substitute_sux(BlockBegin* old_sux, BlockBegin* new_sux);
};


LEAF(Goto, BlockEnd)
 public:
  // creation
  Goto(BlockBegin* sux, bool is_safepoint) : BlockEnd(illegalType, is_safepoint) {
    BlockList* s = new BlockList(1);
    s->append(sux);
    set_sux(s);
  }

};


LEAF(If, BlockEnd)
 private:
  Value       _x;
  Condition   _cond;
  Value       _y;
 public:
  // creation
  // unordered_is_true is valid for float/double compares only
  If(Value x, Condition cond, bool unordered_is_true, Value y, BlockBegin* tsux, BlockBegin* fsux, ValueStack* state_before, bool is_safepoint)
  : BlockEnd(illegalType, is_safepoint)
  , _x(x)
  , _cond(cond)
  , _y(y)
  {
    ASSERT_VALUES
    set_flag(UnorderedIsTrueFlag, unordered_is_true);
    assert(x->type()->tag() == y->type()->tag(), "types must match");
    BlockList* s = new BlockList(2);
    s->append(tsux);
    s->append(fsux);
    set_sux(s);
    set_state_before(state_before);
  }

  // accessors
  Value x() const                                { return _x; }
  Condition cond() const                         { return _cond; }
  bool unordered_is_true() const                 { return check_flag(UnorderedIsTrueFlag); }
  Value y() const                                { return _y; }
  BlockBegin* sux_for(bool is_true) const        { return sux_at(is_true ? 0 : 1); }
  BlockBegin* tsux() const                       { return sux_for(true); }
  BlockBegin* fsux() const                       { return sux_for(false); }
  BlockBegin* usux() const                       { return sux_for(unordered_is_true()); }

  // manipulation
  void swap_operands() {
    Value t = _x; _x = _y; _y = t;
    _cond = mirror(_cond);
  }

  void swap_sux() {
    assert(number_of_sux() == 2, "wrong number of successors");
    BlockList* s = sux();
    BlockBegin* t = s->at(0); s->at_put(0, s->at(1)); s->at_put(1, t);
    _cond = negate(_cond);
    set_flag(UnorderedIsTrueFlag, !check_flag(UnorderedIsTrueFlag));
  }

  // generic
  virtual void input_values_do(void f(Value*))   { BlockEnd::input_values_do(f); f(&_x); f(&_y); }
};


LEAF(IfInstanceOf, BlockEnd)
 private:
  ciKlass* _klass;
  Value    _obj;
  bool     _test_is_instance;                    // jump if instance
  int      _instanceof_bci;

 public:
  IfInstanceOf(ciKlass* klass, Value obj, bool test_is_instance, int instanceof_bci, BlockBegin* tsux, BlockBegin* fsux)
  : BlockEnd(illegalType, false) // temporary set to false
  , _klass(klass)
  , _obj(obj)
  , _test_is_instance(test_is_instance)
  , _instanceof_bci(instanceof_bci)
  {
    ASSERT_VALUES
    assert(instanceof_bci >= 0, "illegal bci");
    BlockList* s = new BlockList(2);
    s->append(tsux);
    s->append(fsux);
    set_sux(s);
  }

  // accessors
  //
  // Note 1: If test_is_instance() is true, IfInstanceOf tests if obj *is* an
  //         instance of klass; otherwise it tests if it is *not* and instance
  //         of klass.
  //
  // Note 2: IfInstanceOf instructions are created by combining an InstanceOf
  //         and an If instruction. The IfInstanceOf bci() corresponds to the
  //         bci that the If would have had; the (this->) instanceof_bci() is
  //         the bci of the original InstanceOf instruction.
  ciKlass* klass() const                         { return _klass; }
  Value obj() const                              { return _obj; }
  int instanceof_bci() const                     { return _instanceof_bci; }
  bool test_is_instance() const                  { return _test_is_instance; }
  BlockBegin* sux_for(bool is_true) const        { return sux_at(is_true ? 0 : 1); }
  BlockBegin* tsux() const                       { return sux_for(true); }
  BlockBegin* fsux() const                       { return sux_for(false); }

  // manipulation
  void swap_sux() {
    assert(number_of_sux() == 2, "wrong number of successors");
    BlockList* s = sux();
    BlockBegin* t = s->at(0); s->at_put(0, s->at(1)); s->at_put(1, t);
    _test_is_instance = !_test_is_instance;
  }

  // generic
  virtual void input_values_do(void f(Value*))   { BlockEnd::input_values_do(f); f(&_obj); }
};


BASE(Switch, BlockEnd)
 private:
  Value       _tag;

 public:
  // creation
  Switch(Value tag, BlockList* sux, bool is_safepoint)
  : BlockEnd(illegalType, is_safepoint)
  , _tag(tag) {
    ASSERT_VALUES
    set_sux(sux);
  }

  // accessors
  Value tag() const                              { return _tag; }
  int length() const                             { return number_of_sux() - 1; }

  // generic
  virtual void input_values_do(void f(Value*))   { BlockEnd::input_values_do(f); f(&_tag); }
};


LEAF(TableSwitch, Switch)
 private:
  int _lo_key;

 public:
  // creation
  TableSwitch(Value tag, BlockList* sux, int lo_key, ValueStack* state_before, bool is_safepoint)
  : Switch(tag, sux, is_safepoint)
  , _lo_key(lo_key) { set_state_before(state_before); }

  // accessors
  int lo_key() const                             { return _lo_key; }
  int hi_key() const                             { return _lo_key + length() - 1; }
};


LEAF(LookupSwitch, Switch)
 private:
  intArray* _keys;

 public:
  // creation
  LookupSwitch(Value tag, BlockList* sux, intArray* keys, ValueStack* state_before, bool is_safepoint)
  : Switch(tag, sux, is_safepoint)
  , _keys(keys) {
    assert(keys != NULL, "keys must exist");
    assert(keys->length() == length(), "sux & keys have incompatible lengths");
    set_state_before(state_before);
  }

  // accessors
  int key_at(int i) const                        { return _keys->at(i); }
};


LEAF(Return, BlockEnd)
 private:
  Value _result;
  int   _monitor_no;

 public:
  // creation
  Return(Value result, int monitor_no) : BlockEnd(result == NULL ? voidType : result->type(), true), _result(result), _monitor_no(monitor_no) {}

  // accessors
  Value result() const                           { return _result; }
  bool has_result() const                        { return result() != NULL; }
  int monitor_no() const                         { return _monitor_no; }
  bool is_synchronized() const                   { return monitor_no() >= 0; }

  // generic
  virtual void input_values_do(void f(Value*)) {
    BlockEnd::input_values_do(f);
    if (has_result()) f(&_result);
  }
};


LEAF(Throw, BlockEnd)
 private:
  Value _exception;

 public:
  // creation
  Throw(Value exception) : BlockEnd(illegalType, true), _exception(exception) {
    ASSERT_VALUES
  }

  // accessors
  Value exception() const                        { return _exception; }

  // generic
  virtual bool can_trap() const                  { return true; }
  virtual void input_values_do(void f(Value*))   { BlockEnd::input_values_do(f); f(&_exception); }
  virtual void state_values_do(void f(Value*));
};


LEAF(Base, BlockEnd)
 public:
  // creation
  Base(BlockBegin* std_entry, BlockBegin* osr_entry) : BlockEnd(illegalType, false) {
    assert(std_entry->is_set(BlockBegin::std_entry_flag), "std entry must be flagged");
    assert(osr_entry == NULL || osr_entry->is_set(BlockBegin::osr_entry_flag), "osr entry must be flagged");
    BlockList* s = new BlockList(2);
    if (osr_entry != NULL) s->append(osr_entry);
    s->append(std_entry); // must be default sux!
    set_sux(s);
  }

  // accessors
  BlockBegin* std_entry() const                  { return default_sux(); }
  BlockBegin* osr_entry() const                  { return number_of_sux() < 2 ? NULL : sux_at(0); }
};


BASE(UnsafeOp, Instruction)
 private:
  Value _unsafe;            // The sun.misc.Unsafe object
  BasicType _basic_type;    // ValueType can not express byte-sized integers
  ValueStack* _lock_stack;

 protected:
  // creation
  UnsafeOp(BasicType basic_type, Value unsafe, bool is_put, ValueStack* lock_stack)
  : Instruction(is_put ? voidType : as_ValueType(basic_type))
  , _basic_type(basic_type)
  , _unsafe(unsafe)
  , _lock_stack(lock_stack)
  {
    //Note:  Unsafe ops are not not guaranteed to throw NPE.
    ASSERT_VALUES
    // Convservatively, Unsafe operations must be pinned though we could be
    // looser about this if we wanted to..
    pin();
  }

 public:
  // accessors
  Value unsafe()                                 { return _unsafe; }
  BasicType basic_type()                         { return _basic_type; }
  ValueStack* lock_stack() const                 { return _lock_stack; }

  // generic
  virtual void input_values_do(void f(Value*))   { f(&_unsafe); }
};


BASE(UnsafeRawOp, UnsafeOp)
 private:
  Value _base;                                   // Base address (a Java long)
  Value _index;                                  // Index if computed by optimizer; initialized to NULL
  int   _log2_scale;                             // Scale factor: 0, 1, 2, or 3.
                                                 // Indicates log2 of number of bytes (1, 2, 4, or 8)
                                                 // to scale index by.

 protected:
  UnsafeRawOp(BasicType basic_type, Value unsafe, Value addr, bool is_put, ValueStack* lock_stack)
  : UnsafeOp(basic_type, unsafe, is_put, lock_stack)
  , _base(addr)
  , _index(NULL)
  , _log2_scale(0)
  {
    // Can not use ASSERT_VALUES because index may be NULL
    assert(addr != NULL && addr->type()->is_long(), "just checking");
  }

  UnsafeRawOp(BasicType basic_type, Value unsafe, Value base, Value index, int log2_scale, bool is_put, ValueStack* lock_stack)
  : UnsafeOp(basic_type, unsafe, is_put, lock_stack)
  , _base(base)
  , _index(index)
  , _log2_scale(log2_scale)
  {
  }

 public:
  // accessors
  Value base()                                   { return _base; }
  Value index()                                  { return _index; }
  bool  has_index()                              { return (_index != NULL); }
  int   log2_scale()                             { return _log2_scale; }

  // setters
  void set_base (Value base)                     { _base  = base; }
  void set_index(Value index)                    { _index = index; }
  void set_log2_scale(int log2_scale)            { _log2_scale = log2_scale; }

  // generic
  virtual void input_values_do(void f(Value*))   { UnsafeOp::input_values_do(f);
                                                   f(&_base);
                                                   if (has_index()) f(&_index); }
};


LEAF(UnsafeGetRaw, UnsafeRawOp)
 public:
  UnsafeGetRaw(BasicType basic_type, Value unsafe, Value addr, ValueStack* lock_stack)
  : UnsafeRawOp(basic_type, unsafe, addr, false, lock_stack) {}

  UnsafeGetRaw(BasicType basic_type, Value unsafe, Value base, Value index, int log2_scale, ValueStack* lock_stack)
  : UnsafeRawOp(basic_type, unsafe, base, index, log2_scale, false, lock_stack) {}
};


LEAF(UnsafePutRaw, UnsafeRawOp)
 private:
  Value _value;                                  // Value to be stored

 public:
  UnsafePutRaw(BasicType basic_type, Value unsafe, Value addr, Value value, ValueStack* lock_stack)
  : UnsafeRawOp(basic_type, unsafe, addr, true, lock_stack)
  , _value(value)
  {
    assert(value != NULL, "just checking");
  }

  UnsafePutRaw(BasicType basic_type, Value unsafe, Value base, Value index, int log2_scale, Value value, ValueStack* lock_stack)
  : UnsafeRawOp(basic_type, unsafe, base, index, log2_scale, true, lock_stack)
  , _value(value)
  {
    assert(value != NULL, "just checking");
  }

  // accessors
  Value value()                                  { return _value; }

  // generic
  virtual void input_values_do(void f(Value*))   { UnsafeRawOp::input_values_do(f);
                                                   f(&_value); }
};


BASE(UnsafeObjectOp, UnsafeOp)
 private:
  Value _object;                                 // Object to be fetched from or mutated
  Value _offset;                                 // Offset within object
  bool  _is_volatile;                            // true if volatile - dl/JSR166
 public:
  UnsafeObjectOp(BasicType basic_type, Value unsafe, Value object, Value offset, bool is_put, ValueStack* lock_stack, bool is_volatile)
    : UnsafeOp(basic_type, unsafe, is_put, lock_stack), _object(object), _offset(offset), _is_volatile(is_volatile)
  {
    ASSERT_VALUES
  }

  // accessors
  Value object()                                 { return _object; }
  Value offset()                                 { return _offset; }
  bool  is_volatile()                            { return _is_volatile; }
  // generic
  virtual void input_values_do(void f(Value*))   { UnsafeOp::input_values_do(f);
                                                   f(&_object);
                                                   f(&_offset); }
};


LEAF(UnsafeGetObject, UnsafeObjectOp)
 public:
  UnsafeGetObject(BasicType basic_type, Value unsafe, Value object, Value offset, ValueStack* lock_stack, bool is_volatile)
  : UnsafeObjectOp(basic_type, unsafe, object, offset, false, lock_stack, is_volatile) {}

};


LEAF(UnsafePutObject, UnsafeObjectOp)
 private:
  Value _value;                                  // Value to be stored
 public:
  UnsafePutObject(BasicType basic_type, Value unsafe, Value object, Value offset, Value value, ValueStack* lock_stack, bool is_volatile)
  : UnsafeObjectOp(basic_type, unsafe, object, offset, true, lock_stack, is_volatile)
    , _value(value)
  {
    ASSERT_VALUES
  }

  // accessors
  Value value()                                  { return _value; }

  // generic
  virtual void input_values_do(void f(Value*))   { UnsafeObjectOp::input_values_do(f);
                                                   f(&_value); }
};


#undef ASSERT_VALUES
