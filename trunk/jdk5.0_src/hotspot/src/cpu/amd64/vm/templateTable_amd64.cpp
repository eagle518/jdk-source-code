#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)templateTable_amd64.cpp	1.17 03/12/23 16:35:57 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_templateTable_amd64.cpp.incl"

#define __ _masm->

// Platform-dependent initialization

void TemplateTable::pd_initialize()
{
  // No amd64 specific initialization
}

// Address computation: local variables

static inline Address iaddress(int n) 
{ 
  return Address(r14, -n * wordSize); 
}

static inline Address laddress(int n)
{ 
  return iaddress(n + 1); 
}

static inline Address faddress(int n) 
{
  return iaddress(n);
}

static inline Address daddress(int n)
{ 
  return laddress(n);
}

static inline Address aaddress(int n)
{
  return iaddress(n);
}

static inline Address iaddress(Register r)
{
  return Address(r14, r, Address::times_8); 
}

static inline Address laddress(Register r)
{
  return Address(r14, r, Address::times_8, -wordSize);
}

static inline Address faddress(Register r)
{
  return iaddress(r);
}

static inline Address daddress(Register r)
{
  return laddress(r);
}

static inline Address aaddress(Register r)
{ 
  return iaddress(r);
}

static inline Address at_rsp()
{
  return Address(rsp); 
}

static inline Address at_rsp_p1()
{ 
  return Address(rsp,  wordSize);
}

static inline Address at_rsp_p2()
{
  return Address(rsp,  2 * wordSize);
}

static inline Address at_rsp_p3()
{
  return Address(rsp,  3 * wordSize);
}

// Condition conversion
static Assembler::Condition j_not(TemplateTable::Condition cc) 
{
  switch (cc) {
  case TemplateTable::equal        : return Assembler::notEqual;
  case TemplateTable::not_equal    : return Assembler::equal;
  case TemplateTable::less         : return Assembler::greaterEqual;
  case TemplateTable::less_equal   : return Assembler::greater;
  case TemplateTable::greater      : return Assembler::lessEqual;
  case TemplateTable::greater_equal: return Assembler::less;
  }
  ShouldNotReachHere();
  return Assembler::zero;
}


// Miscelaneous helper routines

Address TemplateTable::at_bcp(int offset) 
{
  assert(_desc->uses_bcp(), "inconsistent uses_bcp information");
  return Address(r13, offset);
}

void TemplateTable::patch_bytecode(Bytecodes::Code bytecode, Register bc,
                                   Register scratch,
                                   bool load_bc_into_scratch/*=true*/)
{
  if (!RewriteBytecodes) {
    return;
  }
  // the pair bytecodes have already done the load.
  if (load_bc_into_scratch) {
    __ movl(bc, bytecode);
  }
  Label patch_done;
  if (JvmtiExport::can_post_breakpoint()) {
    Label fast_patch;
    // if a breakpoint is present we can't rewrite the stream directly
    __ movzbl(scratch, at_bcp(0));
    __ cmpl(scratch, Bytecodes::_breakpoint);
    __ jcc(Assembler::notEqual, fast_patch);
    __ get_method(scratch);
    // Let breakpoint table handling rewrite to quicker bytecode 
    __ call_VM(noreg, 
               CAST_FROM_FN_PTR(address, 
                                InterpreterRuntime::set_original_bytecode_at),
               scratch, r13, bc);
    __ jmp(patch_done);
    __ bind(fast_patch);
  }
#ifdef ASSERT
  Label okay;
  __ load_unsigned_byte(scratch, at_bcp(0));
  __ cmpl(scratch, (int) Bytecodes::java_code(bytecode));
  __ jcc(Assembler::equal, okay);
  __ cmpl(scratch, bc);
  __ jcc(Assembler::equal, okay);
  __ stop("patching the wrong bytecode");
  __ bind(okay);
#endif
  // patch bytecode
  __ movb(at_bcp(0), bc);
  __ bind(patch_done);
}


// Individual instructions

void TemplateTable::nop()
{
  transition(vtos, vtos);
  // nothing to do
}

void TemplateTable::shouldnotreachhere()
{
  transition(vtos, vtos);
  __ stop("shouldnotreachhere bytecode");
}

void TemplateTable::aconst_null()
{
  transition(vtos, atos);
  __ xorl(rax, rax);
}

void TemplateTable::iconst(int value)
{
  transition(vtos, itos);
  if (value == 0) {
    __ xorl(rax, rax);
  } else {
    __ movl(rax, value);
  }
}

void TemplateTable::lconst(int value)
{
  transition(vtos, ltos);
  if (value == 0) {
    __ xorl(rax, rax);
  } else {
    __ movl(rax, value);
  }
}

void TemplateTable::fconst(int value) 
{
  transition(vtos, ftos);
  static float one = 1.0f, two = 2.0f;
  switch (value) {
  case 0:
    __ xorps(xmm0, xmm0);
    break;
  case 1:
    __ movss(xmm0, Address((address) &one, relocInfo::none));
    break;
  case 2:
    __ movss(xmm0, Address((address) &two, relocInfo::none));
    break;
  default:
    ShouldNotReachHere();
    break;
  }
}

void TemplateTable::dconst(int value) 
{
  transition(vtos, dtos);
  static double one = 1.0;
  switch (value) {
  case 0:
    __ xorpd(xmm0, xmm0);
    break;
  case 1:
    __ movlpd(xmm0, Address((address) &one, relocInfo::none));
    break;
  default:
    ShouldNotReachHere();
    break;
  }
}

void TemplateTable::bipush() 
{
  transition(vtos, itos);
  __ load_signed_byte(rax, at_bcp(1));
}

void TemplateTable::sipush() 
{
  transition(vtos, itos);
  __ load_unsigned_word(rax, at_bcp(1));
  __ bswapl(rax);
  __ sarl(rax, 16);
}

void TemplateTable::ldc(bool wide) 
{
  transition(vtos, vtos);
  Label call_ldc, notFloat, notClass, Done;

  if (wide) {
    __ get_unsigned_2_byte_index_at_bcp(rbx, 1);
  } else {
    __ load_unsigned_byte(rbx, at_bcp(1));
  }

  __ get_cpool_and_tags(rarg3, rax);
  const int base_offset = constantPoolOopDesc::header_size() * wordSize;
  const int tags_offset = typeArrayOopDesc::header_size(T_BYTE) * wordSize;

  // get type
  __ movzbl(rarg2, Address(rax, rbx, Address::times_1, tags_offset));

  // unresolved string - get the resolved string
  __ cmpl(rarg2, JVM_CONSTANT_UnresolvedString);
  __ jcc(Assembler::equal, call_ldc);

  // unresolved class - get the resolved class
  __ cmpl(rarg2, JVM_CONSTANT_UnresolvedClass);
  __ jcc(Assembler::equal, call_ldc);

  // resolved class - need to call vm to get java mirror of the class
  __ cmpl(rarg2, JVM_CONSTANT_Class);
  __ jcc(Assembler::notEqual, notClass);

  __ bind(call_ldc);
  __ movl(rarg1, wide);
  call_VM(rax, CAST_FROM_FN_PTR(address, InterpreterRuntime::ldc), rarg1);
  __ pushq(rax);
  __ jmp(Done);

  __ bind(notClass);
  __ cmpl(rarg2, JVM_CONSTANT_Float);
  __ jcc(Assembler::notEqual, notFloat);
  // ftos
  __ movss(xmm0, Address(rarg3, rbx, Address::times_8, base_offset));
  __ push_f();
  __ jmp(Done);

  __ bind(notFloat);
#ifdef ASSERT
  { 
    Label L;
    __ cmpl(rarg2, JVM_CONSTANT_Integer);
    __ jcc(Assembler::equal, L);
    __ cmpl(rarg2, JVM_CONSTANT_String);
    __ jcc(Assembler::equal, L);
    __ stop("unexpected tag type in ldc");
    __ bind(L);
  }
#endif
  // atos and itos
  __ cmpl(rarg2, JVM_CONSTANT_Integer);
  __ cmovl(Assembler::equal, 
           rax, Address(rarg3, rbx, Address::times_8, base_offset));
  __ cmovq(Assembler::notEqual,
           rax, Address(rarg3, rbx, Address::times_8, base_offset));
  __ pushq(rax);  // push_i()/push_ptr()

  if (VerifyOops) {
    // String is only oop type we will see here
    __ cmpl(rarg2, JVM_CONSTANT_String);
    __ jcc(Assembler::notEqual, Done);
    __ verify_oop(rax);
  }

  __ bind(Done);
}

void TemplateTable::ldc2_w() 
{
  transition(vtos, vtos);
  Label Long, Done;
  __ get_unsigned_2_byte_index_at_bcp(rbx, 1);

  __ get_cpool_and_tags(rarg3, rax);
  const int base_offset = constantPoolOopDesc::header_size() * wordSize;
  const int tags_offset = typeArrayOopDesc::header_size(T_BYTE) * wordSize;

  // get type
  __ cmpb(Address(rax, rbx, Address::times_1, tags_offset), 
          JVM_CONSTANT_Double);
  __ jcc(Assembler::notEqual, Long);
  // dtos
  __ movlpd(xmm0, Address(rarg3, rbx, Address::times_8, base_offset));
  __ push_d();
  __ jmp(Done);

  __ bind(Long);
  // ltos
  __ movq(rax, Address(rarg3, rbx, Address::times_8, base_offset));
  __ push_l();

  __ bind(Done);
}

void TemplateTable::locals_index(Register reg, int offset) 
{
  __ load_unsigned_byte(reg, at_bcp(offset));
  __ negq(reg);	
}

void TemplateTable::iload()
{
  transition(vtos, itos);
  if (RewriteFrequentPairs) { 
    Label rewrite, done;

    // get next byte
    __ load_unsigned_byte(rbx, 
                          at_bcp(Bytecodes::length_for(Bytecodes::_iload)));
    // if _iload, wait to rewrite to iload2.  We only want to rewrite the
    // last two iloads in a pair.  Comparing against fast_iload means that
    // the next bytecode is neither an iload or a caload, and therefore
    // an iload pair.
    __ cmpl(rbx, Bytecodes::_iload);
    __ jcc(Assembler::equal, done);

    __ cmpl(rbx, Bytecodes::_fast_iload);
    __ movl(rarg3, Bytecodes::_fast_iload2);
    __ jcc(Assembler::equal, rewrite);

    // if _caload, rewrite to fast_icaload
    __ cmpl(rbx, Bytecodes::_caload);
    __ movl(rarg3, Bytecodes::_fast_icaload);
    __ jcc(Assembler::equal, rewrite);

    // rewrite so iload doesn't check again.
    __ movl(rarg3, Bytecodes::_fast_iload);

    // rewrite
    // ecx: fast bytecode
    __ bind(rewrite);
    patch_bytecode(Bytecodes::_iload, rarg3, rbx, false);
    __ bind(done);
  }

  // Get the local value into tos
  locals_index(rbx);
  __ movl(rax, iaddress(rbx));
}

void TemplateTable::fast_iload2()
{
  transition(vtos, itos);
  locals_index(rbx);
  __ movl(rax, iaddress(rbx));
  __ push(itos);
  locals_index(rbx, 3);
  __ movl(rax, iaddress(rbx));
}
  
void TemplateTable::fast_iload()
{
  transition(vtos, itos);
  locals_index(rbx);
  __ movl(rax, iaddress(rbx));
}

void TemplateTable::lload()
{
  transition(vtos, ltos);
  locals_index(rbx);
  __ movq(rax, laddress(rbx));
}

void TemplateTable::fload()
{
  transition(vtos, ftos);
  locals_index(rbx);
  __ movss(xmm0, faddress(rbx));
}

void TemplateTable::dload()
{
  transition(vtos, dtos);
  locals_index(rbx);
  __ movlpd(xmm0, daddress(rbx));
}

void TemplateTable::aload()
{
  transition(vtos, atos);
  locals_index(rbx);
  __ movq(rax, aaddress(rbx));
}

void TemplateTable::locals_index_wide(Register reg) 
{
  __ movl(reg, at_bcp(2));
  __ bswapl(reg);
  __ shrl(reg, 16);
  __ negq(reg);
}

void TemplateTable::wide_iload()
{
  transition(vtos, itos);
  locals_index_wide(rbx);
  __ movl(rax, iaddress(rbx));
}

void TemplateTable::wide_lload()
{
  transition(vtos, ltos);
  locals_index_wide(rbx);
  __ movq(rax, laddress(rbx));
}

void TemplateTable::wide_fload()
{
  transition(vtos, ftos);
  locals_index_wide(rbx);
  __ movss(xmm0, faddress(rbx));
}

void TemplateTable::wide_dload()
{
  transition(vtos, dtos);
  locals_index_wide(rbx);
  __ movlpd(xmm0, daddress(rbx));
}

void TemplateTable::wide_aload()
{
  transition(vtos, atos);
  locals_index_wide(rbx);
  __ movq(rax, aaddress(rbx));
}

void TemplateTable::index_check(Register array, Register index) 
{
  // destroys rbx
  // check array
  __ null_check(array, arrayOopDesc::length_offset_in_bytes());
  // check index
  __ cmpl(index, Address(array, arrayOopDesc::length_offset_in_bytes()));
  if (index != rbx) {
    // ??? convention: move aberrant index into ebx for exception message
    assert(rbx != array, "different registers");
    __ movl(rbx, index);
  }
  __ jcc(Assembler::aboveEqual, 
         Interpreter::_throw_ArrayIndexOutOfBoundsException_entry, 
         relocInfo::none);
}

void TemplateTable::iaload()
{
  transition(itos, itos);
  __ pop_ptr(rarg2);
  // eax: index
  // rarg2: array
  index_check(rarg2, rax); // kills rbx
  __ movl(rax, Address(rarg2, rax, 
                       Address::times_4,
                       arrayOopDesc::base_offset_in_bytes(T_INT)));
}

void TemplateTable::laload()
{
  transition(itos, ltos);
  __ pop_ptr(rarg2);
  // eax: index
  // rarg2: array
  index_check(rarg2, rax); // kills rbx
  __ movq(rax, Address(rarg2, rbx, 
                       Address::times_8, 
                       arrayOopDesc::base_offset_in_bytes(T_LONG)));
}

void TemplateTable::faload()
{
  transition(itos, ftos);
  __ pop_ptr(rarg2);
  // eax: index
  // rarg2: array
  index_check(rarg2, rax); // kills rbx
  __ movss(xmm0, Address(rarg2, rax, 
                         Address::times_4,
                         arrayOopDesc::base_offset_in_bytes(T_FLOAT)));
}

void TemplateTable::daload()
{
  transition(itos, dtos);
  __ pop_ptr(rarg2);
  // eax: index
  // rarg2: array
  index_check(rarg2, rax); // kills rbx
  __ movlpd(xmm0, Address(rarg2, rax, 
                          Address::times_8,
                          arrayOopDesc::base_offset_in_bytes(T_DOUBLE)));
}

void TemplateTable::aaload()
{
  transition(itos, atos);
  __ pop_ptr(rarg2);
  // eax: index
  // rarg2: array
  index_check(rarg2, rax); // kills rbx
  __ movq(rax, Address(rarg2, rax, 
                       Address::times_8,
                       arrayOopDesc::base_offset_in_bytes(T_OBJECT)));
}

void TemplateTable::baload() 
{
  transition(itos, itos);
  __ pop_ptr(rarg2);
  // eax: index
  // rarg2: array
  index_check(rarg2, rax); // kills rbx
  __ load_signed_byte(rax, 
                      Address(rarg2, rax, 
                              Address::times_1, 
                              arrayOopDesc::base_offset_in_bytes(T_BYTE)));
}

void TemplateTable::caload()
{
  transition(itos, itos);
  __ pop_ptr(rarg2);
  // eax: index
  // rarg2: array
  index_check(rarg2, rax); // kills rbx
  __ load_unsigned_word(rax, 
                        Address(rarg2, rax, 
                                Address::times_2,
                                arrayOopDesc::base_offset_in_bytes(T_CHAR)));
}

// iload followed by caload frequent pair
void TemplateTable::fast_icaload()
{
  transition(vtos, itos);
  // load index out of locals
  locals_index(rbx);
  __ movl(rax, iaddress(rbx));

  // eax: index
  // rarg2: array
  __ pop_ptr(rarg2);
  index_check(rarg2, rax); // kills rbx
  __ load_unsigned_word(rax, 
                        Address(rarg2, rax, 
                                Address::times_2,
                                arrayOopDesc::base_offset_in_bytes(T_CHAR)));
}

void TemplateTable::saload()
{
  transition(itos, itos);
  __ pop_ptr(rarg2);
  // eax: index
  // rarg2: array
  index_check(rarg2, rax); // kills rbx
  __ load_signed_word(rax,
                      Address(rarg2, rax, 
                              Address::times_2,
                              arrayOopDesc::base_offset_in_bytes(T_SHORT)));
}

void TemplateTable::iload(int n)
{
  transition(vtos, itos);
  __ movl(rax, iaddress(n));
}

void TemplateTable::lload(int n) 
{
  transition(vtos, ltos);
  __ movq(rax, laddress(n));
}

void TemplateTable::fload(int n) 
{
  transition(vtos, ftos);
  __ movss(xmm0, faddress(n));
}

void TemplateTable::dload(int n)
{
  transition(vtos, dtos);
  __ movlpd(xmm0, daddress(n));
}

void TemplateTable::aload(int n)
{
  transition(vtos, atos);
  __ movq(rax, aaddress(n));
}

void TemplateTable::aload_0() 
{
  transition(vtos, atos);
  // According to bytecode histograms, the pairs:
  //
  // _aload_0, _fast_igetfield
  // _aload_0, _fast_agetfield
  // _aload_0, _fast_fgetfield
  //
  // occur frequently. If RewriteFrequentPairs is set, the (slow)
  // _aload_0 bytecode checks if the next bytecode is either
  // _fast_igetfield, _fast_agetfield or _fast_fgetfield and then
  // rewrites the current bytecode into a pair bytecode; otherwise it
  // rewrites the current bytecode into _fast_aload_0 that doesn't do
  // the pair check anymore.
  //
  // Note: If the next bytecode is _getfield, the rewrite must be
  //       delayed, otherwise we may miss an opportunity for a pair.
  //
  // Also rewrite frequent pairs
  //   aload_0, aload_1
  //   aload_0, iload_1
  // These bytecodes with a small amount of code are most profitable
  // to rewrite
  if (RewriteFrequentPairs) {
    Label rewrite, done;
    // get next byte
    __ load_unsigned_byte(rbx, 
                          at_bcp(Bytecodes::length_for(Bytecodes::_aload_0)));

    // do actual aload_0
    aload(0);

    // if _getfield then wait with rewrite
    __ cmpl(rbx, Bytecodes::_getfield);
    __ jcc(Assembler::equal, done);

    // if _igetfield then reqrite to _fast_iaccess_0
    assert(Bytecodes::java_code(Bytecodes::_fast_iaccess_0) == 
           Bytecodes::_aload_0, 
           "fix bytecode definition");
    __ cmpl(rbx, Bytecodes::_fast_igetfield);
    __ movl(rarg3, Bytecodes::_fast_iaccess_0);
    __ jcc(Assembler::equal, rewrite);

    // if _agetfield then reqrite to _fast_aaccess_0
    assert(Bytecodes::java_code(Bytecodes::_fast_aaccess_0) == 
           Bytecodes::_aload_0, 
           "fix bytecode definition");
    __ cmpl(rbx, Bytecodes::_fast_agetfield);
    __ movl(rarg3, Bytecodes::_fast_aaccess_0);
    __ jcc(Assembler::equal, rewrite);

    // if _fgetfield then reqrite to _fast_faccess_0
    assert(Bytecodes::java_code(Bytecodes::_fast_faccess_0) == 
           Bytecodes::_aload_0,
           "fix bytecode definition");
    __ cmpl(rbx, Bytecodes::_fast_fgetfield);
    __ movl(rarg3, Bytecodes::_fast_faccess_0);
    __ jcc(Assembler::equal, rewrite);

    // else rewrite to _fast_aload0
    assert(Bytecodes::java_code(Bytecodes::_fast_aload_0) == 
           Bytecodes::_aload_0,
           "fix bytecode definition");
    __ movl(rarg3, Bytecodes::_fast_aload_0);

    // rewrite
    // ecx: fast bytecode
    __ bind(rewrite);
    patch_bytecode(Bytecodes::_aload_0, rarg3, rbx, false);

    __ bind(done);
  } else {
    aload(0);
  }
}

void TemplateTable::istore() 
{
  transition(itos, vtos);
  locals_index(rbx);
  __ movl(iaddress(rbx), rax);
}

void TemplateTable::lstore() 
{
  transition(ltos, vtos);
  locals_index(rbx);
  __ movq(laddress(rbx), rax);
}

void TemplateTable::fstore() 
{
  transition(ftos, vtos);
  locals_index(rbx);
  __ movss(faddress(rbx), xmm0);
}

void TemplateTable::dstore() 
{
  transition(dtos, vtos);
  locals_index(rbx);
  __ movsd(daddress(rbx), xmm0);
}

void TemplateTable::astore()
{
  transition(vtos, vtos);
  __ pop_ptr();
  locals_index(rbx);
  __ movq(aaddress(rbx), rax);
}

void TemplateTable::wide_istore() 
{
  transition(vtos, vtos);
  __ pop_i();
  locals_index_wide(rbx);
  __ movl(iaddress(rbx), rax);
}

void TemplateTable::wide_lstore() 
{
  transition(vtos, vtos);
  __ pop_l();
  locals_index_wide(rbx);
  __ movq(laddress(rbx), rax);
}

void TemplateTable::wide_fstore() 
{
  transition(vtos, vtos);
  __ pop_f();
  locals_index_wide(rbx);
  __ movss(faddress(rbx), xmm0);
}

void TemplateTable::wide_dstore() 
{
  transition(vtos, vtos);
  __ pop_d();
  locals_index_wide(rbx);
  __ movsd(daddress(rbx), xmm0);
}

void TemplateTable::wide_astore() 
{
  transition(vtos, vtos);
  __ pop_ptr();
  locals_index_wide(rbx);
  __ movq(aaddress(rbx), rax);
}

void TemplateTable::iastore() 
{
  transition(itos, vtos);
  __ pop_i(rbx);
  __ pop_ptr(rarg2);
  // eax: value
  // ebx: index
  // rarg2: array
  index_check(rarg2, rbx); // prefer index in ebx
  __ movl(Address(rarg2, rbx, 
                  Address::times_4,
                  arrayOopDesc::base_offset_in_bytes(T_INT)),
          rax);
}

void TemplateTable::lastore()
{
  transition(ltos, vtos);
  __ pop_i(rbx);
  __ pop_ptr(rarg2);
  // rax: value
  // ebx: index
  // rarg2: array
  index_check(rarg2, rbx); // prefer index in ebx
  __ movq(Address(rarg2, rbx, 
                  Address::times_8, 
                  arrayOopDesc::base_offset_in_bytes(T_LONG)),
          rax);
}

void TemplateTable::fastore()
{
  transition(ftos, vtos);
  __ pop_i(rbx);
  __ pop_ptr(rarg2);
  // xmm0: value
  // ebx:  index
  // rarg2:  array
  index_check(rarg2, rbx); // prefer index in ebx
  __ movss(Address(rarg2, rbx, 
                   Address::times_4, 
                   arrayOopDesc::base_offset_in_bytes(T_FLOAT)),
           xmm0);
}

void TemplateTable::dastore() 
{
  transition(dtos, vtos);
  __ pop_i(rbx);
  __ pop_ptr(rarg2);
  // xmm0: value
  // ebx:  index
  // rarg2:  array
  index_check(rarg2, rbx); // prefer index in ebx
  __ movsd(Address(rarg2, rbx, 
                   Address::times_8, 
                   arrayOopDesc::base_offset_in_bytes(T_DOUBLE)),
           xmm0);
}

void TemplateTable::aastore() 
{
  Label is_null, ok_is_subtype, done;
  transition(vtos, vtos);
  // stack: ..., array, index, value
  __ movq(rax, at_rsp());    // value
  __ movl(rarg3, at_rsp_p1()); // index
  __ movq(rarg2, at_rsp_p2()); // array
  index_check(rarg2, rarg3);     // kills rbx
  // do array store check - check for NULL value first
  __ testq(rax, rax);
  __ jcc(Assembler::zero, is_null);

  __ profile_checkcast(false, rbx); // Blows rbx

  // Move subklass into rbx
  __ movq(rbx, Address(rax, oopDesc::klass_offset_in_bytes()));
  // Move superklass into rax
  __ movq(rax, Address(rarg2, oopDesc::klass_offset_in_bytes()));
  __ movq(rax, Address(rax, 
                       sizeof(oopDesc) + 
                       objArrayKlass::element_klass_offset_in_bytes()));
  // Compress array + index*8 + 12 into a single register.  Frees rarg3.
  __ leaq(rarg2, Address(rarg2, rarg3, 
                       Address::times_8, 
                       arrayOopDesc::base_offset_in_bytes(T_OBJECT)));

  // Generate subtype check.  Blows rarg3.  Resets rarg0 to locals.
  // Superklass in rax.  Subklass in rbx.
  __ gen_subtype_check(rbx, ok_is_subtype);

  // Come here on failure
  // object is at TOS
  __ jmp(Interpreter::_throw_ArrayStoreException_entry, relocInfo::none);

  // Come here on success
  __ bind(ok_is_subtype);
  __ movq(rax, at_rsp()); // Value
  __ movq(Address(rarg2), rax);
  __ store_check(rarg2);
  __ jmp(done);

  // Have a NULL in rax, rarg2=array, ecx=index.  Store NULL at ary[idx]
  __ bind(is_null);
  __ profile_checkcast(true, rbx);
  __ movq(Address(rarg2, rarg3, 
                  Address::times_8, 
                  arrayOopDesc::base_offset_in_bytes(T_OBJECT)), 
          rax);

  // Pop stack arguments
  __ bind(done);
  __ addq(rsp, 3 * wordSize);
}

void TemplateTable::bastore() 
{
  transition(itos, vtos);
  __ pop_i(rbx);
  __ pop_ptr(rarg2);
  // eax: value
  // ebx: index
  // rarg2: array
  index_check(rarg2, rbx); // prefer index in ebx
  __ movb(Address(rarg2, rbx, 
                  Address::times_1, 
                  arrayOopDesc::base_offset_in_bytes(T_BYTE)), 
          rax);
}

void TemplateTable::castore() {
  transition(itos, vtos);
  __ pop_i(rbx);
  __ pop_ptr(rarg2);
  // eax: value
  // ebx: index
  // rarg2: array
  index_check(rarg2, rbx);  // prefer index in ebx
  __ movw(Address(rarg2, rbx, 
                  Address::times_2,
                  arrayOopDesc::base_offset_in_bytes(T_CHAR)),
          rax);
}

void TemplateTable::sastore() 
{
  castore();
}

void TemplateTable::istore(int n) 
{
  transition(itos, vtos);
  __ movl(iaddress(n), rax);
}

void TemplateTable::lstore(int n) 
{
  transition(ltos, vtos);
  __ movq(laddress(n), rax);
}

void TemplateTable::fstore(int n) 
{
  transition(ftos, vtos);
  __ movss(faddress(n), xmm0);
}

void TemplateTable::dstore(int n) 
{
  transition(dtos, vtos);
  __ movsd(daddress(n), xmm0);
}

void TemplateTable::astore(int n) 
{
  transition(vtos, vtos);
  __ pop_ptr();
  __ movq(aaddress(n), rax);
}

void TemplateTable::pop() 
{
  transition(vtos, vtos);
  __ addq(rsp, wordSize);
}

void TemplateTable::pop2()
{
  transition(vtos, vtos);
  __ addq(rsp, 2 * wordSize);
}

void TemplateTable::dup() 
{
  transition(vtos, vtos);
  // stack: ..., a
  __ pushq(at_rsp());
  // stack: ..., a, a
}

void TemplateTable::dup_x1() 
{
  transition(vtos, vtos);
  // stack: ..., a, b
  __ movq(rax, at_rsp_p1()); // get a
  __ movq(rbx, at_rsp());    // get b
  __ movq(at_rsp_p1(), rbx); // put b
  __ movq(at_rsp(), rax);    // put a
  __ pushq(rbx);             // push b
  // stack: ..., b, a, b
}

void TemplateTable::dup_x2() 
{
  transition(vtos, vtos);
  // stack: ..., a, b, c
  __ movq(rax, at_rsp_p2()); // get a
  __ movq(rbx, at_rsp_p1()); // get b
  __ movq(rarg3, at_rsp());    // get c
  __ movq(at_rsp_p2(), rarg3); // put c
  __ movq(at_rsp_p1(), rax); // put a
  __ movq(at_rsp(), rbx);    // put b
  __ pushq(rarg3);             // push c
  // stack: ..., c, a, b, c
}

void TemplateTable::dup2() 
{
  transition(vtos, vtos);
  // stack: ..., a, b
  __ pushq(at_rsp_p1());
  __ pushq(at_rsp_p1());
  // stack: ..., a, b, a, b
}

void TemplateTable::dup2_x1() 
{
  transition(vtos, vtos);
  // stack: ..., a, b, c
  __ movq(rax, at_rsp_p2()); // get a
  __ movq(rbx, at_rsp_p1()); // get b
  __ movq(rarg3, at_rsp());    // get c
  __ movq(at_rsp_p2(), rbx); // put b
  __ movq(at_rsp_p1(), rarg3); // put c
  __ movq(at_rsp(), rax);    // put a
  __ pushq(rbx);             // push b
  __ pushq(rarg3);             // push c
  // stack: ..., b, c, a, b, c
}

void TemplateTable::dup2_x2() 
{
  transition(vtos, vtos);
  // stack: ..., a, b, c, d
  __ movq(rax, at_rsp_p3()); // get a
  __ movq(rbx, at_rsp_p2()); // get b
  __ movq(rarg3, at_rsp_p1()); // get c
  __ movq(rarg2, at_rsp());    // get d
  __ movq(at_rsp_p3(), rarg3); // put c
  __ movq(at_rsp_p2(), rarg2); // put d
  __ movq(at_rsp_p1(), rax); // put a
  __ movq(at_rsp(), rbx);    // put b
  __ pushq(rarg3);             // push c
  __ pushq(rarg2);             // push d
  // stack: ..., c, d, a, b, c, d
}

void TemplateTable::swap() 
{
  transition(vtos, vtos);
  // stack: ..., a, b
  __ movq(rax, at_rsp_p1()); // get a
  __ movq(rbx, at_rsp());    // get b
  __ movq(at_rsp_p1(), rbx); // put b
  __ movq(at_rsp(), rax);    // put a
  // stack: ..., b, a
}

void TemplateTable::iop2(Operation op)
{
  transition(itos, itos);
  switch (op) {
  case add  :                    __ pop_i(rdx); __ addl (rax, rdx); break;
  case sub  : __ movl(rdx, rax); __ pop_i(rax); __ subl (rax, rdx); break;
  case mul  :                    __ pop_i(rdx); __ imull(rax, rdx); break;
  case _and :                    __ pop_i(rdx); __ andl (rax, rdx); break;
  case _or  :                    __ pop_i(rdx); __ orl  (rax, rdx); break;
  case _xor :                    __ pop_i(rdx); __ xorl (rax, rdx); break;
  case shl  : __ movl(rcx, rax); __ pop_i(rax); __ shll (rax);      break;
  case shr  : __ movl(rcx, rax); __ pop_i(rax); __ sarl (rax);      break;
  case ushr : __ movl(rcx, rax); __ pop_i(rax); __ shrl (rax);      break;
  default   : ShouldNotReachHere();
  }
}

void TemplateTable::lop2(Operation op)
{
  transition(ltos, ltos);
  switch (op) {
  case add  :                    __ pop_l(rdx); __ addq (rax, rdx); break;
  case sub  : __ movq(rdx, rax); __ pop_l(rax); __ subq (rax, rdx); break;
  case _and :                    __ pop_l(rdx); __ andq (rax, rdx); break;
  case _or  :                    __ pop_l(rdx); __ orq  (rax, rdx); break;
  case _xor :                    __ pop_l(rdx); __ xorq (rax, rdx); break;
  default : ShouldNotReachHere();
  }
}

void TemplateTable::idiv() 
{
  transition(itos, itos);
  __ movl(rcx, rax);
  __ pop_i(rax);
  // Note: could xor eax and ecx and compare with (-1 ^ min_int). If
  //       they are not equal, one could do a normal division (no correction
  //       needed), which may speed up this implementation for the common case.
  //       (see also JVM spec., p.243 & p.271)
  __ corrected_idivl(rcx);
}

void TemplateTable::irem()
{
  transition(itos, itos);
  __ movl(rcx, rax);
  __ pop_i(rax);
  // Note: could xor eax and ecx and compare with (-1 ^ min_int). If
  //       they are not equal, one could do a normal division (no correction
  //       needed), which may speed up this implementation for the common case.
  //       (see also JVM spec., p.243 & p.271)
  __ corrected_idivl(rcx);
  __ movl(rax, rdx);
}

void TemplateTable::lmul()
{
  transition(ltos, ltos);
  __ pop_l(rdx);
  __ imulq(rax, rdx);
}

void TemplateTable::ldiv() 
{
  transition(ltos, ltos);
  __ movq(rcx, rax);
  __ pop_l(rax);
  // generate explicit div0 check
  __ testq(rcx, rcx);
  __ jcc(Assembler::zero, 
         Interpreter::_throw_ArithmeticException_entry, relocInfo::none);
  // Note: could xor rax and rcx and compare with (-1 ^ min_int). If
  //       they are not equal, one could do a normal division (no correction
  //       needed), which may speed up this implementation for the common case.
  //       (see also JVM spec., p.243 & p.271)
  __ corrected_idivq(rcx); // kills rbx
}

void TemplateTable::lrem() 
{
  transition(ltos, ltos);
  __ movq(rcx, rax);
  __ pop_l(rax);
  __ testq(rcx, rcx);
  __ jcc(Assembler::zero, 
         Interpreter::_throw_ArithmeticException_entry, relocInfo::none);
  // Note: could xor rax and rcx and compare with (-1 ^ min_int). If
  //       they are not equal, one could do a normal division (no correction
  //       needed), which may speed up this implementation for the common case.
  //       (see also JVM spec., p.243 & p.271)
  __ corrected_idivq(rcx); // kills rbx
  __ movq(rax, rdx);
}

void TemplateTable::lshl() 
{
  transition(itos, ltos);
  __ movl(rcx, rax);                             // get shift count
  __ pop_l(rax);                                 // get shift value
  __ shlq(rax);
}

void TemplateTable::lshr() 
{
  transition(itos, ltos);
  __ movl(rcx, rax);                             // get shift count
  __ pop_l(rax);                                 // get shift value
  __ sarq(rax);
}

void TemplateTable::lushr() 
{
  transition(itos, ltos);
  __ movl(rcx, rax);                             // get shift count
  __ pop_l(rax);                                 // get shift value
  __ shrq(rax);
}

void TemplateTable::fop2(Operation op) 
{
  transition(ftos, ftos);
  switch (op) {
  case add:
    __ addss(xmm0, at_rsp());
    __ addq(rsp, wordSize);
    break;
  case sub:
    __ movss(xmm1, xmm0);
    __ pop_f(xmm0);
    __ subss(xmm0, xmm1);
    break;
  case mul:
    __ mulss(xmm0, at_rsp());
    __ addq(rsp, wordSize);
    break;
  case div:
    __ movss(xmm1, xmm0);
    __ pop_f(xmm0);
    __ divss(xmm0, xmm1);
    break;
  case rem:
    __ movss(xmm1, xmm0);
    __ pop_f(xmm0);
    __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::frem), 1);
    break;
  default:
    ShouldNotReachHere();
    break;
  }
}

void TemplateTable::dop2(Operation op) 
{
  transition(dtos, dtos);
  switch (op) {
  case add:
    __ addsd(xmm0, at_rsp());
    __ addq(rsp, 2 * wordSize);
    break;
  case sub:
    __ movsd(xmm1, xmm0);
    __ pop_d(xmm0); 
    __ subsd(xmm0, xmm1);
    break;
  case mul:
    __ mulsd(xmm0, at_rsp());
    __ addq(rsp, 2 * wordSize);
    break;
  case div:
    __ movsd(xmm1, xmm0);
    __ pop_d(xmm0);
    __ divsd(xmm0, xmm1);
    break;
  case rem:
    __ movsd(xmm1, xmm0);
    __ pop_d(xmm0);
    __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::drem), 1);
    break;
  default:
    ShouldNotReachHere();
    break;
  }
}

void TemplateTable::ineg() 
{
  transition(itos, itos);
  __ negl(rax);
}

void TemplateTable::lneg() 
{
  transition(ltos, ltos);
  __ negq(rax);
}

// Note: 'double' and 'long long' have 32-bits alignment on x86.
static jlong* double_quadword(jlong *adr, jlong lo, jlong hi) {
  // Use the expression (adr)&(~0xF) to provide 128-bits aligned address
  // of 128-bits operands for SSE instructions.
  jlong *operand = (jlong*)(((intptr_t)adr)&((intptr_t)(~0xF)));
  // Store the value to a 128-bits operand.
  operand[0] = lo;
  operand[1] = hi;
  return operand;
}

// Buffer for 128-bits masks used by SSE instructions.
static jlong float_signflip_pool[2*2];
static jlong double_signflip_pool[2*2];

void TemplateTable::fneg()
{
  transition(ftos, ftos);
  static jlong *float_signflip  = double_quadword(&float_signflip_pool[1], 0x8000000080000000, 0x8000000080000000); 
  __ xorps(xmm0, Address((address) float_signflip, relocInfo::none));
}

void TemplateTable::dneg()
{
  transition(dtos, dtos);
  static jlong *double_signflip  = double_quadword(&double_signflip_pool[1], 0x8000000000000000, 0x8000000000000000); 
  __ xorpd(xmm0, Address((address) double_signflip, relocInfo::none));
}

void TemplateTable::iinc() 
{
  transition(vtos, vtos);
  __ load_signed_byte(rarg2, at_bcp(2)); // get constant
  locals_index(rbx);
  __ addl(iaddress(rbx), rarg2);
}

void TemplateTable::wide_iinc() 
{
  transition(vtos, vtos);
  __ movl(rarg2, at_bcp(4)); // get constant
  locals_index_wide(rbx);
  __ bswapl(rarg2); // swap bytes & sign-extend constant
  __ sarl(rarg2, 16);
  __ addl(iaddress(rbx), rarg2);
  // Note: should probably use only one movl to get both
  //       the index and the constant -> fix this
}

void TemplateTable::convert() 
{
  // Checking
#ifdef ASSERT
  {
    TosState tos_in  = ilgl;
    TosState tos_out = ilgl;
    switch (bytecode()) {
    case Bytecodes::_i2l: // fall through
    case Bytecodes::_i2f: // fall through
    case Bytecodes::_i2d: // fall through
    case Bytecodes::_i2b: // fall through
    case Bytecodes::_i2c: // fall through
    case Bytecodes::_i2s: tos_in = itos; break;
    case Bytecodes::_l2i: // fall through
    case Bytecodes::_l2f: // fall through
    case Bytecodes::_l2d: tos_in = ltos; break;
    case Bytecodes::_f2i: // fall through
    case Bytecodes::_f2l: // fall through
    case Bytecodes::_f2d: tos_in = ftos; break;
    case Bytecodes::_d2i: // fall through
    case Bytecodes::_d2l: // fall through
    case Bytecodes::_d2f: tos_in = dtos; break;
    default             : ShouldNotReachHere();
    }
    switch (bytecode()) {
    case Bytecodes::_l2i: // fall through
    case Bytecodes::_f2i: // fall through
    case Bytecodes::_d2i: // fall through
    case Bytecodes::_i2b: // fall through
    case Bytecodes::_i2c: // fall through
    case Bytecodes::_i2s: tos_out = itos; break;
    case Bytecodes::_i2l: // fall through
    case Bytecodes::_f2l: // fall through
    case Bytecodes::_d2l: tos_out = ltos; break;
    case Bytecodes::_i2f: // fall through
    case Bytecodes::_l2f: // fall through
    case Bytecodes::_d2f: tos_out = ftos; break;
    case Bytecodes::_i2d: // fall through
    case Bytecodes::_l2d: // fall through
    case Bytecodes::_f2d: tos_out = dtos; break;
    default             : ShouldNotReachHere();
    }
    transition(tos_in, tos_out);
  }
#endif // ASSERT

  static const int64_t is_nan = 0x8000000000000000L;

  // Conversion
  switch (bytecode()) {
  case Bytecodes::_i2l:
    __ movslq(rax, rax);
    break;
  case Bytecodes::_i2f:
    __ cvtsi2ssl(xmm0, rax);
    break;
  case Bytecodes::_i2d:
    __ cvtsi2sdl(xmm0, rax);
    break;
  case Bytecodes::_i2b:
    __ movsbl(rax, rax);
    break;
  case Bytecodes::_i2c:
    __ movzwl(rax, rax);
    break;
  case Bytecodes::_i2s:
    __ movswl(rax, rax);
    break;
  case Bytecodes::_l2i:
    __ movl(rax, rax);
    break;
  case Bytecodes::_l2f:
    __ cvtsi2ssq(xmm0, rax);
    break;
  case Bytecodes::_l2d:
    __ cvtsi2sdq(xmm0, rax);
    break;
  case Bytecodes::_f2i:
  {
    Label L;
    __ cvttss2sil(rax, xmm0);
    __ cmpl(rax, 0x80000000); // NaN or overflow/underflow?
    __ jcc(Assembler::notEqual, L);
    __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::f2i), 1);
    __ bind(L);
  }
    break;
  case Bytecodes::_f2l:
  {
    Label L;
    __ cvttss2siq(rax, xmm0);
    // NaN or overflow/underflow?
    __ cmpq(rax, Address((address) &is_nan, relocInfo::none));
    __ jcc(Assembler::notEqual, L);
    __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::f2l), 1);
    __ bind(L);
  }
    break;
  case Bytecodes::_f2d:
    __ cvtss2sd(xmm0, xmm0);
    break;
  case Bytecodes::_d2i:
  {
    Label L;
    __ cvttsd2sil(rax, xmm0);
    __ cmpl(rax, 0x80000000); // NaN or overflow/underflow?
    __ jcc(Assembler::notEqual, L);
    __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::d2i), 1);
    __ bind(L);
  }
    break;
  case Bytecodes::_d2l:
  {
    Label L;
    __ cvttsd2siq(rax, xmm0);
    // NaN or overflow/underflow?
    __ cmpq(rax, Address((address) &is_nan, relocInfo::none));
    __ jcc(Assembler::notEqual, L);
    __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::d2l), 1);
    __ bind(L);
  }
    break;
  case Bytecodes::_d2f:
    __ cvtsd2ss(xmm0, xmm0);
    break;
  default:
    ShouldNotReachHere();
  }
}

void TemplateTable::lcmp() 
{
  transition(ltos, itos);
  Label less, done;
  __ pop_l(rarg2);
  __ cmpq(rarg2, rax);
  __ movl(rax, -1);
  __ jcc(Assembler::less, done);
  __ setb(Assembler::notEqual, rax);
  __ movzbl(rax, rax);
  __ bind(done);
}

void TemplateTable::float_cmp(bool is_float, int unordered_result) 
{
  Label done;
  if (is_float) {
    // XXX get rid of pop here, use ... reg, mem32
    __ pop_f(xmm1);
    __ ucomiss(xmm1, xmm0);
  } else {
    // XXX get rid of pop here, use ... reg, mem64
    __ pop_d(xmm1);
    __ ucomisd(xmm1, xmm0);
  }
  if (unordered_result < 0) {
    __ movl(rax, -1);
    __ jcc(Assembler::parity, done);
    __ jcc(Assembler::below, done);
    __ setb(Assembler::notEqual, rarg2);
    __ movzbl(rax, rarg2);
  } else {
    __ movl(rax, 1);
    __ jcc(Assembler::parity, done);
    __ jcc(Assembler::above, done);
    __ movl(rax, 0);
    __ jcc(Assembler::equal, done);
    __ decl(rax);
  }
  __ bind(done);
}

void TemplateTable::branch(bool is_jsr, bool is_wide)
{
  __ get_method(rarg3); // rarg3 holds method
  __ profile_taken_branch(rax, rbx); // rax holds updated MDP, rbx
                                     // holds bumped taken count

#ifndef CORE
  const ByteSize be_offset = methodOopDesc::backedge_counter_offset() + 
                             InvocationCounter::counter_offset();
  const ByteSize inv_offset = methodOopDesc::invocation_counter_offset() +
                              InvocationCounter::counter_offset();
  const int method_offset = frame::interpreter_frame_method_offset * wordSize;
#endif // CORE

  // Load up edx with the branch displacement
  __ movl(rdx, at_bcp(1));
  __ bswapl(rdx);

  if (!is_wide) {
    __ sarl(rdx, 16);
  }
  __ movslq(rdx, rdx);

  // Handle all the JSR stuff here, then exit.
  // It's much shorter and cleaner than intermingling with the non-JSR
  // normal-branch stuff occuring below.
  if (is_jsr) {
    // Pre-load the next target bytecode into rbx
    __ load_unsigned_byte(rbx, Address(r13, rdx, Address::times_1, 0));

    // compute return address as bci in rax
    __ leaq(rax, at_bcp((is_wide ? 5 : 3) -
                        in_bytes(constMethodOopDesc::codes_offset())));
    __ subq(rax, Address(rarg3, methodOopDesc::const_offset()));
    // Adjust the bcp in r13 by the displacement in rdx
    __ addq(r13, rdx);
    // jsr returns atos that is not an oop
    __ dispatch_only_noverify(atos);
    return;
  }

  // Normal (non-jsr) branch handling

  // Adjust the bcp in r13 by the displacement in rdx  
  __ addq(r13, rdx);

#ifdef CORE
  // Pre-load the next target bytecode into ebx
  __ load_unsigned_byte(rbx, Address(r13));

  // continue with the bytecode @ target
  // rax: return bci for jsr's, unused otherwise
  // rbx: target bytecode
  // r13: target bcp
  __ dispatch_only(vtos);

#else

  assert(UseLoopCounter || !UseOnStackReplacement, 
         "on-stack-replacement requires loop counters");
  Label backedge_counter_overflow;
  Label profile_method;
  Label dispatch;
  if (UseLoopCounter) {
    // increment backedge counter for backward branches
    // rax: MDO
    // ebx: MDO bumped taken-count
    // rcx: method
    // rdx: target offset
    // r13: target bcp
    // r14: locals pointer
    __ testl(rdx, rdx);             // check if forward or backward branch
    __ jcc(Assembler::positive, dispatch); // count only if backward branch

    // increment counter 
    __ movl(rax, Address(rarg3, be_offset));        // load backedge counter
    __ incrementl(rax, InvocationCounter::count_increment); // increment
                                                            // counter
    __ movl(Address(rarg3, be_offset), rax);        // store counter

    __ movl(rax, Address(rarg3, inv_offset));    // load invocation counter
    __ andl(rax, InvocationCounter::count_mask_value); // and the status bits
    __ addl(rax, Address(rarg3, be_offset));        // add both counters

    if (ProfileInterpreter) {
      // Test to see if we should create a method data oop
      __ cmpl(rax, Address((address) 
                           &InvocationCounter::InterpreterProfileLimit,
                           relocInfo::none));
      __ jcc(Assembler::less, dispatch);

      // if no method data exists, go to profile method
      __ test_method_data_pointer(rax, profile_method);

      if (UseOnStackReplacement) {
        // check for overflow against ebx which is the MDO taken count
        __ cmpl(rbx, Address((address) &InvocationCounter::
                             InterpreterBackwardBranchLimit,
                             relocInfo::none));
        __ jcc(Assembler::below, dispatch);

        // When ProfileInterpreter is on, the backedge_count comes
        // from the methodDataOop, which value does not get reset on
        // the call to frequency_counter_overflow().  To avoid
        // excessive calls to the overflow routine while the method is
        // being compiled, add a second test to make sure the overflow
        // function is called only once every overflow_frequency.
        const int overflow_frequency = 1024;
	__ andl(rbx, overflow_frequency - 1);
        __ jcc(Assembler::zero, backedge_counter_overflow);

      }
    } else {
      if (UseOnStackReplacement) {
        // check for overflow against eax, which is the sum of the
        // counters
        __ cmpl(rax, Address((address) &InvocationCounter::
                             InterpreterBackwardBranchLimit,
                             relocInfo::none));
        __ jcc(Assembler::aboveEqual, backedge_counter_overflow);

      }
    }
    __ bind(dispatch);
  }

  // Pre-load the next target bytecode into rbx
  __ load_unsigned_byte(rbx, Address(r13));

  // continue with the bytecode @ target
  // eax: return bci for jsr's, unused otherwise
  // ebx: target bytecode
  // r13: target bcp
  __ dispatch_only(vtos);

  if (UseLoopCounter) {
    if (ProfileInterpreter) {
      // Out-of-line code to allocate method data oop.
      __ bind(profile_method);
      __ call_VM(noreg, 
                 CAST_FROM_FN_PTR(address, 
                                  InterpreterRuntime::profile_method), r13);
      __ load_unsigned_byte(rbx, Address(r13));  // restore target bytecode
      __ movq(rarg3, Address(rbp, method_offset));
      __ movq(rarg3, Address(rarg3, 
                           in_bytes(methodOopDesc::method_data_offset())));
      __ movq(Address(rbp, frame::interpreter_frame_mdx_offset * wordSize),
              rarg3);
      __ test_method_data_pointer(rarg3, dispatch);
      // offset non-null mdp by MDO::data_offset() + IR::profile_method()
      __ addq(rarg3, in_bytes(methodDataOopDesc::data_offset()));
      __ addq(rarg3, rax);
      __ movq(Address(rbp, frame::interpreter_frame_mdx_offset * wordSize),
              rarg3);
      __ jmp(dispatch);
    }

    if (UseOnStackReplacement) {
      // invocation counter overflow
      __ bind(backedge_counter_overflow);
      __ negq(rdx);
      __ addq(rdx, r13); // branch bcp
      assert(!Bytecodes::safepoint_safe(bytecode()),
             "can call OSR method which may not stop");
      // IcoResult frequency_counter_overflow([JavaThread*], address branch_bcp)
      __ call_VM_Ico(noreg,
                     CAST_FROM_FN_PTR(address, 
                                      InterpreterRuntime::frequency_counter_overflow),
              rdx);
      __ load_unsigned_byte(rbx, Address(r13));  // restore target bytecode

      // rax: osr nmethod (osr ok) or NULL (osr not possible)
      // ebx: target bytecode
      // rdx: osr adapter frame return address
      // r14: locals pointer
      // r13: bcp
      __ testq(rax, rax);                        // test result
      __ jcc(Assembler::zero, dispatch);         // no osr if null
      // nmethod may have been invalidated (VM may block upon call_VM return)
      __ movl(rarg3, Address(rax, nmethod::entry_bci_offset()));
      __ cmpl(rarg3, InvalidOSREntryBci);
      __ jcc(Assembler::equal, dispatch);
      
      // We have the address of an on stack replacement routine in rax
      // and the return address in rdx (the osr-adapter).  We need to
      // prepare for the call.

      // Reserve space for incoming arguments.  This is needed since
      // an I2C adapter could be introduced as part of deoptimization.
      // The I2C adapter will think that it owns the incoming
      // arguments area.  If we do not reserve space for this
      // explicitly, then the I2C adapter might override the
      // old-rbp/old-rsp offsets in the OSR adapter frame, and frame
      // traversal will fail.  Figure out size of outgoing arguments.
      __ movq(rbx, Address(rax, nmethod::method_offset_in_bytes()));
      // Make sure not to use rarg3 in the src part.  Otherwise, wrong
      // assembly will be generated.
      __ load_unsigned_word(rarg3, 
                            Address(rbx, 
                                    methodOopDesc::
                                    size_of_parameters_offset()));      

      __ shll(rarg3, LogBytesPerWord);
      __ subq(rsp, rarg3);

      // align the stack.  The osr stub generation code should have
      // selected a blob with the right frame size for the frame after
      // alignment.
      __ andq(rsp, -16); 

      // osr adapter frame return address
      __ pushq(rdx);
      // nmethod to rdx to free up rax
      __ movq(rdx, rax);
      // Load monitor address into inline-cache register
      const int entry_size = 
        frame::interpreter_frame_monitor_size() * wordSize;
      const Address monitor_block_bot(rbp,
                                 frame::interpreter_frame_initial_sp_offset *
                                 wordSize - entry_size);
      __ leaq(rax, monitor_block_bot); // points to bottom entry of
                                       // monitor block
      // continue at osr code
      // rax: locks pointer
      // ebx: dead
      // rdx: osr nmethod
      // r14: locals pointer
      // r13: bcp
      __ jmp(Address(rdx, nmethod::osr_entry_point_offset()));
    }
  }
#endif // not CORE
}


void TemplateTable::if_0cmp(Condition cc)
{
  transition(itos, vtos);
  // assume branch is more often taken than not (loops use backward branches)
  Label not_taken;
  __ testl(rax, rax);
  __ jcc(j_not(cc), not_taken);
  branch(false, false);
  __ bind(not_taken);
  __ profile_not_taken_branch(rax);
}

void TemplateTable::if_icmp(Condition cc)
{
  transition(itos, vtos);
  // assume branch is more often taken than not (loops use backward branches)
  Label not_taken;
  __ pop_i(rarg2);
  __ cmpl(rarg2, rax);
  __ jcc(j_not(cc), not_taken);
  branch(false, false);
  __ bind(not_taken);
  __ profile_not_taken_branch(rax);
}

void TemplateTable::if_nullcmp(Condition cc)
{
  transition(atos, vtos);
  // assume branch is more often taken than not (loops use backward branches)
  Label not_taken;
  __ testq(rax, rax);
  __ jcc(j_not(cc), not_taken);
  branch(false, false);
  __ bind(not_taken);
  __ profile_not_taken_branch(rax);
}

void TemplateTable::if_acmp(Condition cc) 
{
  transition(atos, vtos);
  // assume branch is more often taken than not (loops use backward branches)
  Label not_taken;
  __ pop_ptr(rarg2);
  __ cmpq(rarg2, rax);
  __ jcc(j_not(cc), not_taken);
  branch(false, false);
  __ bind(not_taken);
  __ profile_not_taken_branch(rax);
}

void TemplateTable::ret() 
{
  transition(vtos, vtos);
  locals_index(rbx);
  __ movq(rbx, aaddress(rbx)); // get return bci, compute return bcp
  __ profile_ret(rbx, rarg3);
  __ get_method(rax);
  __ movq(r13, Address(rax, methodOopDesc::const_offset()));
  __ leaq(r13, Address(r13, rbx, Address::times_1,
                       constMethodOopDesc::codes_offset()));
  __ dispatch_next(vtos);
}

void TemplateTable::wide_ret()
{
  transition(vtos, vtos);
  locals_index_wide(rbx);
  __ movq(rbx, aaddress(rbx)); // get return bci, compute return bcp
  __ profile_ret(rbx, rarg3);
  __ get_method(rax);
  __ movq(r13, Address(rax, methodOopDesc::const_offset()));
  __ leaq(r13, Address(r13, rbx, Address::times_1, constMethodOopDesc::codes_offset()));
  __ dispatch_next(vtos);
}

void TemplateTable::tableswitch() 
{
  Label default_case, continue_execution;
  transition(itos, vtos);
  // align r13
  __ leaq(rbx, at_bcp(BytesPerInt));
  __ andq(rbx, -BytesPerInt);
  // load lo & hi
  __ movl(rarg3, Address(rbx, BytesPerInt));
  __ movl(rarg2, Address(rbx, 2 * BytesPerInt));
  __ bswapl(rarg3);
  __ bswapl(rarg2);
  // check against lo & hi
  __ cmpl(rax, rarg3);
  __ jcc(Assembler::less, default_case);
  __ cmpl(rax, rarg2);
  __ jcc(Assembler::greater, default_case);
  // lookup dispatch offset
  __ subl(rax, rarg3);
  __ movl(rarg2, Address(rbx, rax, Address::times_4, 3 * BytesPerInt));
  __ profile_switch_case(rax, rbx, rarg3);
  // continue execution
  __ bind(continue_execution);
  __ bswapl(rarg2);
  __ movslq(rarg2, rarg2);
  __ load_unsigned_byte(rbx, Address(r13, rarg2, Address::times_1));
  __ addq(r13, rarg2);
  __ dispatch_only(vtos);
  // handle default
  __ bind(default_case);
  __ profile_switch_default(rax);
  __ movl(rarg2, Address(rbx));
  __ jmp(continue_execution);
}

void TemplateTable::lookupswitch()
{
  transition(itos, itos);
  __ stop("lookupswitch bytecode should have been rewritten");
}

void TemplateTable::fast_linearswitch() 
{
  transition(itos, vtos);
  Label loop_entry, loop, found, continue_execution;  
  // bswap rax so we can avoid bswapping the table entries
  __ bswapl(rax);
  // align r13
  __ leaq(rbx, at_bcp(BytesPerInt)); // btw: should be able to get rid of
                                     // this instruction (change offsets
                                     // below)
  __ andq(rbx, -BytesPerInt);
  // set counter
  __ movl(rarg3, Address(rbx, BytesPerInt));  
  __ bswapl(rarg3);
  __ jmp(loop_entry);
  // table search
  __ bind(loop);
  __ cmpl(rax, Address(rbx, rarg3, Address::times_8, 2 * BytesPerInt));
  __ jcc(Assembler::equal, found);
  __ bind(loop_entry);
  __ decl(rarg3);
  __ jcc(Assembler::greaterEqual, loop);
  // default case
  __ profile_switch_default(rax);
  __ movl(rarg2, Address(rbx));
  __ jmp(continue_execution);
  // entry found -> get offset
  __ bind(found);
  __ movl(rarg2, Address(rbx, rarg3, Address::times_8, 3 * BytesPerInt));
  __ profile_switch_case(rarg3, rax, rbx);
  // continue execution
  __ bind(continue_execution);  
  __ bswapl(rarg2);
  __ movslq(rarg2, rarg2);
  __ load_unsigned_byte(rbx, Address(r13, rarg2, Address::times_1));
  __ addq(r13, rarg2);
  __ dispatch_only(vtos);
}

void TemplateTable::fast_binaryswitch()
{
  transition(itos, vtos);
  // Implementation using the following core algorithm:
  //
  // int binary_search(int key, LookupswitchPair* array, int n) {
  //   // Binary search according to "Methodik des Programmierens" by
  //   // Edsger W. Dijkstra and W.H.J. Feijen, Addison Wesley Germany 1985.
  //   int i = 0;
  //   int j = n;
  //   while (i+1 < j) {
  //     // invariant P: 0 <= i < j <= n and (a[i] <= key < a[j] or Q)
  //     // with      Q: for all i: 0 <= i < n: key < a[i]
  //     // where a stands for the array and assuming that the (inexisting)
  //     // element a[n] is infinitely big.
  //     int h = (i + j) >> 1;
  //     // i < h < j
  //     if (key < array[h].fast_match()) {
  //       j = h;
  //     } else {
  //       i = h;
  //     }
  //   }
  //   // R: a[i] <= key < a[i+1] or Q
  //   // (i.e., if key is within array, i is the correct index)
  //   return i;
  // }

  // Register allocation
  const Register key   = rax; // already set (tosca)
  const Register array = rbx;
  const Register i     = rarg3;
  const Register j     = rarg2;
  const Register h     = rarg0;
  const Register temp  = rarg1;

  // Find array start
  __ leaq(array, at_bcp(3 * BytesPerInt)); // btw: should be able to
                                           // get rid of this
                                           // instruction (change
                                           // offsets below)
  __ andq(array, -BytesPerInt);

  // Initialize i & j
  __ xorl(i, i);                            // i = 0;
  __ movl(j, Address(array, -BytesPerInt)); // j = length(array);    

  // Convert j into native byteordering  
  __ bswapl(j);

  // And start
  Label entry;
  __ jmp(entry);

  // binary search loop
  { 
    Label loop;
    __ bind(loop);
    // int h = (i + j) >> 1;
    __ leal(h, Address(i, j, Address::times_1)); // h = i + j;
    __ sarl(h, 1);                               // h = (i + j) >> 1;
    // if (key < array[h].fast_match()) {
    //   j = h;
    // } else {
    //   i = h;
    // }
    // Convert array[h].match to native byte-ordering before compare
    __ movl(temp, Address(array, h, Address::times_8));
    __ bswapl(temp);
    __ cmpl(key, temp);
    // j = h if (key <  array[h].fast_match())
    __ cmovl(Assembler::less, j, h);
    // i = h if (key >= array[h].fast_match())
    __ cmovl(Assembler::greaterEqual, i, h);
    // while (i+1 < j)
    __ bind(entry);
    __ leal(h, Address(i, 1)); // i+1
    __ cmpl(h, j);             // i+1 < j
    __ jcc(Assembler::less, loop);
  }

  // end of binary search, result index is i (must check again!)
  Label default_case;
  // Convert array[i].match to native byte-ordering before compare
  __ movl(temp, Address(array, i, Address::times_8));
  __ bswapl(temp);
  __ cmpl(key, temp);
  __ jcc(Assembler::notEqual, default_case);

  // entry found -> j = offset
  __ movl(j , Address(array, i, Address::times_8, BytesPerInt));
  __ profile_switch_case(i, key, array);
  __ bswapl(j);
  __ movslq(j, j);
  __ load_unsigned_byte(rbx, Address(r13, j, Address::times_1));
  __ addq(r13, j);
  __ dispatch_only(vtos);

  // default case -> j = default offset
  __ bind(default_case);
  __ profile_switch_default(i);
  __ movl(j, Address(array, -2 * BytesPerInt));
  __ bswapl(j);
  __ movslq(j, j);
  __ load_unsigned_byte(rbx, Address(r13, j, Address::times_1));
  __ addq(r13, j);
  __ dispatch_only(vtos);
}


void TemplateTable::_return(TosState state) 
{
  transition(state, state);
  assert(_desc->calls_vm(), 
         "inconsistent calls_vm information"); // call in remove_activation
  __ remove_activation(state, r13);
  __ jmp(r13);
}

// ----------------------------------------------------------------------------
// Volatile variables demand their effects be made known to all CPU's
// in order.  Store buffers on most chips allow reads & writes to
// reorder; the JMM's ReadAfterWrite.java test fails in -Xint mode
// without some kind of memory barrier (i.e., it's not sufficient that
// the interpreter does not reorder volatile references, the hardware
// also must not reorder them).
// 
// According to the new Java Memory Model (JMM):
// (1) All volatiles are serialized wrt to each other.  ALSO reads &
//     writes act as aquire & release, so:
// (2) A read cannot let unrelated NON-volatile memory refs that
//     happen after the read float up to before the read.  It's OK for
//     non-volatile memory refs that happen before the volatile read to
//     float down below it.
// (3) Similar a volatile write cannot let unrelated NON-volatile
//     memory refs that happen BEFORE the write float down to after the
//     write.  It's OK for non-volatile memory refs that happen after the
//     volatile write to float up before it.
//
// We only put in barriers around volatile refs (they are expensive),
// not _between_ memory refs (that would require us to track the
// flavor of the previous memory refs).  Requirements (2) and (3)
// require some barriers before volatile stores and after volatile
// loads.  These nearly cover requirement (1) but miss the
// volatile-store-volatile-load case.  This final case is placed after
// volatile-stores although it could just as well go before
// volatile-loads.
void TemplateTable::volatile_barrier(Assembler::Membar_mask_bits 
                                     order_constraint)
{
  // Helper function to insert a is-volatile test and memory barrier
  if (os::is_MP()) { // Not needed on single CPU
    __ membar(order_constraint);
  }
}

void TemplateTable::resolve_cache_and_index(int byte_no,
                                            Register Rcache,
                                            Register index) 
{
  assert(byte_no == 1 || byte_no == 2, "byte_no out of range");

  const Register temp = rbx;
  assert_different_registers(Rcache, index, temp);

  const int shift_count = (1 + byte_no) * BitsPerByte;
  Label resolved;
  __ get_cache_and_index_at_bcp(Rcache, index, 1);
  __ movl(temp, Address(Rcache, 
                        index, Address::times_8, 
                        constantPoolCacheOopDesc::base_offset() + 
                        ConstantPoolCacheEntry::indices_offset()));
  __ shrl(temp, shift_count);
  // have we resolved this bytecode? 
  __ andl(temp, 0xFF);
  __ cmpl(temp, (int) bytecode());
  __ jcc(Assembler::equal, resolved);

  // resolve first time through
  address entry;
  switch (bytecode()) {
  case Bytecodes::_getstatic:
  case Bytecodes::_putstatic:
  case Bytecodes::_getfield:
  case Bytecodes::_putfield:
    entry = CAST_FROM_FN_PTR(address, InterpreterRuntime::resolve_get_put);
    break;
  case Bytecodes::_invokevirtual:
  case Bytecodes::_invokespecial:
  case Bytecodes::_invokestatic:
  case Bytecodes::_invokeinterface: 
    entry = CAST_FROM_FN_PTR(address, InterpreterRuntime::resolve_invoke);
    break;
  default:
    ShouldNotReachHere();
    break;
  }
  __ movl(temp, (int) bytecode());
  __ call_VM(noreg, entry, temp);

  // Update registers with resolved info
  __ get_cache_and_index_at_bcp(Rcache, index, 1);
  __ bind(resolved);
}

void TemplateTable::load_field_cp_cache_entry(int byte_no,
                                              Register obj,
                                              Register off,
                                              Register flags,
                                              bool is_static = false) 
{
  // setup registers
  const Register cache = rarg3;
  const Register index = rarg2;
  assert_different_registers(cache, index, flags, off);

  // access constant pool cache
  resolve_cache_and_index(byte_no, cache, index);

  ByteSize cp_base_offset = constantPoolCacheOopDesc::base_offset();
  // Field offset
  __ movq(off, Address(cache, index, Address::times_8, 
                       in_bytes(cp_base_offset + 
                                ConstantPoolCacheEntry::f2_offset())));
  // Flags    
  __ movl(flags, Address(cache, index, Address::times_8,
                         in_bytes(cp_base_offset + 
                                  ConstantPoolCacheEntry::flags_offset())));

  // klass overwrite register
  if (is_static) {
    __ movq(obj, Address(cache, index, Address::times_8,
                         in_bytes(cp_base_offset + 
                                  ConstantPoolCacheEntry::f1_offset())));
  }
}

void TemplateTable::load_invoke_cp_cache_entry(int byte_no,
                                               Register method,
                                               Register itable_index,
                                               Register flags,
                                               bool is_invokevirtual,
                                               bool is_invokevfinal /*unused*/)
{
  // setup registers
  const Register cache = rarg3;
  const Register index = rarg2;
  assert_different_registers(method, flags);
  assert_different_registers(method, cache, index);
  assert_different_registers(itable_index, flags);
  assert_different_registers(itable_index, cache, index);
  // determine constant pool cache field offsets
  const int method_offset = in_bytes(
    constantPoolCacheOopDesc::base_offset() +
      (is_invokevirtual
       ? ConstantPoolCacheEntry::f2_offset()
       : ConstantPoolCacheEntry::f1_offset()));
  const int flags_offset = in_bytes(constantPoolCacheOopDesc::base_offset() +
                                    ConstantPoolCacheEntry::flags_offset());
  // access constant pool cache fields
  const int index_offset = in_bytes(constantPoolCacheOopDesc::base_offset() +
                                    ConstantPoolCacheEntry::f2_offset());

  resolve_cache_and_index(byte_no, cache, index);
  assert(wordSize == 8, "adjust code below");
  __ movq(method, Address(cache, index, Address::times_8, method_offset));
  if (itable_index != noreg) {
    __ movq(itable_index, 
            Address(cache, index, Address::times_8, index_offset));
  }
  __ movl(flags , Address(cache, index, Address::times_8, flags_offset));
}


void TemplateTable::getfield_or_static(int byte_no, bool is_static) 
{
  transition(vtos, vtos);

  // do the JVMTI work here to avoid disturbing the register state below
  if (JvmtiExport::can_post_field_access()) {
    // Check to see if a field access watch has been set before we
    // take the time to call into the VM.
    Label L1;
    __ movl(rarg3, Address((address) JvmtiExport::get_field_access_count_addr(),
                         relocInfo::none));
    __ testl(rarg3, rarg3);
    __ jcc(Assembler::zero, L1);

    // We rely on the bytecode being resolved and the cpCache entry filled in.
    resolve_cache_and_index(byte_no, rarg2, rarg3);
    // cache entry pointer
    __ addq(rarg2, in_bytes(constantPoolCacheOopDesc::base_offset()));
    __ shll(rarg3, LogBytesPerWord);
    __ addq(rarg2, rarg3);
    if (is_static) {
      __ xorl(rarg1, rarg1); // NULL object reference
    } else {
      __ movq(rarg1, at_rsp()); // get object pointer without popping it
      __ verify_oop(rarg1);
    }
    // rarg1: object pointer or NULL
    // rarg2: cache entry pointer
    // rarg3: jvalue object on the stack
    __ call_VM(noreg, CAST_FROM_FN_PTR(address, 
                                       InterpreterRuntime::post_field_access),
               rarg1, rarg2, rarg3);
    __ bind(L1);
  } 

  const Register flags = rax;
  const Register off   = rbx;
  const Register obj   = rarg3;

  load_field_cp_cache_entry(byte_no, obj, off, flags, is_static);

  if (!is_static) {
    // obj is on the stack
    __ popq(obj);   
  }

  const Address field(obj, off, Address::times_1);

  Label Done, notByte, notInt, notShort, notChar, 
              notLong, notFloat, notObj, notDouble;

  __ shrl(flags, ConstantPoolCacheEntry::tosBits);
  assert(btos == 0, "change code, btos != 0");

  __ andl(flags, 0x0F);
  __ jcc(Assembler::notZero, notByte);
  // btos
  __ load_signed_byte(rax, field);
  __ push(btos);
  // Rewrite bytecode to be faster
  if (!is_static) {
    patch_bytecode(Bytecodes::_fast_bgetfield, rarg3, rbx);
  }
  __ jmp(Done);

  __ bind(notByte);
  __ cmpl(flags, atos);
  __ jcc(Assembler::notEqual, notObj);
  // atos
  __ movq(rax, field);
  __ push(atos);
  if (!is_static) {
    patch_bytecode(Bytecodes::_fast_agetfield, rarg3, rbx);
  }
  __ jmp(Done);

  __ bind(notObj);
  __ cmpl(flags, itos);
  __ jcc(Assembler::notEqual, notInt);
  // itos
  __ movl(rax, field);
  __ push(itos);
  // Rewrite bytecode to be faster
  if (!is_static) {
    patch_bytecode(Bytecodes::_fast_igetfield, rarg3, rbx);
  }
  __ jmp(Done);

  __ bind(notInt);
  __ cmpl(flags, ctos);
  __ jcc(Assembler::notEqual, notChar);
  // ctos
  __ load_unsigned_word(rax, field);
  __ push(ctos);
  // Rewrite bytecode to be faster
  if (!is_static) {
    patch_bytecode(Bytecodes::_fast_cgetfield, rarg3, rbx);
  }
  __ jmp(Done);

  __ bind(notChar);
  __ cmpl(flags, stos);
  __ jcc(Assembler::notEqual, notShort);
  // stos
  __ load_signed_word(rax, field);
  __ push(stos);
  // Rewrite bytecode to be faster
  if (!is_static) {
    patch_bytecode(Bytecodes::_fast_sgetfield, rarg3, rbx);
  }
  __ jmp(Done);

  __ bind(notShort);
  __ cmpl(flags, ltos);
  __ jcc(Assembler::notEqual, notLong);
  // ltos
  __ movq(rax, field);
  __ push(ltos);
  // Rewrite bytecode to be faster
  if (!is_static) {
    patch_bytecode(Bytecodes::_fast_lgetfield, rarg3, rbx);
  }
  __ jmp(Done);

  __ bind(notLong);
  __ cmpl(flags, ftos);
  __ jcc(Assembler::notEqual, notFloat);
  // ftos
  __ movss(xmm0, field);
  __ push(ftos);
  // Rewrite bytecode to be faster
  if (!is_static) {
    patch_bytecode(Bytecodes::_fast_fgetfield, rarg3, rbx);
  }
  __ jmp(Done);

  __ bind(notFloat);
#ifdef ASSERT
  __ cmpl(flags, dtos);
  __ jcc(Assembler::notEqual, notDouble);
#endif
  // dtos
  __ movlpd(xmm0, field);
  __ push(dtos);
  // Rewrite bytecode to be faster
  if (!is_static) {
    patch_bytecode(Bytecodes::_fast_dgetfield, rarg3, rbx);
  }
#ifdef ASSERT
  __ jmp(Done);

  __ bind(notDouble);
  __ stop("Bad state");
#endif

  __ bind(Done);
  // [jk] not needed currently
  // volatile_barrier(Assembler::Membar_mask_bits(Assembler::LoadLoad |
  //                                              Assembler::LoadStore));
}


void TemplateTable::getfield(int byte_no) 
{
  getfield_or_static(byte_no, false);
}

void TemplateTable::getstatic(int byte_no) 
{
  getfield_or_static(byte_no, true);
}

void TemplateTable::jvmti_post_field_mod(int byte_no, bool is_static)
{
  transition(vtos, vtos);

  ByteSize cp_base_offset = constantPoolCacheOopDesc::base_offset();

  if (JvmtiExport::can_post_field_modification()) {
    // Check to see if a field modification watch has been set before
    // we take the time to call into the VM.
    Label L1;
    __ movl(rarg3, Address((address) 
                         JvmtiExport::get_field_modification_count_addr(),
                         relocInfo::none));
    __ testl(rarg3, rarg3);
    __ jcc(Assembler::zero, L1);

    // We rely on the bytecode being resolved and the cpCache entry filled in.
    resolve_cache_and_index(byte_no, rarg2, rscratch1);

    if (is_static) {
      // Life is simple.  Null out the object pointer.
      __ xorl(rarg1, rarg1);
    } else {
      // Life is harder. The stack holds the value on top, followed by
      // the object.  We don't know the size of the value, though; it
      // could be one or two words depending on its type. As a result,
      // we must find the type to determine where the object is.
      __ movl(rarg3, Address(rarg2, rscratch1, 
                           Address::times_8, 
                           in_bytes(cp_base_offset +
                                     ConstantPoolCacheEntry::flags_offset())));
      __ shrl(rarg3, ConstantPoolCacheEntry::tosBits);
      // Make sure we don't need to mask rcx for tosBits after the
      // above shift
      ConstantPoolCacheEntry::verify_tosBits();
      __ movq(rarg1, at_rsp_p1());  // initially assume a one word jvalue
      __ cmpl(rarg3, ltos);
      __ cmovq(Assembler::equal,
               rarg1, at_rsp_p2()); // ltos (two word jvalue)
      __ cmpl(rarg3, dtos);
      __ cmovq(Assembler::equal,
               rarg1, at_rsp_p2()); // dtos (two word jvalue)
    }
    // cache entry pointer
    __ addq(rarg2, in_bytes(cp_base_offset));
    __ shll(rscratch1, LogBytesPerWord);
    __ addq(rarg2, rscratch1);
    // object (tos)
    __ movq(rarg3, rsp);
    // rarg1: object pointer set up above (NULL if static)
    // rarg2: cache entry pointer
    // rarg3: jvalue object on the stack
    __ call_VM(noreg, 
               CAST_FROM_FN_PTR(address, 
                                InterpreterRuntime::post_field_modification),
               rarg1, rarg2, rarg3);
    __ bind(L1);
  }
}

void TemplateTable::putfield_or_static(int byte_no, bool is_static)
{
  transition(vtos, vtos);

  jvmti_post_field_mod(byte_no, is_static);

  const Register obj   = rarg3;
  const Register off   = rbx;
  const Register flags = rax;

  load_field_cp_cache_entry(byte_no, obj, off, flags, is_static);

  // [jk] not needed currently
  // volatile_barrier(Assembler::Membar_mask_bits(Assembler::LoadStore |
  //                                              Assembler::StoreStore));

  Label notVolatile, Done;
  __ movl(rarg2, flags);
  __ shrl(rarg2, ConstantPoolCacheEntry::volatileField);
  __ andl(rarg2, 0x1);

  // field address
  const Address field(obj, off, Address::times_1);

  Label notByte, notInt, notShort, notChar, 
        notLong, notFloat, notObj, notDouble;

  __ shrl(flags, ConstantPoolCacheEntry::tosBits);

  assert(btos == 0, "change code, btos != 0");
  __ andl(flags, 0x0f);
  __ jcc(Assembler::notZero, notByte);
  // btos
  __ pop(btos);
  if (!is_static) {
    __ popq(obj);
    __ verify_oop(obj);
  }
  __ movb(field, rax);
  if (!is_static) {
    patch_bytecode(Bytecodes::_fast_bputfield, rarg3, rbx);
  }
  __ jmp(Done);

  __ bind(notByte);
  __ cmpl(flags, atos);
  __ jcc(Assembler::notEqual, notObj);
  // atos
  __ pop(atos);
  if (!is_static) {
    __ popq(obj);
    __ verify_oop(obj);
  }
  __ movq(field, rax);
  __ store_check(obj, field); // Need to mark card
  if (!is_static) {
    patch_bytecode(Bytecodes::_fast_aputfield, rarg3, rbx);
  }
  __ jmp(Done);

  __ bind(notObj);
  __ cmpl(flags, itos);
  __ jcc(Assembler::notEqual, notInt);
  // itos
  __ pop(itos);
  if (!is_static) {
    __ popq(obj);
    __ verify_oop(obj);
  }
  __ movl(field, rax);
  if (!is_static) {
    patch_bytecode(Bytecodes::_fast_iputfield, rarg3, rbx);
  }
  __ jmp(Done);

  __ bind(notInt);
  __ cmpl(flags, ctos);
  __ jcc(Assembler::notEqual, notChar);
  // ctos
  __ pop(ctos);
  if (!is_static) {
    __ popq(obj);
    __ verify_oop(obj);
  }
  __ movw(field, rax);
  if (!is_static) {
    patch_bytecode(Bytecodes::_fast_cputfield, rarg3, rbx);
  }
  __ jmp(Done);

  __ bind(notChar);
  __ cmpl(flags, stos);
  __ jcc(Assembler::notEqual, notShort);
  // stos
  __ pop(stos);
  if (!is_static) {
    __ popq(obj);
    __ verify_oop(obj);
  }
  __ movw(field, rax);
  if (!is_static) {
    patch_bytecode(Bytecodes::_fast_sputfield, rarg3, rbx);
  }
  __ jmp(Done);

  __ bind(notShort);
  __ cmpl(flags, ltos);
  __ jcc(Assembler::notEqual, notLong);
  // ltos
  __ pop(ltos);
  if (!is_static) {
    __ popq(obj);
    __ verify_oop(obj);
  }
  __ movq(field, rax);
  if (!is_static) {
    patch_bytecode(Bytecodes::_fast_lputfield, rarg3, rbx);
  }
  __ jmp(Done);

  __ bind(notLong);
  __ cmpl(flags, ftos);
  __ jcc(Assembler::notEqual, notFloat);
  // ftos
  __ pop(ftos);
  if (!is_static) {
    __ popq(obj);
    __ verify_oop(obj);
  }
  __ movss(field, xmm0);
  if (!is_static) {
    patch_bytecode(Bytecodes::_fast_fputfield, rarg3, rbx);
  }
  __ jmp(Done);

  __ bind(notFloat);
#ifdef ASSERT
  __ cmpl(flags, dtos);
  __ jcc(Assembler::notEqual, notDouble);
#endif
  // dtos
  __ pop(dtos);
  if (!is_static) {
    __ popq(obj);
    __ verify_oop(obj);
  }
  __ movsd(field, xmm0);
  if (!is_static) {
    patch_bytecode(Bytecodes::_fast_dputfield, rarg3, rbx);
  }

#ifdef ASSERT
  __ jmp(Done);

  __ bind(notDouble);
  __ stop("Bad state");
#endif

  __ bind(Done);
  // Check for volatile store
  __ testl(rarg2, rarg2);
  __ jcc(Assembler::zero, notVolatile);
  volatile_barrier(Assembler::Membar_mask_bits(Assembler::StoreLoad |
                                               Assembler::StoreStore));

  __ bind(notVolatile);
}

void TemplateTable::putfield(int byte_no) 
{
  putfield_or_static(byte_no, false);
}

void TemplateTable::putstatic(int byte_no) 
{
  putfield_or_static(byte_no, true);
}

void TemplateTable::jvmti_post_fast_field_mod()
{
  if (JvmtiExport::can_post_field_modification()) {
    // Check to see if a field modification watch has been set before
    // we take the time to call into the VM.
    Label L2;
    __ movl(rarg3, Address(JvmtiExport::get_field_modification_count_addr(),
                         relocInfo::none));
    __ testl(rarg3, rarg3);
    __ jcc(Assembler::zero, L2);
    __ popq(rbx);                  // copy the object pointer from tos
    __ verify_oop(rbx);
    __ pushq(rbx);                 // put the object pointer back on tos
    __ subq(rsp, sizeof(jvalue));  // add space for a jvalue object
    __ movq(rarg3, rsp);
    const Address field(rarg3);

    switch (bytecode()) {          // load values into the jvalue object
    case Bytecodes::_fast_aputfield: // fall through
    case Bytecodes::_fast_lputfield: __ movq(field, rax); break;
    case Bytecodes::_fast_iputfield: __ movl(field, rax); break;
    case Bytecodes::_fast_bputfield: __ movb(field, rax); break;
    case Bytecodes::_fast_sputfield: // fall through
    case Bytecodes::_fast_cputfield: __ movw(field, rax); break;
    case Bytecodes::_fast_fputfield: __ movss(field, xmm0); break;
    case Bytecodes::_fast_dputfield: __ movsd(field, xmm0); break;
    default:
      ShouldNotReachHere();
    }

    // Save rax because call_VM() will clobber it, then use it for
    // JVMDI purposes
    __ pushq(rax);
    // access constant pool cache entry
    __ get_cache_entry_pointer_at_bcp(rarg2, rax, 1);
    __ verify_oop(rbx);
    // rbx: object pointer copied above
    // rarg2: cache entry pointer
    // rarg3: jvalue object on the stack
    __ call_VM(noreg,
               CAST_FROM_FN_PTR(address, 
                                InterpreterRuntime::post_field_modification),
               rbx, rarg2, rarg3);
    __ popq(rax);     // restore lower value   
    __ addq(rsp, sizeof(jvalue));  // release jvalue object space
    __ bind(L2);
  }
}

void TemplateTable::fast_storefield(TosState state)
{
  transition(state, vtos);

  ByteSize base = constantPoolCacheOopDesc::base_offset();

  jvmti_post_fast_field_mod();

  // access constant pool cache
  __ get_cache_and_index_at_bcp(rarg3, rbx, 1);

  // test for volatile with rarg2
  __ movl(rarg2, Address(rarg3, rbx, Address::times_8, 
                       in_bytes(base +
                                ConstantPoolCacheEntry::flags_offset())));

  // replace index with field offset from cache entry
  __ movq(rbx, Address(rarg3, rbx, Address::times_8,
                       in_bytes(base + ConstantPoolCacheEntry::f2_offset())));

  // [jk] not needed currently
  // volatile_barrier(Assembler::Membar_mask_bits(Assembler::LoadStore |
  //                                              Assembler::StoreStore));

  Label notVolatile;
  __ shrl(rarg2, ConstantPoolCacheEntry::volatileField);
  __ andl(rarg2, 0x1);

  // Get object from stack
  __ popq(rarg3);
  __ verify_oop(rarg3);

  // field address
  const Address field(rarg3, rbx, Address::times_1);

  // access field
  switch (bytecode()) {
  case Bytecodes::_fast_aputfield: 
    __ movq(field, rax); 
    __ store_check(rarg3, field);
    break;
  case Bytecodes::_fast_lputfield:
    __ movq(field, rax);
    break;
  case Bytecodes::_fast_iputfield:
    __ movl(field, rax);
    break;
  case Bytecodes::_fast_bputfield:
    __ movb(field, rax);
    break;
  case Bytecodes::_fast_sputfield:
    // fall through
  case Bytecodes::_fast_cputfield:
    __ movw(field, rax);
    break;
  case Bytecodes::_fast_fputfield:
    __ movss(field, xmm0);
    break;
  case Bytecodes::_fast_dputfield:
    __ movsd(field, xmm0); 
    break;
  default:
    ShouldNotReachHere();
  }

  // Check for volatile store
  __ testl(rarg2, rarg2);
  __ jcc(Assembler::zero, notVolatile);
  volatile_barrier(Assembler::Membar_mask_bits(Assembler::StoreLoad |
                                               Assembler::StoreStore));
  __ bind(notVolatile);
}


void TemplateTable::fast_accessfield(TosState state) 
{
  transition(atos, state);

  // Do the JVMTI work here to avoid disturbing the register state below
  if (JvmtiExport::can_post_field_access()) {
    // Check to see if a field access watch has been set before we
    // take the time to call into the VM.
    Label L1;
    __ movl(rarg3, Address((address) JvmtiExport::get_field_access_count_addr(),
                         relocInfo::none));
    __ testl(rarg3, rarg3);
    __ jcc(Assembler::zero, L1);
    // access constant pool cache entry
    __ get_cache_entry_pointer_at_bcp(rarg2, rarg3, 1);
    __ movq(r12, rax);  // save object pointer before call_VM() clobbers it
    __ verify_oop(rax);
    __ movq(rarg1, rax);
    // rarg1: object pointer copied above
    // rarg2: cache entry pointer
    __ call_VM(noreg, 
               CAST_FROM_FN_PTR(address, 
                                InterpreterRuntime::post_field_access),
               rarg1, rarg2);
    __ movq(rax, r12); // restore object pointer
    __ bind(L1);
  }

  // access constant pool cache
  __ get_cache_and_index_at_bcp(rarg3, rbx, 1);
  // replace index with field offset from cache entry
  // [jk] not needed currently
  // if (os::is_MP()) {
  //   __ movl(rarg2, Address(rarg3, rbx, Address::times_8, 
  //                        in_bytes(constantPoolCacheOopDesc::base_offset() +
  //                                 ConstantPoolCacheEntry::flags_offset())));
  //   __ shrl(rarg2, ConstantPoolCacheEntry::volatileField);
  //   __ andl(rarg2, 0x1);
  // }
  __ movq(rbx, Address(rarg3, rbx, Address::times_8,
                       in_bytes(constantPoolCacheOopDesc::base_offset() + 
                                ConstantPoolCacheEntry::f2_offset())));

  // rax: object
  __ verify_oop(rax);
  __ null_check(rax, 0);
  Address field(rax, rbx, Address::times_1);

  // access field
  switch (bytecode()) {
  case Bytecodes::_fast_agetfield:
    __ movq(rax, field);
    __ verify_oop(rax);
    break;
  case Bytecodes::_fast_lgetfield:
    __ movq(rax, field);
    break;
  case Bytecodes::_fast_igetfield:
    __ movl(rax, field);
    break;
  case Bytecodes::_fast_bgetfield: 
    __ movsbl(rax, field);
    break;
  case Bytecodes::_fast_sgetfield:
    __ load_signed_word(rax, field);
    break;
  case Bytecodes::_fast_cgetfield:
    __ load_unsigned_word(rax, field);
    break;
  case Bytecodes::_fast_fgetfield:
    __ movss(xmm0, field);
    break;
  case Bytecodes::_fast_dgetfield:
    __ movlpd(xmm0, field);
    break;
  default:
    ShouldNotReachHere();
  }
  // [jk] not needed currently
  // if (os::is_MP()) { 
  //   Label notVolatile;
  //   __ testl(rarg2, rarg2);
  //   __ jcc(Assembler::zero, notVolatile);
  //   __ membar(Assembler::LoadLoad);
  //   __ bind(notVolatile);
  //};
}

void TemplateTable::fast_xaccess(TosState state) 
{
  transition(vtos, state);

  // get receiver
  __ movq(rax, aaddress(0));
  // access constant pool cache
  __ get_cache_and_index_at_bcp(rarg3, rarg2, 2);
  __ movq(rbx, 
          Address(rarg3, rarg2, Address::times_8, 
                  in_bytes(constantPoolCacheOopDesc::base_offset() + 
                           ConstantPoolCacheEntry::f2_offset())));
  // make sure exception is reported in correct bcp range (getfield is
  // next instruction)
  __ incq(r13);
  __ null_check(rax, 0);
  switch (state) {
  case itos: 
    __ movl(rax, Address(rax, rbx, Address::times_1));
    break;
  case atos:
    __ movq(rax, Address(rax, rbx, Address::times_1));
    __ verify_oop(rax);
    break;
  case ftos:
    __ movss(xmm0, Address(rax, rbx, Address::times_1));
    break;
  default:
    ShouldNotReachHere();
  }

  // [jk] not needed currently
  // if (os::is_MP()) {
  //   Label notVolatile;
  //   __ movl(rarg2, Address(rarg3, rarg2, Address::times_8, 
  //                        in_bytes(constantPoolCacheOopDesc::base_offset() +
  //                                 ConstantPoolCacheEntry::flags_offset())));
  //   __ shrl(rarg2, ConstantPoolCacheEntry::volatileField);
  //   __ testl(rarg2, 0x1);
  //   __ jcc(Assembler::zero, notVolatile);
  //   __ membar(Assembler::LoadLoad);
  //   __ bind(notVolatile);
  // }

  __ decq(r13);
}



//-----------------------------------------------------------------------------
// Calls

void TemplateTable::count_calls(Register method, Register temp) 
{  
  // implemented elsewhere
  ShouldNotReachHere();
}

void TemplateTable::prepare_invoke(Register method, 
                                   Register index, 
                                   int byte_no, 
                                   Bytecodes::Code code) 
{
  // determine flags
  const bool is_invokeinterface  = code == Bytecodes::_invokeinterface;
  const bool is_invokevirtual    = code == Bytecodes::_invokevirtual;
  const bool is_invokespecial    = code == Bytecodes::_invokespecial;
  const bool load_receiver       = code != Bytecodes::_invokestatic;
  const bool receiver_null_check = is_invokespecial;
  const bool save_flags = is_invokeinterface || is_invokevirtual;
  // setup registers & access constant pool cache
  const Register recv   = rarg3;
  const Register flags  = rarg2;  
  assert_different_registers(method, index, recv, flags);

  // save 'interpreter return address'
  __ save_bcp();

  load_invoke_cp_cache_entry(byte_no, method, index, flags, is_invokevirtual);

  // load receiver if needed (note: no return address pushed yet)
  if (load_receiver) {
    __ movl(recv, flags);
    __ andl(recv, 0xFF);
    __ movq(recv, Address(rsp, recv, Address::times_8, -wordSize));
    __ verify_oop(recv);
  }

  // do null check if needed
  if (receiver_null_check) {
    __ null_check(recv);
  }

  if (save_flags) {
    __ movl(r13, flags);
  }

  // compute return type
  __ shrl(flags, ConstantPoolCacheEntry::tosBits);
  // Make sure we don't need to mask flags for tosBits after the above shift
  ConstantPoolCacheEntry::verify_tosBits();
  // load return address
  { 
    const int64_t table =
      is_invokeinterface
      ? (int64_t) Interpreter::return_5_addrs_by_index_table()
      : (int64_t) Interpreter::return_3_addrs_by_index_table();
    __ movq(rscratch1, table);
    __ movq(flags, Address(rscratch1, flags, Address::times_8));
  }

  // push return address
  __ pushq(flags);

  // Restore flag field from the constant pool cache, and restore esi
  // for later null checks.  esi is the bytecode pointer
  if (save_flags) {
    __ movl(flags, r13);
    __ restore_bcp();
  }
}


void TemplateTable::invokevirtual_helper(Register index,
                                         Register recv,
                                         Register flags) 
{
  // Uses temporary registers rax, rarg2  assert_different_registers(index, recv, rax, rarg2);

  // Test for an invoke of a final method
  Label notFinal;
  __ movl(rax, flags);
  __ andl(rax, (1 << ConstantPoolCacheEntry::vfinalMethod));
  __ jcc(Assembler::zero, notFinal);

  const Register method = index;  // method must be rbx
  assert(method == rbx,
         "methodOop must be rbx for interpreter calling convention");

  // do the call - the index is actually the method to call
  __ verify_oop(method);

  // It's final, need a null check here!
  __ null_check(recv);

  // profile this call
  __ profile_final_call(rax);

  __ jmp(Address(method, methodOopDesc::interpreter_entry_offset()));

  __ bind(notFinal);

  // get receiver klass
  __ null_check(recv, oopDesc::klass_offset_in_bytes());
  // Keep recv in rcx for callee expects it there
  __ movq(rax, Address(recv, oopDesc::klass_offset_in_bytes()));

  __ verify_oop(rax);

  // profile this call
  __ profile_virtual_call(rax, r14, rarg2);

  // get target methodOop & entry point
  const int base = instanceKlass::vtable_start_offset() * wordSize;    
  assert(vtableEntry::size() * wordSize == 8, 
         "adjust the scaling in the code below");
  __ movq(method, Address(rax, index, 
                          Address::times_8, 
                          base + vtableEntry::method_offset_in_bytes()));  
  __ movq(rarg2, Address(method, methodOopDesc::interpreter_entry_offset()));  
  __ movq(rax, method); // the i2c_adapters need methodOop in rax  
  // do the call
#ifdef ASSERT
  { 
    Label L;
    __ testq(rarg2, rarg2);
    __ jcc(Assembler::notZero, L);
    __ stop("interpreter entry point is null");
    __ bind(L);
  }
#endif
  __ jmp(rarg2);
}


void TemplateTable::invokevirtual(int byte_no) 
{
  transition(vtos, vtos);
  prepare_invoke(rbx, noreg, byte_no, bytecode());

  // rbx: index
  // rarg3: receiver    
  // rarg2: flags    

  invokevirtual_helper(rbx, rarg3, rarg2);
}


void TemplateTable::invokespecial(int byte_no) 
{
  transition(vtos, vtos);
  prepare_invoke(rbx, noreg, byte_no, bytecode());
  // do the call
  __ verify_oop(rbx);
  __ profile_call(rax);
  __ jmp(Address(rbx, methodOopDesc::interpreter_entry_offset()));
}


void TemplateTable::invokestatic(int byte_no) 
{
  transition(vtos, vtos);
  prepare_invoke(rbx, noreg, byte_no, bytecode());
  // do the call
  __ verify_oop(rbx);
  __ profile_call(rax);
  __ jmp(Address(rbx, methodOopDesc::interpreter_entry_offset()));
}

void TemplateTable::fast_invokevfinal(int byte_no)
{
  transition(vtos, vtos);
  __ stop("fast_invokevfinal not used on amd64");
}

void TemplateTable::invokeinterface(int byte_no) 
{
  transition(vtos, vtos);
  prepare_invoke(rax, rbx, byte_no, bytecode());
  
  // rax: Interface
  // rbx: index
  // rarg3: receiver    
  // rarg2: flags

  // Special case of invokeinterface called for virtual method of
  // java.lang.Object.  See cpCacheOop.cpp for details.
  // This code isn't produced by javac, but could be produced by
  // another compliant java compiler.
  Label notMethod;
  __ movl(r14, rarg2);
  __ andl(r14, (1 << ConstantPoolCacheEntry::methodInterface));
  __ jcc(Assembler::zero, notMethod);

  invokevirtual_helper(rbx, rarg3, rarg2);
  __ bind(notMethod);

  // Get receiver klass into rarg2 - also a null check
  __ restore_locals(); // restore r14
  __ movq(rarg2, Address(rarg3, oopDesc::klass_offset_in_bytes()));
  __ verify_oop(rarg2);

  // profile this call
  __ profile_virtual_call(rarg2, r13, r14);

  __ movq(r14, rarg2); // Save klassOop in r14

  // Compute start of first itableOffsetEntry (which is at the end of
  // the vtable)
  const int base = instanceKlass::vtable_start_offset() * wordSize;
  // Get length of vtable
  assert(vtableEntry::size() * wordSize == 8, 
         "adjust the scaling in the code below");
  __ movl(r13, Address(rarg2, 
                       instanceKlass::vtable_length_offset() * wordSize));
  __ leaq(rarg2, Address(rarg2, r13, Address::times_8, base));
  
  if (HeapWordsPerLong > 1) {
    // Round up to align_object_offset boundary
    __ round_to_q(rarg2, BytesPerLong);
  }

  Label entry, search, interface_ok;
  
  __ jmp(entry);   
  __ bind(search);
  __ addq(rarg2, itableOffsetEntry::size() * wordSize);
  
  __ bind(entry);    

  // Check that the entry is non-null.  A null entry means that the
  // receiver class doesn't implement the interface, and wasn't the
  // same as the receiver class checked when the interface was
  // resolved.
  __ pushq(rarg2);
  __ movq(rarg2, Address(rarg2, itableOffsetEntry::interface_offset_in_bytes()));
  __ testq(rarg2, rarg2);
  __ jcc(Assembler::notZero, interface_ok);
  // throw exception
  __ popq(rarg2); // pop saved register first.
  __ popq(rbx); // pop return address (pushed by prepare_invoke)
  __ restore_bcp(); // r13 must be correct for exception handler (was
                    // destroyed)
  __ restore_locals(); // make sure locals pointer is correct as well
                       // (was destroyed)
  __ call_VM(noreg, CAST_FROM_FN_PTR(address,
                   InterpreterRuntime::throw_IncompatibleClassChangeError));
  // the call_VM checks for exception, so we should never return here.
  __ should_not_reach_here();
  __ bind(interface_ok);

  __ popq(rarg2);

  __ cmpq(rax, Address(rarg2, itableOffsetEntry::interface_offset_in_bytes()));
  __ jcc(Assembler::notEqual, search);
        
  __ movl(rarg2, Address(rarg2, itableOffsetEntry::offset_offset_in_bytes()));

  __ addq(rarg2, r14); // Add offset to klassOop
  assert(itableMethodEntry::size() * wordSize == 8,
         "adjust the scaling in the code below");
  __ movq(rbx, Address(rarg2, rbx, Address::times_8));
  // rbx: methodOop to call
  // rarg3: receiver
  // Check for abstract method error
  // Note: This should be done more efficiently via a
  // throw_abstract_method_error interpreter entry point and a
  // conditional jump to it in case of a null method.
  { 
    Label L;
    __ testq(rbx, rbx);
    __ jcc(Assembler::notZero, L);
    // throw exception
    // note: must restore interpreter registers to canonical
    //       state for exception handling to work correctly!
    __ popq(rbx);  // pop return address (pushed by prepare_invoke)
    __ restore_bcp(); // r13 must be correct for exception handler
                      // (was destroyed)
    __ restore_locals(); // make sure locals pointer is correct as
                         // well (was destroyed)
    __ call_VM(noreg, 
               CAST_FROM_FN_PTR(address, 
                             InterpreterRuntime::throw_AbstractMethodError));
    // the call_VM checks for exception, so we should never return here.
    __ should_not_reach_here();
    __ bind(L);
  }

  __ movq(rarg2, Address(rbx, methodOopDesc::interpreter_entry_offset()));  

  COMPILER2_ONLY(__ movq(rax, rbx);) // the i2c_adapters need methodOop in rax

  // do the call
#ifdef ASSERT
  { 
    Label L;
    __ testq(rarg2, rarg2);
    __ jcc(Assembler::notZero, L);
    __ stop("interpreter entry point is null");
    __ bind(L);
  }
#endif    
  // rarg3: receiver
  // rbx: methodOop
  __ jmp(rarg2);  
}

//-----------------------------------------------------------------------------
// Allocation

void TemplateTable::_new() 
{
  transition(vtos, atos);
  __ get_unsigned_2_byte_index_at_bcp(rarg2, 1);
  Label slow_case;
  Label done;
  Label initialize_header;
  Label initialize_object; // including clearing the fields
  Label allocate_shared;

  __ get_cpool_and_tags(rarg1, rax);
  // get instanceKlass
  __ movq(rarg1, Address(rarg1, rarg2, 
                       Address::times_8, sizeof(constantPoolOopDesc)));

  // make sure the class we're about to instantiate has been
  // resolved. Note: slow_case does a pop of stack, which is why we
  // loaded class/pushed above
  const int tags_offset = typeArrayOopDesc::header_size(T_BYTE) * wordSize;
  __ cmpb(Address(rax, rarg2, Address::times_1, tags_offset),
          JVM_CONSTANT_UnresolvedClass);
  __ jcc(Assembler::equal, slow_case);

  // make sure klass is initialized & doesn't have finalizer
  // make sure klass is fully initialized
  __ cmpl(Address(rarg1, 
                  instanceKlass::init_state_offset_in_bytes() + 
                  sizeof(oopDesc)), 
          instanceKlass::fully_initialized);
  __ jcc(Assembler::notEqual, slow_case);
  // has_finalizer
  __ movl(rax,
          Address(rarg1, 
                  Klass::access_flags_offset_in_bytes() + sizeof(oopDesc)));
  __ testl(rax, JVM_ACC_CAN_BE_FASTPATH_ALLOCATED);
  __ jcc(Assembler::zero, slow_case);

  // get instance_size in instanceKlass (already aligned)
  __ movl(rarg2, 
          Address(rarg1, 
                  Klass::size_helper_offset_in_bytes() + sizeof(oopDesc)));

  // Allocate the instance
  // 1) Try to allocate in the TLAB
  // 2) if fail and the object is large allocate in the shared Eden
  // 3) if the above fails (or is not applicable), go to a slow case
  // (creates a new TLAB, etc.)

  if (UseTLAB) {
    __ movq(rax, Address(r15_thread, in_bytes(JavaThread::tlab_top_offset())));
    __ leaq(rbx, Address(rax, rarg2, Address::times_8));
    __ cmpq(rbx, Address(r15_thread, in_bytes(JavaThread::tlab_end_offset())));
    __ jcc(Assembler::above, allocate_shared);
    __ movq(Address(r15_thread, in_bytes(JavaThread::tlab_top_offset())), rbx);
    if (ZeroTLAB) {
      // the fields have been already cleared
      __ jmp(initialize_header);
    } else {
      // initialize both the header and fields
      __ jmp(initialize_object);

      if (CMSIncrementalMode) {
        // No allocation in shared eden. 
        __ jmp(slow_case);
      }
    }
  }

  // Allocation in the shared Eden
  //
  // edx: instance size in words
  __ bind(allocate_shared);

  {
    const Register RtopAddr = rscratch1;
    const Register RendAddr = rscratch2;

    __ movq(RtopAddr, (int64_t) Universe::heap()->top_addr());
    __ movq(RendAddr, (int64_t) Universe::heap()->end_addr());
    __ movq(rax, Address(RtopAddr));

    // For retries rax gets set by cmpxchgq
    Label retry;
    __ bind(retry);
    __ leaq(rbx, Address(rax, rarg2, Address::times_8));
    __ cmpq(rbx, Address(RendAddr));
    __ jcc(Assembler::above, slow_case);

    // Compare rax with the top addr, and if still equal, store the new
    // top addr in rbx at the address of the top addr pointer. Sets ZF if was
    // equal, and clears it otherwise. Use lock prefix for atomicity on MPs.
    //
    // rax: object begin
    // rbx: object end
    // edx: instance size in words
    if (os::is_MP()) {
      __ lock();
    }
    __ cmpxchgq(rbx, Address(RtopAddr));

    // if someone beat us on the allocation, try again, otherwise continue 
    __ jcc(Assembler::notEqual, retry);
  }

  // The object is initialized before the header.  If the object size is
  // zero, go directly to the header initialization.
  __ bind(initialize_object);
  __ decrementl(rarg2, sizeof(oopDesc) / oopSize);
  __ jcc(Assembler::zero, initialize_header);

  // Initialize object fields
  __ xorl(rarg3, rarg3); // use zero reg to clear memory (shorter code)
  { 
    Label loop;
    __ bind(loop);
    __ movq(Address(rax, rarg2, Address::times_8, sizeof(oopDesc) - oopSize), 
            rarg3);
    __ decl(rarg2);
    __ jcc(Assembler::notZero, loop);
  }

  // initialize object header only.
  __ bind(initialize_header);
  __ movq(Address(rax, oopDesc::mark_offset_in_bytes()), 
          (int) markOopDesc::prototype()); // header (address 0x1)
  __ movq(Address(rax, oopDesc::klass_offset_in_bytes()), rarg1);  // klass
  __ jmp(done);

  // slow case
  __ bind(slow_case);
  __ get_constant_pool(rarg1);
  __ get_unsigned_2_byte_index_at_bcp(rarg2, 1);
  call_VM(rax, CAST_FROM_FN_PTR(address, InterpreterRuntime::_new), rarg1, rarg2);
  __ verify_oop(rax);

  // continue
  __ bind(done);
}

void TemplateTable::newarray() 
{
  transition(itos, atos);
  __ load_unsigned_byte(rarg1, at_bcp(1));
  __ movl(rarg2, rax);
  call_VM(rax, CAST_FROM_FN_PTR(address, InterpreterRuntime::newarray),
          rarg1, rarg2);
}

void TemplateTable::anewarray() 
{
  transition(itos, atos);
  __ get_unsigned_2_byte_index_at_bcp(rarg2, 1);
  __ get_constant_pool(rarg1);
  __ movl(rarg3, rax);
  call_VM(rax, CAST_FROM_FN_PTR(address, InterpreterRuntime::anewarray), 
          rarg1, rarg2, rarg3);
}

void TemplateTable::arraylength() 
{
  transition(atos, itos);
  __ null_check(rax, arrayOopDesc::length_offset_in_bytes());
  __ movl(rax, Address(rax, arrayOopDesc::length_offset_in_bytes()));
}

void TemplateTable::checkcast() 
{
  transition(atos, atos);
  Label done, is_null, ok_is_subtype, quicked, resolved;
  __ testq(rax, rax); // object is in rax
  __ jcc(Assembler::zero, is_null);

  __ profile_checkcast(false, rarg3); // Blows rarg3

  // Get cpool & tags index
  __ get_cpool_and_tags(rarg3, rarg2); // rarg3=cpool, rarg2=tags array
  __ get_unsigned_2_byte_index_at_bcp(rbx, 1); // rbx=index
  // See if bytecode has already been quicked
  __ cmpb(Address(rarg2, rbx, 
                  Address::times_1, 
                  typeArrayOopDesc::header_size(T_BYTE) * wordSize),
          JVM_CONSTANT_Class);
  __ jcc(Assembler::equal, quicked);

  __ movq(r12, rarg3); // save rarg3 XXX
  __ push(atos); // save receiver for result, and for GC
  call_VM(rax, CAST_FROM_FN_PTR(address, InterpreterRuntime::quicken_io_cc));
  __ pop_ptr(rarg2); // restore receiver
  __ movq(rarg3, r12); // restore rarg3 XXX
  __ jmp(resolved);

  // Get superklass in rax and subklass in rbx
  __ bind(quicked);
  __ movq(rarg2, rax); // Save object in rarg2; rax needed for subtype check
  __ movq(rax, Address(rarg3, rbx, 
                       Address::times_8, sizeof(constantPoolOopDesc)));

  __ bind(resolved);
  __ movq(rbx, Address(rarg2, oopDesc::klass_offset_in_bytes()));

  // Generate subtype check.  Blows rarg3.  Resets rarg0.  Object in rarg2.
  // Superklass in rax.  Subklass in rbx.
  __ gen_subtype_check(rbx, ok_is_subtype);

  // Come here on failure
  __ pushq(rarg2);
  // object is at TOS
  __ jmp(Interpreter::_throw_ClassCastException_entry, relocInfo::none);

  // Come here on success
  __ bind(ok_is_subtype);
  __ movq(rax, rarg2); // Restore object in rarg2

  // Collect counts on whether this check-cast sees NULLs a lot or not.
  if (ProfileInterpreter) {
    __ jmp(done);
  }
  __ bind(is_null);
  __ profile_checkcast(true, rarg3);
  __ bind(done);
}

void TemplateTable::instanceof() 
{
  transition(atos, itos);
  Label done, ok_is_subtype, quicked, resolved;
  __ testq(rax, rax);
  __ jcc(Assembler::zero, done);

  // Get cpool & tags index
  __ get_cpool_and_tags(rarg3, rarg2); // rarg3=cpool, rarg2=tags array
  __ get_unsigned_2_byte_index_at_bcp(rbx, 1); // rbx=index
  // See if bytecode has already been quicked
  __ cmpb(Address(rarg2, rbx,
                  Address::times_1, 
                  typeArrayOopDesc::header_size(T_BYTE) * wordSize),
          JVM_CONSTANT_Class);
  __ jcc(Assembler::equal, quicked);

  __ movq(r12, rarg3); // save rarg3
  __ push(atos); // save receiver for result, and for GC
  call_VM(rax, CAST_FROM_FN_PTR(address, InterpreterRuntime::quicken_io_cc));
  __ pop_ptr(rarg2); // restore receiver
  __ movq(rarg2, Address(rarg2, oopDesc::klass_offset_in_bytes()));
  __ movq(rarg3, r12); // restore rarg3
  __ jmp(resolved);

  // Get superklass in rax and subklass in rdx
  __ bind(quicked);
  __ movq(rarg2, Address(rax, oopDesc::klass_offset_in_bytes()));
  __ movq(rax, Address(rarg3, rbx,
                       Address::times_8, sizeof(constantPoolOopDesc)));

  __ bind(resolved);

  // Generate subtype check.  Blows rarg3.  Resets rarg0.
  // Superklass in rax.  Subklass in rarg2.
  __ gen_subtype_check(rarg2, ok_is_subtype);

  // Come here on failure
  __ xorl(rax, rax);
  __ jmp(done);
  // Come here on success
  __ bind(ok_is_subtype);
  __ movl(rax, 1);

  __ bind(done);
  // rax = 0: obj == NULL or  obj is not an instanceof the specified klass
  // rax = 1: obj != NULL and obj is     an instanceof the specified klass
}

//-----------------------------------------------------------------------------
// Breakpoints
void TemplateTable::_breakpoint() 
{
  // Note: We get here even if we are single stepping..
  // jbug inists on setting breakpoints at every bytecode 
  // even if we are in single step mode.  
 
  transition(vtos, vtos);

  // get the unpatched byte code
  __ get_method(rarg1);
  __ call_VM(noreg, 
             CAST_FROM_FN_PTR(address, 
                              InterpreterRuntime::get_original_bytecode_at),
             rarg1, r13);
  __ movq(rbx, rax);

  // post the breakpoint event
  __ get_method(rarg1);
  __ call_VM(noreg, 
             CAST_FROM_FN_PTR(address, InterpreterRuntime::_breakpoint),
             rarg1, r13);

  // complete the execution of original bytecode
  __ dispatch_only_normal(vtos);
} 

//-----------------------------------------------------------------------------
// Exceptions

void TemplateTable::athrow() 
{
  transition(atos, vtos);
  __ null_check(rax);
  __ jmp(Interpreter::throw_exception_entry(), relocInfo::none);
}

//-----------------------------------------------------------------------------
// Synchronization
//
// Note: monitorenter & exit are symmetric routines; which is reflected
//       in the assembly code structure as well
//
// Stack layout:
//
// [expressions  ] <--- rsp               = expression stack top
// ..
// [expressions  ]
// [monitor entry] <--- monitor block top = expression stack bot
// ..
// [monitor entry]
// [frame data   ] <--- monitor block bot
// ...
// [saved rbp    ] <--- rbp
void TemplateTable::monitorenter() 
{
  transition(atos, vtos);

  // check for NULL object
  __ null_check(rax);

  const Address monitor_block_top(
        rbp, frame::interpreter_frame_monitor_block_top_offset * wordSize);
  const Address monitor_block_bot(
        rbp, frame::interpreter_frame_initial_sp_offset * wordSize);
  const int entry_size = frame::interpreter_frame_monitor_size() * wordSize;

  Label allocated;

  // initialize entry pointer
  __ xorl(rarg1, rarg1); // points to free slot or NULL

  // find a free slot in the monitor block (result in rarg1)
  { 
    Label entry, loop, exit;
    __ movq(rarg3, monitor_block_top); // points to current entry,
                                     // starting with top-most entry
    __ leaq(rarg2, monitor_block_bot); // points to word before bottom
                                     // of monitor block
    __ jmp(entry);

    __ bind(loop);
    // check if current entry is used
    __ cmpq(Address(rarg3, BasicObjectLock::obj_offset_in_bytes()), (int) NULL);
    // if not used then remember entry in rarg1
    __ cmovq(Assembler::equal, rarg1, rarg3); 
    // check if current entry is for same object
    __ cmpq(rax, Address(rarg3, BasicObjectLock::obj_offset_in_bytes()));
    // if same object then stop searching
    __ jcc(Assembler::equal, exit);
    // otherwise advance to next entry
    __ addq(rarg3, entry_size);
    __ bind(entry);
    // check if bottom reached
    __ cmpq(rarg3, rarg2);
    // if not at bottom then check this entry
    __ jcc(Assembler::notEqual, loop);
    __ bind(exit);
  }

  __ testq(rarg1, rarg1); // check if a slot has been found
  __ jcc(Assembler::notZero, allocated); // if found, continue with that one

  // allocate one if there's no free slot
  { 
    Label entry, loop;
    // 1. compute new pointers       // rsp: old expression stack top
    __ movq(rarg1, monitor_block_bot); // rarg1: old expression stack bottom
    __ subq(rsp, entry_size);        // move expression stack top
    __ subq(rarg1, entry_size);        // move expression stack bottom
    __ movq(rarg3, rsp);               // set start value for copy loop
    __ movq(monitor_block_bot, rarg1); // set new monitor block bottom
    __ jmp(entry);
    // 2. move expression stack contents
    __ bind(loop);
    __ movq(rarg2, Address(rarg3, entry_size)); // load expression stack
                                            // word from old location
    __ movq(Address(rarg3), rarg2);             // and store it at new location
    __ addq(rarg3, wordSize);                 // advance to next word
    __ bind(entry);
    __ cmpq(rarg3, rarg1);                      // check if bottom reached
    __ jcc(Assembler::notEqual, loop);      // if not at bottom then
                                            // copy next word
  }
  
  // call run-time routine
  // rarg1: points to monitor entry
  __ bind(allocated);

  // Increment bcp to point to the next bytecode, so exception
  // handling for async. exceptions work correctly.
  // The object has already been poped from the stack, so the
  // expression stack looks correct.
  __ incq(r13);

  // store object  
  __ movq(Address(rarg1, BasicObjectLock::obj_offset_in_bytes()), rax); 
  __ lock_object(rarg1);

  // The bcp has already been incremented. Just need to dispatch to
  // next instruction.
  __ dispatch_next(vtos);
}


void TemplateTable::monitorexit()
{
  transition(atos, vtos);

  // check for NULL object
  __ null_check(rax);

  const Address monitor_block_top(
        rbp, frame::interpreter_frame_monitor_block_top_offset * wordSize);
  const Address monitor_block_bot(
        rbp, frame::interpreter_frame_initial_sp_offset * wordSize);
  const int entry_size = frame::interpreter_frame_monitor_size() * wordSize;

  Label found;

  // find matching slot
  { 
    Label entry, loop;
    __ movq(rarg1, monitor_block_top); // points to current entry,
                                     // starting with top-most entry
    __ leaq(rarg2, monitor_block_bot); // points to word before bottom
                                     // of monitor block
    __ jmp(entry);

    __ bind(loop);
    // check if current entry is for same object
    __ cmpq(rax, Address(rarg1, BasicObjectLock::obj_offset_in_bytes()));
    // if same object then stop searching
    __ jcc(Assembler::equal, found);
    // otherwise advance to next entry
    __ addq(rarg1, entry_size);
    __ bind(entry);
    // check if bottom reached
    __ cmpq(rarg1, rarg2);
    // if not at bottom then check this entry
    __ jcc(Assembler::notEqual, loop);
  }

  // error handling. Unlocking was not block-structured
  __ call_VM(noreg, CAST_FROM_FN_PTR(address, 
                   InterpreterRuntime::throw_illegal_monitor_state_exception));
  __ should_not_reach_here();

  // call run-time routine
  // rsi: points to monitor entry
  __ bind(found);
  __ pushq(rax); // make sure object is on stack (contract with oopMaps)  
  __ unlock_object(rarg1);    
  __ popq(rax); // discard object  
}


// Wide instructions
void TemplateTable::wide() 
{
  transition(vtos, vtos);
  __ load_unsigned_byte(rbx, at_bcp(1));
  __ movq(rscratch1, (int64_t) Interpreter::_wentry_point);
  __ jmp(Address(rscratch1, rbx, Address::times_8));
  // Note: the r13 increment step is part of the individual wide
  // bytecode implementations
}


// Multi arrays
void TemplateTable::multianewarray() 
{
  transition(vtos, atos);
  __ load_unsigned_byte(rax, at_bcp(3)); // get number of dimensions
  // last dim is on top of stack; we want address of first one:
  // first_addr = last_addr + (ndims - 1) * wordSize
  __ leaq(rarg1, Address(rsp, rax, Address::times_8, -wordSize));
  call_VM(rax, 
          CAST_FROM_FN_PTR(address, InterpreterRuntime::multianewarray),
          rarg1);
  __ load_unsigned_byte(rbx, at_bcp(3));
  __ leaq(rsp, Address(rsp, rbx, Address::times_8));  // get rid of counts
}
