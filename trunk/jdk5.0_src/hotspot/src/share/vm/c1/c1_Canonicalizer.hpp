#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_Canonicalizer.hpp	1.20 03/12/23 16:38:59 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

class Canonicalizer: InstructionVisitor {
 private:
  Instruction* _canonical;
  int _bci;

  void set_canonical(Value x);
  void set_bci(int bci)                          { _bci = bci; }
  void set_constant(jint x)                      { set_canonical(new Constant(new IntConstant(x))); }
  void set_constant(jlong x)                     { set_canonical(new Constant(new LongConstant(x))); }
  void set_constant(jfloat x)                    { set_canonical(new Constant(new FloatConstant(x))); }
  void set_constant(jdouble x)                   { set_canonical(new Constant(new DoubleConstant(x))); }
  void move_const_to_right(Op2* x);
  void do_Op2(Op2* x);
  void do_UnsafeRawOp(UnsafeRawOp* x);

  void unsafe_raw_match(UnsafeRawOp* x,
                        Instruction** base,
                        Instruction** index,
                        int* scale);

 public:
  Canonicalizer(Value x, int bci)                { _canonical = x; _bci = bci; if (CanonicalizeNodes) x->visit(this); }
  Value canonical() const                        { return _canonical; }
  int bci() const                                { return _bci; }

  virtual void do_Phi            (Phi*             x);
  virtual void do_Constant       (Constant*        x);
  virtual void do_Local          (Local*           x);
  virtual void do_LoadLocal      (LoadLocal*       x);
  virtual void do_StoreLocal     (StoreLocal*      x);
  virtual void do_LoadField      (LoadField*       x);
  virtual void do_StoreField     (StoreField*      x);
  virtual void do_ArrayLength    (ArrayLength*     x);
  virtual void do_LoadIndexed    (LoadIndexed*     x);
  virtual void do_StoreIndexed   (StoreIndexed*    x);
  virtual void do_CachingChange  (CachingChange*   x);
  virtual void do_NegateOp       (NegateOp*        x);
  virtual void do_ArithmeticOp   (ArithmeticOp*    x);
  virtual void do_ShiftOp        (ShiftOp*         x);
  virtual void do_LogicOp        (LogicOp*         x);
  virtual void do_CompareOp      (CompareOp*       x);
  virtual void do_IfOp           (IfOp*            x);
  virtual void do_IfInstanceOf   (IfInstanceOf*    x);
  virtual void do_Convert        (Convert*         x);
  virtual void do_NullCheck      (NullCheck*       x);
  virtual void do_Invoke         (Invoke*          x);
  virtual void do_NewInstance    (NewInstance*     x);
  virtual void do_NewTypeArray   (NewTypeArray*    x);
  virtual void do_NewObjectArray (NewObjectArray*  x);
  virtual void do_NewMultiArray  (NewMultiArray*   x);
  virtual void do_CheckCast      (CheckCast*       x);
  virtual void do_InstanceOf     (InstanceOf*      x);
  virtual void do_MonitorEnter   (MonitorEnter*    x);
  virtual void do_MonitorExit    (MonitorExit*     x);
  virtual void do_Intrinsic      (Intrinsic*       x);
  virtual void do_BlockBegin     (BlockBegin*      x);
  virtual void do_Goto           (Goto*            x);
  virtual void do_If             (If*              x);
  virtual void do_TableSwitch    (TableSwitch*     x);
  virtual void do_LookupSwitch   (LookupSwitch*    x);
  virtual void do_Return         (Return*          x);
  virtual void do_Throw          (Throw*           x);
  virtual void do_Base           (Base*            x);
  virtual void do_UnsafeGetRaw   (UnsafeGetRaw*    x);
  virtual void do_UnsafePutRaw   (UnsafePutRaw*    x);
  virtual void do_UnsafeGetObject(UnsafeGetObject* x);
  virtual void do_UnsafePutObject(UnsafePutObject* x);
};

