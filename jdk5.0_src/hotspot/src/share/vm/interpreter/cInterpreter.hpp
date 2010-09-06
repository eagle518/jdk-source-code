#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)cInterpreter.hpp	1.11 03/12/23 16:40:37 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#ifdef CC_INTERP

// CVM definitions find hotspot equivalents...

union VMJavaVal64 {
    jlong   l;
    jdouble d;
    uint32_t      v[2];
};

class JavaSlot : public AllStatic {
  // Accessors convert from JVM slot definition (i.e. JVM spec) representation
  // to native VM representation
public:
  // The implementations are really platform dependent but this works for now
  // In real life on LP64 machines probably have assemble double/longs since slot
  // size is wider than 32bits (unless we get tricky and pack into single slot
  // and leave other slot vacant).
  //
  static inline jdouble Double(address p);
  static inline jint Int(address p);
  static inline jfloat Float(address p);
  static inline jlong Long(address p);
  // STACK_ICELL
  static inline oop Object(address p);
  static inline address Address(address p);
  static inline intptr_t Raw(address p);

  // For copying an internal vm representation to a slot
  static inline void set_Address(address value, address p);
  static inline void set_Int(jint value, address p);
  static inline void set_Float(jfloat value, address p);
  static inline void set_Object(oop value, address p);

  // For copying a slot representation to another slot
  static inline void set_Raw(address value, address p);
  static inline void set_Double(address value, address p);
  static inline void set_Long(address value, address p);
};

class JavaLocals VALUE_OBJ_CLASS_SPEC {
private:
  intptr_t* _base;  // Base of locals on stack
public:

  // Accessors convert from local slot representation to native VM representation
  // double/long are claim two slots in the jvm definition and are always accessed
  // via the low numbered slot. Depending on endian issues and slot size issues
  // the actual data can be stored in either slot (LP64 first class implementations)
  // or both slots with high/low 32bits in either order depending on native representation
  // The implementations are therefore highly platform specific
  //
  // ia32 implementation - locals is array on the stack with indices going from 0..-(locals-1)
  // because the locals are actually overlayed on the parameters to the call on the
  // expression stack which also grows down. Strange but true...
  //
  inline jdouble Double(int slot);
  inline jint Int(int slot);
  inline jfloat Float(int slot);
  inline jlong Long(int slot);

  inline oop Object(int slot);
  inline address Address(int slot);
  inline intptr_t Raw(int slot);

  // For copying an internal vm representation to a slot
  inline void set_Address(address value, int slot);
  inline void set_Int(jint value, int slot);
  inline void set_Float(jfloat value, int slot);
  inline void set_Object(oop value, int slot);
  inline void set_Double(jdouble value, int slot);
  inline void set_Long(jlong value, int slot);

  // For copying a slot representation to another slot
  inline void set_Raw(address value, int slot);
  inline void set_Double(address value, int slot);
  inline void set_Long(address value, int slot);

  // Return the address of the slot representation
  inline address Double_At(int slot);
  inline address Long_At(int slot);
  inline address Raw_At(int slot);

  inline void Locals(intptr_t* _new_base);
  inline intptr_t* base(void);
  inline intptr_t** base_addr(void);
   // pd stuff here
};

class JavaStack VALUE_OBJ_CLASS_SPEC {
private:
  intptr_t* _tos;
public:

  // Stack is slot number based and all references down the stack are automagically adjusted
  // from a ordering where index is always the "topmost" slot number that the item would be in.
  // The topmost slot on the expression stack that is actually holding a value is always -1,
  // the rub comes when we are pushing a new double/long on to the stack in those instances
  // since these items take up two slots on the conceptual stack a reference to them before
  // the stack top is adjusted will be slot 1, while for single slot items it will be slot 0.
  // The slot values are automagically converted so that endian, packing, and stack direction
  // issues in the actual implementation are always hidden.
  //
  // This allows us to pack longs/doubles into a single slot if they are wide enough but still
  // use up the jvm spec. prescribed number of slots for them. This lets us use native formats
  // in the slots and not have to assemble/disassemble on LP64 machines where the slots are
  // wide enough to hold them.
  //
  // The noticable difference between the implementation of the JavaStack and JavaLocals is
  // how the slots are referenced. The implicit representation of the conceptual jvm 
  // expression stack in the interpreter is that of elements growing down. So the topmost
  // expression stack slot is at offset -1, next most recent at -2, etc. So except for the
  // item currently being pushed (0 for single slots, 1 for double/long slots) all offsets
  // are negative. This can cause address calculations to look different betwen the JavaStack
  // implementation and the JavaLocals implementation depending on whether the stack grows
  // up/down and how the JavaLocals array is represented.

  // Accessors convert from stack representation to native VM representation

  inline jdouble Double(int offset);
  inline jint Int(int offset);
  inline jfloat Float(int offset);
  inline jlong Long(int offset);
  // STACK_ICELL
  inline oop Object(int offset);
  inline address Address(int offset);
  inline intptr_t Raw(int offset);

  // For copying an internal vm representation to a slot
  inline void set_Address(address value, int offset);
  inline void set_Int(jint value, int offset);
  inline void set_Float(jfloat value, int offset);
  inline void set_Object(oop value, int offset);
  inline void set_Double(jdouble value, int offset);
  inline void set_Long(jlong value, int offset);

  // For copying a slot representation to a stack location (offset)
  inline void set_Raw(address value, int offset);
  inline void set_Double(address value, int offset);
  inline void set_Long(address value, int offset);

  // Return the address of the slot representation
  inline address Double_At(int offset);
  inline address Long_At(int offset);
  inline address Raw_At(int offset);

  // Stack grows down
  inline void Pop(int count);
  inline void Push(int count);
  inline void Adjust(int count);
  // QQQ We hate these next two
  inline void Reset(intptr_t* base);
  inline void Tos(intptr_t* new_tos);
  inline intptr_t* get_Tos();
  inline intptr_t* top();
  // pd stuff here
};
//

typedef class cInterpreter* interpreterState;

struct call_message {
    methodOop _callee;               /* method to call during call_method request */
    address   _callee_entry_point;   /* address to jump to for call_method request */
    int       _bcp_advance;          /* size of the invoke bytecode operation */
};

// Result returned to frame manager
union frame_manager_message {
    call_message _to_call;            /* describes callee */
    Bytecodes::Code _return_kind;     /* i_return, a_return, ... */
};

class cInterpreter : StackObj {
friend class AbstractInterpreterGenerator;
friend class InterpreterGenerator;
friend class InterpreterMacroAssembler;
friend class frame;
friend class VMStructs;

public:
    enum messages {
         no_request = 0,            // unused
         initialize,                // Perform one time interpreter initializations (assumes all switches set)
         // status message to C++ interpreter
         method_entry,              // initial method entry to interpreter
         method_resume,             // frame manager response to return_from_method request (assuming a frame to resume)
         deopt_resume,              // returning from a native call into a deopted frame
         deopt_resume2,             // deopt resume as a result of a PopFrame
         got_monitors,              // frame manager response to more_monitors request
         rethrow_exception,         // unwinding and throwing exception
         // requests to frame manager from C++ interpreter
         call_method,               // request for new frame from interpreter, manager responds with method_entry
         return_from_method,        // request from interpreter to unwind, manager responds with method_continue
         retry_method,              // method compiled, retry method entry UNUSED at present
         more_monitors,             // need a new monitor
	 throwing_exception,        // unwind stack and rethrow
	 popping_frame              // unwind call and retry call
    };

private:
    JavaThread*           _thread;        // the vm's java thread pointer
    address               _bcp;           // instruction pointer
    intptr_t*             _locals;        // local variable pointer
    constantPoolCacheOop  _constants;     // constant pool cache
    methodOop             _method;        // method being executed
#ifndef CORE
    DataLayout*           _mdx;           // compiler profiling data for current bytecode
#endif
    intptr_t*             _stack;         // expression stack
    messages              _msg;           // frame manager <-> interpreter message
    frame_manager_message _result;        // result to frame manager
    interpreterState      _prev_link;     // previous interpreter state
    oop                   _native_mirror; // mirror for interpreted native, null otherwise
    // These are likely platform dependent fields
    // jint*  sender_sp;                  // previous stack pointer
    intptr_t*             _stack_base;    // base of expression stack
    intptr_t*             _stack_limit;   // limit of expression stack
    BasicObjectLock*      _monitor_base;  // base of monitors on the native stack


public:
  // Constructor is only used by the initialization step. All other instances are created
  // by the frame manager.
  cInterpreter(messages msg);

#ifndef CORE
//
// Deoptimization support
//
static void layout_interpreterState(interpreterState to_fill,
				    frame* caller,
				    frame* interpreter_frame,
				    methodOop method,
				    intptr_t* locals,
				    intptr_t* stack,
				    intptr_t* stack_base,
				    intptr_t* monitor_base,
				    intptr_t* frame_bottom,
				    bool top_frame);
#endif

/*
 * Generic 32-bit wide "Java slot" definition. This type occurs
 * in operand stacks, Java locals, object fields, constant pools.
 */
union VMJavaVal32 {
    jint     i;
    jfloat   f;
    oop      r;
    uint32_t raw;
};

/*
 * Generic 64-bit Java value definition
 */
union VMJavaVal64 {
    jlong   l;
    jdouble d;
    uint32_t      v[2];
};

/*
 * Generic 32-bit wide "Java slot" definition. This type occurs
 * in Java locals, object fields, constant pools, and
 * operand stacks (as a CVMStackVal32).
 */
typedef union VMSlotVal32 {
    VMJavaVal32    j;     /* For "Java" values */
    address        a;     /* a return created by jsr or jsr_w */
} VMSlotVal32;


/*
 * Generic 32-bit wide stack slot definition.
 */
union VMStackVal32 {
    VMJavaVal32    j;     /* For "Java" values */
    VMSlotVal32    s;     /* any value from a "slot" or locals[] */
};

inline JavaThread* thread() { return _thread; }

inline address bcp() { return _bcp; }
inline void set_bcp(address new_bcp) { _bcp = new_bcp; }

inline intptr_t* locals() { return _locals; }
// inline void set_locals(JavaLocals new_locals) { _locals = new_locals; }

inline constantPoolCacheOop constants() { return _constants; }
inline methodOop method() { return _method; }
#ifndef CORE
inline DataLayout* mdx() { return _mdx; }
#endif

inline messages msg() { return _msg; }
inline void set_msg(messages new_msg) { _msg = new_msg; }

inline methodOop callee() { return _result._to_call._callee; }
inline void set_callee(methodOop new_callee) { _result._to_call._callee = new_callee; }
inline void set_callee_entry_point(address entry) { _result._to_call._callee_entry_point = entry; }
inline int bcp_advance() { return _result._to_call._bcp_advance; }
inline void set_bcp_advance(int count) { _result._to_call._bcp_advance = count; }

inline void set_return_kind(Bytecodes::Code kind) { _result._return_kind = kind; }

inline interpreterState prev() { return _prev_link; }

inline intptr_t* stack() { return _stack; }
inline void set_stack(intptr_t* new_stack) { _stack = new_stack; }
#if 0
// yuch QQQ
inline void set_stack_raw(intptr_t* newsp) { ShouldNotReachHere(); }
#endif


inline intptr_t* stack_base() { return _stack_base; }
inline intptr_t* stack_limit() { return _stack_limit; }

inline BasicObjectLock* monitor_base() { return _monitor_base; }

/*
 * 64-bit Arithmetic:
 *
 * The functions below follow the semantics of the
 * ladd, land, ldiv, lmul, lor, lxor, and lrem bytecodes,
 * respectively.
 */

static jlong VMlongAdd(jlong op1, jlong op2);
static jlong VMlongAnd(jlong op1, jlong op2);
static jlong VMlongDiv(jlong op1, jlong op2);
static jlong VMlongMul(jlong op1, jlong op2);
static jlong VMlongOr (jlong op1, jlong op2);
static jlong VMlongSub(jlong op1, jlong op2);
static jlong VMlongXor(jlong op1, jlong op2);
static jlong VMlongRem(jlong op1, jlong op2);

/*
 * Shift:
 *
 * The functions below follow the semantics of the
 * lushr, lshl, and lshr bytecodes, respectively.
 */

static jlong VMlongUshr(jlong op1, jint op2);
static jlong VMlongShl (jlong op1, jint op2);
static jlong VMlongShr (jlong op1, jint op2);

/*
 * Unary:
 *
 * Return the negation of "op" (-op), according to
 * the semantics of the lneg bytecode.
 */

static jlong VMlongNeg(jlong op);

/*
 * Return the complement of "op" (~op)
 */

static jlong VMlongNot(jlong op);


/*
 * Comparisons to 0:
 */

static int32_t VMlongLtz(jlong op);     /* op <= 0 */
static int32_t VMlongGez(jlong op);     /* op >= 0 */
static int32_t VMlongEqz(jlong op);     /* op == 0 */

/*
 * Between operands:
 */

static int32_t VMlongEq(jlong op1, jlong op2);    /* op1 == op2 */
static int32_t VMlongNe(jlong op1, jlong op2);    /* op1 != op2 */
static int32_t VMlongGe(jlong op1, jlong op2);    /* op1 >= op2 */
static int32_t VMlongLe(jlong op1, jlong op2);    /* op1 <= op2 */
static int32_t VMlongLt(jlong op1, jlong op2);    /* op1 <  op2 */
static int32_t VMlongGt(jlong op1, jlong op2);    /* op1 >  op2 */

/*
 * Comparisons (returning an jint value: 0, 1, or -1)
 *
 * Between operands:
 *
 * Compare "op1" and "op2" according to the semantics of the
 * "lcmp" bytecode.
 */

static int32_t VMlongCompare(jlong op1, jlong op2);

/*
 * Convert int to long, according to "i2l" bytecode semantics
 */
static jlong VMint2Long(jint val);

/*
 * Convert long to int, according to "l2i" bytecode semantics
 */
static jint VMlong2Int(jlong val);

/*
 * Convert long to float, according to "l2f" bytecode semantics
 */
static jfloat VMlong2Float(jlong val);

/*
 * Convert long to double, according to "l2d" bytecode semantics
 */
static jdouble VMlong2Double(jlong val);

/*
 * Java floating-point float value manipulation.
 *
 * The result argument is, once again, an lvalue.
 *
 * Arithmetic:
 *
 * The functions below follow the semantics of the
 * fadd, fsub, fmul, fdiv, and frem bytecodes,
 * respectively.
 */

static jfloat VMfloatAdd(jfloat op1, jfloat op2);
static jfloat VMfloatSub(jfloat op1, jfloat op2);
static jfloat VMfloatMul(jfloat op1, jfloat op2);
static jfloat VMfloatDiv(jfloat op1, jfloat op2);
static jfloat VMfloatRem(jfloat op1, jfloat op2);

/*
 * Unary:
 *
 * Return the negation of "op" (-op), according to
 * the semantics of the fneg bytecode.
 */

static jfloat VMfloatNeg(jfloat op);

/*
 * Comparisons (returning an int value: 0, 1, or -1)
 *
 * Between operands:
 *
 * Compare "op1" and "op2" according to the semantics of the
 * "fcmpl" (direction is -1) or "fcmpg" (direction is 1) bytecodes.
 */

static int32_t VMfloatCompare(jfloat op1, jfloat op2,
                              int32_t direction);
/*
 * Conversion:
 */

/*
 * Convert float to double, according to "f2d" bytecode semantics
 */

static jdouble VMfloat2Double(jfloat op);

/*
 ******************************************
 * Java double floating-point manipulation.
 ******************************************
 *
 * The result argument is, once again, an lvalue.
 *
 * Conversions:
 */

/*
 * Convert double to int, according to "d2i" bytecode semantics
 */

static jint VMdouble2Int(jdouble val);

/*
 * Convert double to float, according to "d2f" bytecode semantics
 */

static jfloat VMdouble2Float(jdouble val);

/*
 * Convert int to double, according to "i2d" bytecode semantics
 */

static jdouble VMint2Double(jint val);

/*
 * Arithmetic:
 *
 * The functions below follow the semantics of the
 * dadd, dsub, ddiv, dmul, and drem bytecodes, respectively.
 */

static jdouble VMdoubleAdd(jdouble op1, jdouble op2);
static jdouble VMdoubleSub(jdouble op1, jdouble op2);
static jdouble VMdoubleDiv(jdouble op1, jdouble op2);
static jdouble VMdoubleMul(jdouble op1, jdouble op2);
static jdouble VMdoubleRem(jdouble op1, jdouble op2);

/*
 * Unary:
 *
 * Return the negation of "op" (-op), according to
 * the semantics of the dneg bytecode.
 */

static jdouble VMdoubleNeg(jdouble op);

/*
 * Comparisons (returning an int32_t value: 0, 1, or -1)
 *
 * Between operands:
 *
 * Compare "op1" and "op2" according to the semantics of the
 * "dcmpl" (direction is -1) or "dcmpg" (direction is 1) bytecodes.
 */

static int32_t VMdoubleCompare(jdouble op1, jdouble op2, int32_t direction);

/*
 * Copy two typeless 32-bit words from one location to another.
 * This is semantically equivalent to:
 * 
 * to[0] = from[0];
 * to[1] = from[1];
 *
 * but this interface is provided for those platforms that could
 * optimize this into a single 64-bit transfer.
 */

static void VMmemCopy64(uint32_t to[2], const uint32_t from[2]);


// Arithmetic operations

/*
 * Java arithmetic methods. 
 * The functions below follow the semantics of the
 * iadd, isub, imul, idiv, irem, iand, ior, ixor,
 * and ineg bytecodes, respectively.
 */

static jint VMintAdd(jint op1, jint op2);
static jint VMintSub(jint op1, jint op2);
static jint VMintMul(jint op1, jint op2);
static jint VMintDiv(jint op1, jint op2);
static jint VMintRem(jint op1, jint op2);
static jint VMintAnd(jint op1, jint op2);
static jint VMintOr (jint op1, jint op2);
static jint VMintXor(jint op1, jint op2);

/*
 * Shift Operation:
 * The functions below follow the semantics of the
 * iushr, ishl, and ishr bytecodes, respectively.
 */

static jint VMintUshr(jint op, jint num);
static jint VMintShl (jint op, jint num);
static jint VMintShr (jint op, jint num);

/*
 * Unary Operation:
 *
 * Return the negation of "op" (-op), according to
 * the semantics of the ineg bytecode.
 */

static jint VMintNeg(jint op);

/*
 * Int Conversions:
 */

/*
 * Convert int to float, according to "i2f" bytecode semantics
 */

static jfloat VMint2Float(jint val);

/*
 * Convert int to byte, according to "i2b" bytecode semantics
 */

static jbyte VMint2Byte(jint val);

/*
 * Convert int to char, according to "i2c" bytecode semantics
 */

static jchar VMint2Char(jint val);

/*
 * Convert int to short, according to "i2s" bytecode semantics
 */

static jshort VMint2Short(jint val);

/*=========================================================================
 * Bytecode interpreter operations
 *=======================================================================*/

// umm don't like this method modifies its object

// The Interpreter used when 
static void InterpretMethod(interpreterState istate);
// The interpreter used if either JVMDI or JVMPI are enabled
static void InterpretMethodWithChecks(interpreterState istate);
static void End_Of_Interpreter(void);

    // Platform fields/methods 
# include "incls/_cInterpreter_pd.hpp.incl"

}; // cInterpreter
#endif // CC_INTERP
