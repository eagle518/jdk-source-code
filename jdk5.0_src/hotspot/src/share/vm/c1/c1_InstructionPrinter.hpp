#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_InstructionPrinter.hpp	1.56 03/12/23 16:39:08 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

#ifndef PRODUCT
class InstructionPrinter: public InstructionVisitor {
 private:
  enum LayoutConstants {
    bci_pos   =  2,
    use_pos   =  7,
    temp_pos  = 12,
    instr_pos = 19,
    end_pos   = 60
  };

 public:
  // helpers
  static const char* basic_type_name(BasicType type);
  static const char* cond_name(If::Condition cond);
  static const char* op_name(Bytecodes::Code op);

  // type-specific print functions
  void print_klass(ciKlass* klass);
  void print_object(Value obj);

  // generic print functions
  void print_temp(Value value);
  void print_local(AccessLocal* local);
  void print_field(AccessField* field);
  void print_indexed(AccessIndexed* indexed);
  void print_monitor(AccessMonitor* monitor);
  void print_op2(Op2* instr);
  void print_value(Value value);
  void print_instr(Instruction* instr);
  void print_stack(ValueStack* stack);
  void print_inline_level(BlockBegin* block);
  void print_unsafe_op(UnsafeOp* op, const char* name);
  void print_unsafe_raw_op(UnsafeRawOp* op, const char* name);
  void print_unsafe_object_op(UnsafeObjectOp* op, const char* name);

  // line printing of instructions
  void fill_to(int pos, char filler = ' ');
  void print_head();
  void print_line(Instruction* instr);

  // visitor functionality
  virtual void do_Phi            (Phi*             x);
  virtual void do_Local          (Local*           x);
  virtual void do_Constant       (Constant*        x);
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
  virtual void do_IfInstanceOf   (IfInstanceOf*    x);
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
#endif // PRODUCT
