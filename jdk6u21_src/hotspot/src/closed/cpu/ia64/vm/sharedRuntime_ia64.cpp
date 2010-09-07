/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_sharedRuntime_ia64.cpp.incl"

#ifdef COMPILER2
UncommonTrapBlob   *SharedRuntime::_uncommon_trap_blob;
DeoptimizationBlob *SharedRuntime::_deopt_blob;
#endif // COMPILER2

SafepointBlob      *SharedRuntime::_polling_page_safepoint_handler_blob;
SafepointBlob      *SharedRuntime::_polling_page_return_handler_blob;
RuntimeStub*       SharedRuntime::_wrong_method_blob;
RuntimeStub*       SharedRuntime::_ic_miss_blob;
RuntimeStub*       SharedRuntime::_resolve_opt_virtual_call_blob;
RuntimeStub*       SharedRuntime::_resolve_virtual_call_blob;
RuntimeStub*       SharedRuntime::_resolve_static_call_blob;

#define __ masm->

class RegisterSaver {

  friend class MacroAssembler;

  public:

  static OopMap* save_live_registers(MacroAssembler* masm, int* total_frame_words);
  static void restore_live_registers(MacroAssembler* masm);

  // During deoptimization only the result register need to be restored
  // all the other values have already been extracted.

  static void restore_result_registers(MacroAssembler* masm);
};

// Second set_callee_saved  is really a waste but we'll keep things as they were for now
#define ADD_CALLEE_TO_MAP(offset, regname)                                         \
  map->set_callee_saved(VMRegImpl::stack2reg((MacroAssembler::offset*wordSize) >> 2),       \
                        regname->as_VMReg());                            \
  map->set_callee_saved(VMRegImpl::stack2reg(((MacroAssembler::offset*wordSize) + 4) >> 2),  \
                        regname->as_VMReg()->next());


OopMap* RegisterSaver::save_live_registers(MacroAssembler* masm, int* total_frame_words) {

  int frame_size_in_bytes = MacroAssembler::push_regs_size*wordSize;

  assert(frame_size_in_bytes == round_to(MacroAssembler::push_regs_size*wordSize, 16),
         "Not 16 byte aligned");

  // OopMap frame size is in compiler stack slots (jint's) not bytes or words
  int frame_size_in_slots = frame_size_in_bytes / BytesPerInt;

  int frame_size_in_words = frame_size_in_bytes / wordSize;
  *total_frame_words = frame_size_in_words;

  // We can use GR31_SCRATCH here since we are coming from compiled
  // code.  GR31 is a very local temp register in ia64.ad.
  // If it's use changes, this code will need to change.
  __ mov(GR31_SCRATCH, RP);
  __ push_dummy_full_frame(GR31_SCRATCH);
  __ push_scratch_regs();

  // Set an oopmap for the call site.  This oopmap will map all
  // oop-registers and debug-info registers as callee-saved.  This
  // will allow deoptimization at this safepoint to find all possible
  // debug-info recordings, as well as let GC find all oops.

  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map = new OopMap(frame_size_in_slots, 0);

  ADD_CALLEE_TO_MAP(gr2_off, GR2);
  ADD_CALLEE_TO_MAP(gr3_off, GR3);
  ADD_CALLEE_TO_MAP(gr8_off, GR8);
  ADD_CALLEE_TO_MAP(gr9_off, GR9);
  ADD_CALLEE_TO_MAP(gr10_off, GR10);
  ADD_CALLEE_TO_MAP(gr11_off, GR11);
  ADD_CALLEE_TO_MAP(gr14_off, GR14);
  ADD_CALLEE_TO_MAP(gr15_off, GR15);
  ADD_CALLEE_TO_MAP(gr16_off, GR16);
  ADD_CALLEE_TO_MAP(gr17_off, GR17);
  ADD_CALLEE_TO_MAP(gr18_off, GR18);
  ADD_CALLEE_TO_MAP(gr19_off, GR19);
  ADD_CALLEE_TO_MAP(gr20_off, GR20);
  ADD_CALLEE_TO_MAP(gr21_off, GR21);
  ADD_CALLEE_TO_MAP(gr22_off, GR22);
  ADD_CALLEE_TO_MAP(gr23_off, GR23);
  ADD_CALLEE_TO_MAP(gr24_off, GR24);
  ADD_CALLEE_TO_MAP(gr25_off, GR25);
  ADD_CALLEE_TO_MAP(gr26_off, GR26);
  ADD_CALLEE_TO_MAP(gr27_off, GR27);
  ADD_CALLEE_TO_MAP(gr28_off, GR28);
  ADD_CALLEE_TO_MAP(gr29_off, GR29);
  ADD_CALLEE_TO_MAP(gr30_off, GR30);
  ADD_CALLEE_TO_MAP(gr31_off, GR31);

  ADD_CALLEE_TO_MAP(fr6_off, FR6);
  ADD_CALLEE_TO_MAP(fr7_off, FR7);
  ADD_CALLEE_TO_MAP(fr8_off, FR8);
  ADD_CALLEE_TO_MAP(fr9_off, FR9);
  ADD_CALLEE_TO_MAP(fr10_off, FR10);
  ADD_CALLEE_TO_MAP(fr11_off, FR11);
  ADD_CALLEE_TO_MAP(fr12_off, FR12);
  ADD_CALLEE_TO_MAP(fr13_off, FR13);
  ADD_CALLEE_TO_MAP(fr14_off, FR14);
  ADD_CALLEE_TO_MAP(fr15_off, FR15);

  return map;
}


// Pop the current frame and restore all the registers that we
// saved.
void RegisterSaver::restore_live_registers(MacroAssembler* masm) {
  __ pop_scratch_regs();
  // pop_dummy_full_frame is going to use GR2, save it in GR31_SCRATCH
  __ mov(GR31_SCRATCH, GR2);
  __ pop_dummy_full_frame();
  __ mov(GR2, GR31_SCRATCH);
}

// Pop the current frame and restore the registers that might be holding
// a result.
void RegisterSaver::restore_result_registers(MacroAssembler* masm) {

}

// ---------------------------------------------------------------------------
// Read the array of BasicTypes from a signature, and compute where the
// arguments should go.  Values in the VMRegPair regs array refer to 4-byte
// quantities.  Values less than VMRegImpl::stack0 are registers, those above
// refer to 4-byte stack slots.  All stack slots are based off of the window
// top.  VMRegImpl::stack0 refers to the first slot past the 16-word window,
// and VMRegImpl::stack0+1 refers to the memory word 4-byes higher.  Register
// values 0-63 (up to RegisterImpl::number_of_registers) are the 64-bit
// integer registers.  Values 64-95 are the (32-bit only) float registers.
// Each 32-bit quantity is given its own number, so the integer registers
// (in either 32- or 64-bit builds) use 2 numbers.  For example, there is
// an O0-low and an O0-high.  Essentially, all int register numbers are doubled.

// Register results are passed in O0-O5, for outgoing call arguments.  To
// convert to incoming arguments, convert all O's to I's.  The regs array
// refer to the low and hi 32-bit words of 64-bit registers or stack slots.
// If the regs[].second() field is set to VMRegImpl::Bad(), it means it's unused (a
// 32-bit value was passed).  If both are VMRegImpl::Bad(), it means no value was
// passed (used as a placeholder for the other half of longs and doubles in
// the 64-bit build).  regs[].second() is either VMRegImpl::Bad() or regs[].second() is
// regs[].first()+1 (regs[].first() may be misaligned in the C calling convention).
// Sparc never passes a value in regs[].second() but not regs[].first() (regs[].first()
// == VMRegImpl::Bad() && regs[].second() != VMRegImpl::Bad()) nor unrelated values in the
// same VMRegPair.

// QQQ Comment needs serious work

// Note: the INPUTS in sig_bt are in units of Java argument words, which are
// either 32-bit or 64-bit depending on the build.  The OUTPUTS are in 32-bit
// units regardless of build.


// ---------------------------------------------------------------------------
// The compiled Java calling convention.  The Java convention always passes
// 64-bit values in adjacent aligned locations (either registers or stack),
// floats in float registers and doubles in aligned float pairs.  Values are
// packed in the registers.  There is no backing varargs store for values in
// registers.  In the 32-bit build, longs are passed in G1 and G4 (cannot be
// passed in I's, because longs in I's get their heads chopped off at
// interrupt).
int SharedRuntime::java_calling_convention(const BasicType *sig_bt,
                                           VMRegPair *regs,
                                           int total_args_passed,
                                           int is_outgoing) {
  // Must map argument stack index to single or double argument register
  static const Register In_ArgReg[8] = {
    GR_I0, GR_I1, GR_I2, GR_I3, GR_I4, GR_I5, GR_I6, GR_I7 };

  static const Register Out_ArgReg[8] = {
    GR_O0, GR_O1, GR_O2, GR_O3, GR_O4, GR_O5, GR_O6, GR_O7 };

  static const FloatRegister F_ArgReg[8] = {
    FR8, FR9, FR10, FR11, FR12, FR13, FR14, FR15 };

  // Convention is to pack the first 8 int/oop args into the first 8
  // input registers (I0-GR39), extras spill to the stack.  Pack
  // the first 8 float args into FR8-FR15, extras spill to the stack.
  // same registers as they fit, else spill to the stack.
  uint stk_reg = 0;
  uint flt_reg = 0;
  uint int_reg = 0;
  int j;

  // Do the signature layout
  for( j = 0; j < total_args_passed && int_reg < 8; j++) {
    switch( sig_bt[j] ) {
    case T_BOOLEAN:
    case T_BYTE:
    case T_CHAR:
    case T_SHORT:
    case T_INT:
      regs[j].set1(is_outgoing ? Out_ArgReg[int_reg]->as_VMReg() : In_ArgReg[int_reg]->as_VMReg());
      int_reg++;
      break;
    case T_LONG:
      assert( j+1 < total_args_passed && sig_bt[j+1] == T_VOID, "expecting half" );
      // fall thru
    case T_OBJECT:
    case T_ARRAY:
    case T_ADDRESS:
      regs[j].set2(is_outgoing ? Out_ArgReg[int_reg]->as_VMReg() : In_ArgReg[int_reg]->as_VMReg());
      int_reg++;
      break;
    case T_FLOAT:
      regs[j].set1(F_ArgReg[flt_reg++]->as_VMReg());
      int_reg++;
      break;
    case T_DOUBLE:
      assert( j+1 < total_args_passed && sig_bt[j+1] == T_VOID, "expecting half" );
      regs[j].set2(F_ArgReg[flt_reg++]->as_VMReg());
      int_reg++;
      break;
    case T_VOID:  // Halves of longs and doubles
      regs[j].set_bad();
      break;
    default:
      ShouldNotReachHere();
    }
  }

  for( ; j < total_args_passed; j++) {
    switch( sig_bt[j] ) {
    case T_BOOLEAN:
    case T_BYTE:
    case T_CHAR:
    case T_SHORT:
    case T_INT:
    case T_FLOAT:
      regs[j].set1(VMRegImpl::stack2reg(stk_reg));
      stk_reg += 2;
      break;
    case T_LONG:
    case T_DOUBLE:
      assert( j+1 < total_args_passed && sig_bt[j+1] == T_VOID, "expecting half" );
      // fall thru
    case T_OBJECT:
    case T_ARRAY:
    case T_ADDRESS:
      regs[j].set2(VMRegImpl::stack2reg(stk_reg));
      stk_reg += 2;
      break;
    case T_VOID:  // Halves of longs and doubles
      regs[j].set_bad();
      break;
    default:
      ShouldNotReachHere();
    }
  }

  return round_to(stk_reg + 1, 2);
}

#define method_(field_name) GR27_method, in_bytes(methodOopDesc::field_name ## _offset())

static void gen_c2i_adapter(MacroAssembler *masm,
                            int total_args_passed,
                            int comp_args_on_stack,
                            const BasicType *sig_bt,
                            const VMRegPair *regs,
                            Label& skip_fixup) {
  // Before we get into the guts of the C2I adapter, see if we should be here
  // at all.  We've come from compiled code and are attempting to jump to the
  // interpreter, which means the caller made a static call to get here
  // (vcalls always get a compiled target if there is one).  Check for a
  // compiled target.  If there is one, we need to patch the caller's call.
  Label L;

  const Register ientry_addr     = GR14_SCRATCH;
  // This MUST MATCH code in generate_i2c2i_adapters!
  const Register ientry          = GR14_SCRATCH;
  const Register code_addr       = GR15_SCRATCH;
  const Register code            = GR15_SCRATCH;
  const PredicateRegister no_code = PR6_SCRATCH;

  const Register store_addr = GR16_SCRATCH;
  const Register load_addr  = GR17_SCRATCH;
  const Register value      = GR18_SCRATCH;
  const Register orig_PFS   = GR19_SCRATCH;


  __ add(code_addr, method_(code));
  // Schedule the branch target address early.
  __ add(ientry_addr, method_(interpreter_entry));
  __ ld8(code, code_addr);
  // Load the function descriptor (interpreted calls use function descriptors)
  __ ld8(ientry, ientry_addr);

  // Do we need to patch the call?

  __ cmp(no_code, PR0, (int) NULL_WORD, code, Assembler::equal);
  __ br(no_code, skip_fixup);

  // Call into the VM to patch the caller, then continue as interpreted
  // IF you lose the race you go interpreted we don't see
  // any possible endless c2i -> i2c ->c2i ...  transitions no
  // matter how rare.

  const Register caller_pc = GR28_SCRATCH;

  __ mov(caller_pc, RP);

  // Must do some weird juggling of PFS as we are post-call but pre-alloc and if
  // we push a dummy frame we'll lose the current PFS.

  __ mov(orig_PFS, AR_PFS);
  // protect the argument registers
  __ push_dummy_thin_frame(GR0);


  // preserve global registers we need

  const Register L12_oPFS = GR_L12;
  const Register L13_RP = GR_L13;
  const Register L14_method = GR_L14;
  const Register L15_ientry = GR_L15;

  // Must protect the return pc until we get to frame manager
  __ mov(L12_oPFS, orig_PFS);
  __ mov(L13_RP, caller_pc);
  __ mov(L14_method, GR27_method);
  __ mov(L15_ientry, ientry);

  __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::fixup_callers_callsite), GR27_method, caller_pc);

  // Restore the needed globals.

  __ mov(GR27_method, L14_method);
  __ mov(ientry, L15_ientry);
  __ mov(caller_pc, L13_RP);
  __ mov(orig_PFS, L12_oPFS);

  __ pop_dummy_thin_frame();
  __ mov(RP, caller_pc);
  __ mov(AR_PFS, orig_PFS);

  __ bind(skip_fixup);


  // Since all args are passed on the stack, total_args_passed*wordSize is the
  // space we need.  We need ABI scratch area but we use the caller's since
  // it has already been allocated.

  int extraspace = round_to(total_args_passed, 2)*wordSize;

  // Remember the senderSP so we can pop the interpreter arguments off of the stack
  __ mov(GR28_sender_SP, SP);

  // This should always fit in 14 bit immediate
  __ adds(SP, -extraspace, SP);

  // We use the caller's ABI scratch area (out_preserved_stack_slots) for the initial
  // args. This essentially moves the callers ABI scratch area from the top to the
  // bottom of the arg area

  const int abi_scratch = SharedRuntime::out_preserve_stack_slots()*VMRegImpl::stack_slot_size;

  __ add(store_addr, SP, extraspace + abi_scratch - wordSize);

  __ mov(BR7, ientry);

  // Now write the args into the outgoing interpreter space
  for (int i=0; i<total_args_passed; i++) {
    VMReg r_1 = regs[i].first();
    VMReg r_2 = regs[i].second();
    if (!r_1->is_valid()) {
      assert(!r_2->is_valid(), "");
      continue;
    }
    if (r_1->is_stack()) {

      // The calling convention produces OptoRegs that ignore the preserve area (abi scratch)
      // we must account for it here.
      int ld_off = r_1->reg2stack() * VMRegImpl::stack_slot_size + abi_scratch;
      __ add(load_addr, GR28_sender_SP, ld_off);
      if (!r_2->is_valid()) {
        __ ld4(value, load_addr);
      } else {
        __ ld8(value, load_addr);
      }
      // Pretend stack targets were loaded into value
      r_1 = value->as_VMReg();
    }

    if (r_1->is_Register()) {
      Register r = r_1->as_Register();
      if (!r_2->is_valid()) {
        __ st8(store_addr, r, -wordSize);
      } else {
        // longs are given 2 64-bit slots in the interpreter, but the
        // data is passed in only 1 slot.
        if (sig_bt[i] == T_LONG || sig_bt[i] == T_DOUBLE ) {
          __ st8(store_addr, GR0, -wordSize);
        }
        __ st8(store_addr, r, -wordSize);
      }
    } else {
      assert(r_1->is_FloatRegister(), "");
      FloatRegister f = r_1->as_FloatRegister();
      if (!r_2->is_valid()) {
        __ stfs(store_addr, f, -wordSize);
      } else {
        // In 64bit, doubles are given 2 64-bit slots in the interpreter, but the
        // data is passed in only 1 slot.
        // one of these should get known junk...
        __ stfd(store_addr, FR0, -wordSize);
        __ stfd(store_addr, f, -wordSize);
      }
    }
  }


  // Pass O5_savedSP as an argument to the interpreter.
  // The interpreter will restore SP to this value before returning.
  // __ add(SP, extraspace, O5_savedSP);
  // Jump to the interpreter just as if interpreter was doing it.


  // Since a "call" has already taken place the O's are I's
  // Itos is prepushed so it points to first word in ABI scratch

  __ mov(GR_Itos, store_addr);

  __ br(BR7);

} // END c2iadapter

static void gen_i2c_adapter(MacroAssembler *masm,
                            int total_args_passed,
                            int comp_args_on_stack,
                            const BasicType *sig_bt,
                            const VMRegPair *regs) {
  // Generate an I2C adapter: adjust the I-frame to make space for the C-frame
  // layout.  Lesp was saved by the calling I-frame and will be restored on
  // return.  Meanwhile, outgoing arg space is all owned by the callee
  // C-frame, so we can mangle it at will.  After adjusting the frame size,
  // hoist register arguments and repack other args according to the compiled
  // code convention.  Finally, end in a jump to the compiled code.  The entry
  // point address is the start of the buffer.

  // We will only enter here from an interpreted frame and never from after
  // passing thru a c2i. Azul allowed this but we do not. If we lose the
  // race and use a c2i we will remain interpreted for the race loser(s).
  // This removes all sorts of headaches on the x86 side and also eliminates
  // the possibility of having c2i -> i2c -> c2i -> ... endless transitions.

  // Inputs:
  // GR4_thread     - TLS
  // GR27_method    - Method oop
  // GR_Itos/GR_Otos- Pointer to interpreter's args
  // ??             - Caller's saved SP, to be restored if needed
  // R12            - Current SP!
  // RP             - Valid return address

  // Because ia64, unlike sparc we have already done a "call" to get here
  // The Oregisters are now Iregisters and we have NO locals or outputs.
  // However we can allocate a new window's worth of O's and L's and
  // the eventual caller will never be able to tell. This makes our life
  // much easier than on other platforms.

  // Outputs:
  // GR4_thread     - TLS
  // GR27_method    - Method oop
  // I0-I(n)        - arg(s) in integer register(s) as appropriate
  // F10-F(n)       - arg(s) in floating register(s) as appropriate
  // R12            - Adjusted SP
  // RP             - Valid return address


  // ON ENTRY TO THE CODE WE ARE MAKING, WE HAVE AN INTERPRETED FRAME
  // WITH RP HOLDING A VALID RETURN PC
  //
  // |              |
  // :  java stack  :
  // |              |
  // +--------------+ <--- start of outgoing args
  // |   receiver   |   |
  // : rest of args :   |---size is java-arg-words
  // |              |   |
  // +--------------+ <--- GR_Itos (misaligned) and Lesp if prior is not C2I
  // | ABI scratch? |
  // |              |
  // +--------------+ <--- SP

  // WE REPACK THE STACK.  We use the common calling convention layout as
  // discovered by calling SharedRuntime::calling_convention.  We assume it
  // causes an arbitrary shuffle of memory, which may require some register
  // temps to do the shuffle.  We hope for (and optimize for) the case where
  // temps are not needed.  We may have to resize the stack slightly, in case
  // we need alignment padding (32-bit interpreter can pass longs & doubles
  // misaligned, but the compilers expect them aligned).
  //
  // |              |
  // :  java stack  :
  // |              |
  // +--------------+ <--- start of outgoing args
  // |  pad, align  |   |
  // +--------------+   |
  // |              |   |---Outgoing stack args, packed low.
  // |              |   |   First few args in registers.
  // |              |   |
  // |              |   |
  // +--------------+ <--- SP' + 2*wordsize
  // |              |
  // : ABI scratch? :
  // |              |
  // +--------------+ <--- SP'

  // ON EXIT FROM THE CODE WE ARE MAKING, WE STILL HAVE AN INTERPRETED FRAME
  // WITH RP HOLDING A VALID RETURN PC - ITS JUST THAT THE ARGS ARE NOW SETUP
  // FOR COMPILED CODE AND THE FRAME SLIGHTLY GROWN.

  const Register centry_addr = GR14_SCRATCH;
  const Register centry      = GR14_SCRATCH;

  const Register value       = GR15_SCRATCH;

  const Register ld_ptr      = GR16_SCRATCH;
  const Register st_ptr      = GR17_SCRATCH;

  // Point at the initial arg (receiver) the interpreter pushed.

  __ add(ld_ptr, GR_Itos, total_args_passed*wordSize);

  // Cut-out for having no stack args.  Since up to 8 args are passed
  // in registers, we will commonly have no stack args.
  if (comp_args_on_stack) {
    // Sig words on the stack are greater-than VMRegImpl::stack0.  Those in
    // registers are below.  By subtracting stack0, we either get a negative
    // number (all values in registers) or the maximum stack slot accessed.
    // int comp_args_on_stack = VMRegImpl::reg2stack(max_arg);
    // Convert VMRegImpl (4 byte) stack slots to words.
    int comp_words_on_stack = round_to(comp_args_on_stack*VMRegImpl::stack_slot_size, wordSize)>>LogBytesPerWord;
    // Round up to miminum stack alignment, in wordSize
    comp_words_on_stack = round_to(comp_words_on_stack, 2);

    __ sub(SP, SP, comp_words_on_stack*wordSize);
  }

  // Will jump to the compiled code just as if compiled code was doing it.
  // Pre-load the register-jump target early, to schedule it better.

  __ add(centry_addr, method_(from_compiled));

  // First compiled arg is just above the ABI scratch
  __ add(st_ptr, SP, 2*wordSize);

  // compiled entries are direct and do not use function descriptor
  __ ld8(centry, centry_addr);

  // Now generate the shuffle code.  Pick up all register args and move the
  // rest through G1_SCRATCH.
  for (int i=0; i<total_args_passed; i++) {
    if (sig_bt[i] == T_VOID) {
      assert(i > 0 && (sig_bt[i-1] == T_LONG || sig_bt[i-1] == T_DOUBLE), "missing half");
      continue;
    }

    // Pick up 0, 1 or 2 words from ld_ptr
    assert(!regs[i].second()->is_valid() || regs[i].first()->next() == regs[i].second(),
            "scrambled load targets?");
    VMReg r_1 = regs[i].first();
    VMReg r_2 = regs[i].second();
    if (!r_1->is_valid()) {
      assert(!r_2->is_valid(), "");
      continue;
    }
    if (r_1->is_FloatRegister()) {
      if (!r_2->is_valid()) {
        __ ldfs(r_1->as_FloatRegister(), ld_ptr, -wordSize);
      } else {
        // Skip the unused interpreter slot
        __ add(ld_ptr, ld_ptr, -wordSize);
        __ ldfd(r_1->as_FloatRegister(), ld_ptr, -wordSize);
      }
    } else {
      Register r;
      if (r_1->is_stack()) {
        // Must do a memory to memory move thru "value"
        r = value;
        // The calling convention produces OptoRegs that ignore the preserve area
        // we must account for it here.
        int st_off = (r_1->reg2stack() + SharedRuntime::out_preserve_stack_slots())*VMRegImpl::stack_slot_size;
        __ add(st_ptr, SP, st_off);
      } else {
        r = r_1->as_Register();
      }
      if (!r_2->is_valid()) {
        // Not sure we need to do this but it shouldn't hurt
        if (sig_bt[i] == T_OBJECT || sig_bt[i] == T_ADDRESS || sig_bt[i] == T_ARRAY) {
          __ ld8(r, ld_ptr, -wordSize);
        } else {
          __ ld4(r, ld_ptr, -wordSize);
        }
      } else {
        // In 64bit, longs are given 2 64-bit slots in the interpreter, but the
        // data is passed in only 1 slot.
        if (sig_bt[i] == T_LONG || sig_bt[i] == T_DOUBLE) {
          __ add(ld_ptr, ld_ptr, -wordSize);
        }
        __ ld8(r, ld_ptr, -wordSize);
      }

      if (r_1->is_stack()) {
        // Now store  value where the compiler expects it
        __ st8(st_ptr, r);
      }
    }
  }
  // 6243940 We might end up in handle_wrong_method if
  // the callee is deoptimized as we race thru here. If that
  // happens we don't want to take a safepoint because the
  // caller frame will look interpreted and arguments are now
  // "compiled" so it is much better to make this transition
  // invisible to the stack walking code. Unfortunately if
  // we try and find the callee by normal means a safepoint
  // is possible. So we stash the desired callee in the thread
  // and the vm will find there should this case occur.

  const Register callee_target_addr       = GR15_SCRATCH;
  __ add(callee_target_addr, thread_(callee_target));
  __ st8(callee_target_addr, GR27_method);

  // Jump to the compiled code just as if compiled code was doing it.
  __ mov(BR7, centry);
  __ br(BR7);

} // END i2c_adapter

// ---------------------------------------------------------------
AdapterHandlerEntry* SharedRuntime::generate_i2c2i_adapters(MacroAssembler *masm,
                                                            int total_args_passed,
                                                            int comp_args_on_stack,
                                                            const BasicType *sig_bt,
                                                            const VMRegPair *regs) {
  //
  // This entry can be called by the interpreter or call stub which expects
  // function descriptors.
  //
  // address i2c_entry = __ emit_fd();
  address i2c_entry = __ pc();
  gen_i2c_adapter(masm, total_args_passed, comp_args_on_stack, sig_bt, regs);


  // -------------------------------------------------------------------------
  // Generate a C2I adapter.  On entry we know GR27 holds the methodOop.  The
  // args start out packed in the compiled layout.  They need to be unpacked
  // into the interpreter layout.  This will almost always require some stack
  // space.  We grow the current (compiled) stack, then repack the args.  We
  // finally end in a jump to the generic interpreter entry point.  On exit
  // from the interpreter, the interpreter will restore our SP.

  // calls from compiled code do not expect function descriptors
  //
   __ flush_bundle();
  address c2i_unverified_entry = __ pc();
  // address c2i_unverified_entry = __ emit_fd();
  Label skip_fixup;
  {
    const Register klass_addr = GR14_SCRATCH;
    const Register klass      = GR14_SCRATCH;
    const Register holder_klass_addr = GR15_SCRATCH;
    const Register holder_klass      = GR15_SCRATCH;
    const Register holder_method_addr = GR16_SCRATCH;


    // __ verify_oop(I0);
    // __ verify_oop(GR27_method);
    __ add(klass_addr, GR_I0, oopDesc::klass_offset_in_bytes());
    __ add(holder_klass_addr, GR27_method, compiledICHolderOopDesc::holder_klass_offset());

    __ ld8(klass_addr, klass_addr);
    __ ld8(holder_klass, holder_klass_addr);
    // __ verify_oop(klass_addr);

    const PredicateRegister PR_ok = PR6_SCRATCH;
    Label ok;

    __ cmp(PR_ok, PR0, klass_addr, holder_klass, Assembler::equal);
    __ add(holder_method_addr, GR27_method, compiledICHolderOopDesc::holder_method_offset());
    __ br(PR_ok, ok);

    __ mova(GR3, SharedRuntime::get_ic_miss_stub(), relocInfo::runtime_call_type );
    __ movi(BR6_SCRATCH, GR3 );
    __ br(BR6_SCRATCH, Assembler::spnt );

    __ bind(ok);

    const Register ientry_addr     = GR14_SCRATCH;
    // This MUST MATCH code in gen_c2i_adapter!
    const Register ientry          = GR14_SCRATCH;

    const Register code_addr     = GR14_SCRATCH;
    // This MUST MATCH code in gen_c2i_adapter!
    const Register code          = GR14_SCRATCH;

    __ ld8(GR27_method, holder_method_addr);

    __ add(code_addr, method_(code));
    __ ld8(code, code_addr);
    // Do we need to patch the call?

    const PredicateRegister no_code = PR6_SCRATCH;

    __ cmp(no_code, PR0, (int) NULL_WORD, code, Assembler::equal);
    __ add(no_code, ientry_addr, method_(interpreter_entry));
    __ ld8(no_code, ientry, ientry_addr);
    __ br(no_code, skip_fixup);

    __ mova(GR3, SharedRuntime::get_ic_miss_stub(), relocInfo::runtime_call_type );
    __ movi(BR6_SCRATCH, GR3 );
    __ br(BR6_SCRATCH, Assembler::spnt );

  }

  // calls from compiled code do not expect function descriptors
   __ flush_bundle();
  address c2i_entry = __ pc();

  gen_c2i_adapter(masm, total_args_passed, comp_args_on_stack, sig_bt, regs, skip_fixup);


  __ flush();
  return new AdapterHandlerEntry(i2c_entry, c2i_entry, c2i_unverified_entry);

}

int SharedRuntime::c_calling_convention(const BasicType *sig_bt,
                                         VMRegPair *regs,
                                         int total_args_passed) {
  // We return the amount of VMRegImpl stack slots we need to reserve for all
  // the arguments NOT counting out_preserve_stack_slots.

  // Must map argument stack index to single or double argument register
  static const Register Out_ArgReg[8] = {
    GR_O0, GR_O1, GR_O2, GR_O3, GR_O4, GR_O5, GR_O6, GR_O7 };

  static const FloatRegister F_ArgReg[8] = {
    FR8, FR9, FR10, FR11, FR12, FR13, FR14, FR15 };

  // Convention is to pack the first 8 int/oop args into the first 8
  // input registers (I0-GR39), extras spill to the stack.  Pack
  // the first 8 float args into FR8-FR15, extras spill to the stack.
  // same registers as they fit, else spill to the stack.
  uint stk_reg = 0;
  uint flt_reg = 0;
  uint int_reg = 0;
  int j;

  // Do the signature layout
  for( j = 0; j < total_args_passed && int_reg < 8; j++) {
    switch( sig_bt[j] ) {
    case T_BOOLEAN:
    case T_CHAR:
    case T_BYTE:
    case T_SHORT:
    case T_INT:
      regs[j].set1(Out_ArgReg[int_reg++]->as_VMReg());
      break;
    case T_LONG:
      assert( j+1 < total_args_passed && sig_bt[j+1] == T_VOID, "expecting half" );
      // fall thru
    case T_OBJECT:
    case T_ADDRESS:
    case T_ARRAY:
      regs[j].set2(Out_ArgReg[int_reg++]->as_VMReg());
      break;
    case T_FLOAT:
      regs[j].set1(F_ArgReg[flt_reg++]->as_VMReg());
      int_reg++;
      break;
    case T_DOUBLE:
      assert( j+1 < total_args_passed && sig_bt[j+1] == T_VOID, "expecting half" );
      regs[j].set2(F_ArgReg[flt_reg++]->as_VMReg());
      int_reg++;
      break;
    case T_VOID:  // Halves of longs and doubles
      regs[j].set_bad();
      break;
    default:
      ShouldNotReachHere();
    }
  }

  for( ; j < total_args_passed; j++) {
    switch( sig_bt[j] ) {
    case T_BOOLEAN:
    case T_CHAR:
    case T_BYTE:
    case T_SHORT:
    case T_INT:
    case T_FLOAT:
      regs[j].set1(VMRegImpl::stack2reg(stk_reg));
      stk_reg += 2;
      break;
    case T_DOUBLE:
    case T_LONG:
      assert( j+1 < total_args_passed && sig_bt[j+1] == T_VOID, "expecting half" );
    case T_ARRAY:
    case T_ADDRESS:
    case T_OBJECT:
      regs[j].set2(VMRegImpl::stack2reg(stk_reg));
      stk_reg += 2;
      break;
    case T_VOID:  // Halves of longs and doubles
      regs[j].set_bad();
      break;
    default:
      ShouldNotReachHere();
    }
  }
  return stk_reg;
}


// ---------------------------------------------------------------------------
void SharedRuntime::save_native_result(MacroAssembler *masm, BasicType ret_type, int frame_slots) {

  // Unused until we generate signature based native wrappers
  ShouldNotReachHere();

#if 0
  switch (ret_type) {
  }
#endif
}

void SharedRuntime::restore_native_result(MacroAssembler *masm, BasicType ret_type, int frame_slots) {

  // Unused until we generate signature based native wrappers
  ShouldNotReachHere();
}

// ---------------------------------------------------------------------------
// Generate a native wrapper for a given method.  The method takes arguments
// in the Java compiled code convention, marshals them to the native
// convention (handlizes oops, etc), transitions to native, makes the call,
// returns to java state (possibly blocking), unhandlizes any result and
// returns.
nmethod *SharedRuntime::generate_native_wrapper(MacroAssembler* masm,
                                                methodHandle method,
                                                int total_args_passed,
                                                int comp_args_on_stack,
                                                BasicType *sig_bt,
                                                VMRegPair *regs,
                                                BasicType ret_type) {


  ShouldNotReachHere();

  __ flush();

  // nmethod *nm = nmethod::new_native_nmethod(method, masm->oop_recorder(), masm->code(), frame_slots, oop_maps);
  nmethod *nm = NULL;
  return nm;
}

// this function returns the adjust size (in number of words) to a c2i adapter
// activation for use during deoptimization
int Deoptimization::last_frame_adjust(int callee_parameters, int callee_locals) {
  assert( callee_locals >= callee_parameters,
          "test and remove; got more parms than locals" );
  // Keep the stack aligned on 2 word boundaries
  return round_to(callee_locals - callee_parameters, 2);
}

// "Top of Stack" slots that may be unused by the calling convention but must
// otherwise be preserved.
// On Intel these are not necessary and the value can be zero.
// On Sparc this describes the words reserved for storing a register window
// when an interrupt occurs.
// On ia64 this is the ABI scratch area (2 words)
uint SharedRuntime::out_preserve_stack_slots() {

  return 2 * VMRegImpl::slots_per_word;
}

//------------------------------generate_deopt_blob----------------------------
// Ought to generate an ideal graph & compile, but here's some IA64 ASM
// instead.
void SharedRuntime::generate_deopt_blob() {
  // allocate space for the code
  ResourceMark rm;
  // setup code generation tools
  CodeBuffer buffer("deopt_blob", 4096, 512);
  MacroAssembler* masm               = new MacroAssembler(&buffer);
  Register        GR10_exception_tmp     = GR10;
  Label           cont, loop;

  OopMapSet *oop_maps = new OopMapSet();

  //
  // This is the entry point for code which is returning to a de-optimized
  // frame.
  // The steps taken by this frame are as follows:
  //   - push a dummy "unpack_frame" and save the return values (O0, O1, F0)
  //     which are coming in
  //   - call the C routine: Deoptimization::fetch_unroll_info (this function
  //     returns information about the number and size of interpreter frames
  //     which are equivalent to the frame which is being deoptimized)
  //   - deallocate the unpack frame
  //   - deallocate the deoptimization frame
  //   - in a loop using the information returned in the previous step
  //     push new interpreter frames (take care to propagate the return
  //     values through each new frame pushed)
  //   - create a dummy "unpack_frame" and save the return values (O0, O1, F0)
  //   - call the C routine: Deoptimization::unpack_frames (this function
  //     lays out values on the interpreter frame which was just created)
  //   - deallocate the dummy unpack_frame
  //   - ensure that all the return values are correctly set and then do
  //     a return to the interpreter entry point
  //
  // Refer to the following methods for more information:
  //   - Deoptimization::fetch_unroll_info
  //   - Deoptimization::unpack_frames

  address start = __ pc();

  // space to save live return results and abi scratch area
  int framesize = (round_to(sizeof(double)*2, 16) + round_to(sizeof(jlong)*2, 16))/wordSize + 2;

  // We have been "called" by the deoptimized nmethod. RP is wrong by one
  // bundle from what the original RP before we return to the deoptimized
  // method so we correct it and put it in a useful location.

  __ mov(GR9_issuing_pc, RP);
  __ add(GR9_issuing_pc, -BytesPerInstWord, GR9_issuing_pc);
  __ mov(GR10_exception_tmp, Deoptimization::Unpack_deopt);

  // because we have been "called" we are not in the deoptee's register window
  // we must go back to that window. Do a dummy return to remove the zero
  // length window the call created.

  __ return_from_dummy_frame();
  __ br(cont);
  __ flush_bundle();

  int exception_offset = __ pc() - start;

  // We reached here via a branch we are in the deoptee register window

  // GR9 has the original throwing PC
  __ mov(GR10_exception_tmp, Deoptimization::Unpack_exception);

  __ bind(cont);

  //
  // frames: deoptee, caller_of_deoptee

  // push a dummy "unpack_frame" taking care to preserve float return values
  // and call Deoptimization::fetch_unroll_info to get the UnrollBlock

  // GR9_issuing_pc contains the return point to the deoptimized nmethod
  // that existed when the nmethod was deoptimized. We push that frame here
  // and the stack walking will think we are just about to return from that
  // call and do the correct thing.

  __ push_dummy_full_frame(GR9_issuing_pc);

  // frames: unpack frame, deoptee, possible_i2c, caller_of_deoptee
  // unpack frame has pushed caller's unat bits so spills here won't
  // cause any surprises

  __ mov(GR_Lstate, GR10_exception_tmp);                    // Save this where it is safe
  __ sub(SP, SP, round_to(sizeof(double)*2, 16));
  __ stfspill(SP, FR_RET);
  __ sub(SP, SP, round_to(sizeof(jlong)*2, 16));
  __ st8spill(SP, GR_RET);
  __ sub(SP, SP, 2*wordSize);                           // ABI scratch

  // What about things like LC?  Saved/restored by push/pop_dummy_full_frame

  // Local register assignment

  const Register GR15_frame_sizes  = GR15;   // first the address, then the value
  const Register GR16_frame_count  = GR16;   // first the address, then the value
  const Register GR17_caller_adj   = GR17;   // first the address, then the value
  const Register GR18_frame_pcs    = GR18;   // first the address, then the value

  // We need to set last_Java_frame because fetch_unroll_info will call last_Java_frame()
  // However we can't block and no GC will occur so we don't need an oopmap and the
  // value of the pc in the frame is not particularly important (just identify the blob)

  int offset;
  offset = __ set_last_Java_frame(SP);

  __ call_VM_leaf(CAST_FROM_FN_PTR(address, Deoptimization::fetch_unroll_info), GR4_thread);

  // Add an oopmap that describes the above call
  // At this point on ia64 there are no callee save register that need to be
  // described. If c2 ever has live callee save registers we would need to save them
  // before calling fetch_unroll_info and describe them in the oopmap.
  // At the moment we have none so an empty oopmap is plenty.

  oop_maps->add_gc_map(offset, new OopMap( framesize, 0));

  __ reset_last_Java_frame();

  __ add(GR18_frame_pcs,    GR_RET /* info */, Deoptimization::UnrollBlock::frame_pcs_offset_in_bytes());
  __ add(GR15_frame_sizes,  GR_RET /* info */, Deoptimization::UnrollBlock::frame_sizes_offset_in_bytes());
  __ add(GR16_frame_count,  GR_RET /* info */, Deoptimization::UnrollBlock::number_of_frames_offset_in_bytes());
  __ add(GR17_caller_adj,   GR_RET /* info */, Deoptimization::UnrollBlock::caller_adjustment_offset_in_bytes());

  __ mov(GR10_exception_tmp, GR_L8);                    // and restore to where we keep it
  __ add(SP, SP, 2*wordSize);                           // ABI area
  __ ld8fill(GR_RET, SP);
  __ add(SP, SP, round_to(sizeof(jlong)*2, 16));
  __ ldffill(FR_RET, SP);
  __ add(SP, SP, round_to(sizeof(double)*2, 16));

  __ pop_dummy_full_frame();

  // frames: deoptee, caller_of_deoptee

  __ ld8(GR15_frame_sizes,  GR15_frame_sizes);
  __ ld8(GR18_frame_pcs,    GR18_frame_pcs);
  __ ld4(GR16_frame_count,  GR16_frame_count);
  __ ld4(GR17_caller_adj,   GR17_caller_adj);

  // Now we must pop the frame we are deoptimizing

  __ pop_dummy_thin_frame();  // Not really a dummy, but junk now.

  // frames: caller_of_deoptee

  // We now have in LC and UNAT the values that we there on entry to the deoptimized frame
  // We will propagate them to all the frames we create only the original caller of
  // the deoptimized frame will care (and if that caller is an i2c we will pop it and
  // used its LC and UNAT instead).
  // Seems like things like r4-r7 are ok because Java never touches them so they should
  // already be saved/restored by non-Java code and their values here should be correct
  // already.

  // loop through the UnrollBlock info and create interpreter frames
  // Everything must be done in global registers because the windows
  // will be changing constantly

  #ifdef ASSERT
  // make sure that there is at least one entry in the array
  __ cmp4(PR6, PR0, 0, GR16_frame_count, Assembler::equal);
  __ breaki(PR6, 1);
  #endif

  // We could use LC but then we'd have to shuffle it to save the preserved value.
  // Doesn't seem to be worth the bother

  const Register GR19_pc  = GR19;

  __ bind(loop);

  __ ld8(GR2, GR15_frame_sizes, sizeof(intptr_t));      // Get size of next memory frame to allocate
  __ ld8(GR19_pc, GR18_frame_pcs, sizeof(address));     // Get size of next return address to store
  __ sub(GR16_frame_count, GR16_frame_count, 1);

  __ push_dummy_full_frame(GR19_pc);                     // Allocate register stack side of the frame

  #ifdef ASSERT
    // trash local registers to show a clear pattern in backtraces
    // __ mov(GR_L0, 0xDEAD00); GR_Lsave_SP
    // __ mov(GR_L1, 0xDEAD01); GR_Lsave_caller_BSP
    // __ mov(GR_L2, 0xDEAD02); GR_Lsave_PFS
    // __ mov(GR_L3, 0xDEAD03); GR_Lsave_RP
    // __ mov(GR_L4, 0xDEAD04); GR_Lsave_LC
    // __ mov(GR_L5, 0xDEAD05); GR_Lsave_GP
    // __ mov(GR_L6, 0xDEAD06); GR_Lsave_UNAT
    // __ mov(GR_L7, 0xDEAD07); GR_Lsave_PR
    __ mov(GR_L8, 0xDEAD08);  //GR_Lstate
    __ mov(GR_L9, 0xDEAD09);  //GR_Lscratch0
    __ mov(GR_L10, 0xDEAD10); //GR_Lmethod_addr
    __ mov(GR_L11, 0xDEAD11);
    __ mov(GR_L12, 0xDEAD12);
    __ mov(GR_L13, 0xDEAD13);
    __ mov(GR_L14, 0xDEAD14);
    __ mov(GR_L15, 0xDEAD15);

  #endif
  __ sub(SP, SP, GR2);                                  // Allocate memory stack side of the frame
  // The first frame we push gets adjusted by an amount that
  // can hold the parameters the initial frame got from it's
  // caller.
  __ sub(SP, SP, GR17_caller_adj);
  // No more frames will need the "last_frame_adjustment"
  __ mov(GR17_caller_adj, GR0);

  __ cmp4(PR0, PR6, 0, GR16_frame_count, Assembler::equal);
  __ br(PR6, loop, Assembler::dptk);

  // frames: new skeletal interpreter frame(s), caller_of_deoptee

  // push a dummy "unpack_frame" taking care of float/int return values and
  // call Deoptimization::unpack_frames to have the unpacker layout
  // information in the interpreter frames just created and then return
  // to the interpreter entry point

  __ ld8(GR19_pc, GR18_frame_pcs, sizeof(address));     // Get return address we want to store in the new frame

  __ push_dummy_full_frame(GR19_pc);                     // this will be a return to the frame manager (somewhere)

  // frames: unpack_frame, new skeletal interpreter frame(s), possible_c2_i,  caller_of_deoptee

  // Flush the windows to the stack. For Itanium2 we will have to also turn off eager filling
  // so that the windows don't get filled while we're in the middle of changing them!

  // We must be sure that the current windows flush so flushrs inline
  // will not be sufficient. The fact that the current window refills
  // is not important since we just want to be certain that the
  // stack is walkable and it will be.

  __ sub(SP, SP, round_to(sizeof(double)*2, 16));
  __ stfspill(SP, FR_RET);
  __ sub(SP, SP, round_to(sizeof(jlong)*2, 16));
  __ st8spill(SP, GR_RET);
  __ sub(SP, SP, 2*wordSize);                           // ABI

  __ set_last_Java_frame(SP);
  __ call_VM_leaf((address)StubRoutines::ia64::flush_register_stack());

  __ call_VM_leaf(CAST_FROM_FN_PTR(address, Deoptimization::unpack_frames), GR4_thread, GR10_exception_tmp);
  __ reset_last_Java_frame();

  __ add(SP, SP, 2*wordSize);                           // ABI
  __ ld8fill(GR_RET, SP);
  __ add(SP, SP, round_to(sizeof(jlong)*2, 16));
  __ ldffill(FR_RET, SP);
  __ add(SP, SP, round_to(sizeof(double)*2, 16));

  // We should be in the newly constructed top frame of the interpreter (frame_manager)

  __ pop_full_frame();

  // frames: new skeletal interpreter frame(s), possible_c2_i,  caller_of_deoptee
  __ ret();

  __ flush();
  _deopt_blob = DeoptimizationBlob::create(&buffer, oop_maps, 0, exception_offset, 0, framesize);
}


//------------------------------generate_uncommon_trap_blob--------------------
// Ought to generate an ideal graph & compile, but here's some IA64 ASM
// instead.
static UncommonTrapBlob* generate_uncommon_trap_blob() {
  // allocate space for the code
  ResourceMark rm;
  // setup code generation tools
  CodeBuffer buffer("uncommon_trap_blob", 2048, 512);
  MacroAssembler* masm                = new MacroAssembler(&buffer);
  Label           cont, loop;

  const Register GR15_klass_index     = GR15;
  //
  // This is the entry point for all traps the compiler takes when it thinks
  // it cannot handle further execution of compilation code. The frame is
  // deoptimized in these cases and converted into interpreter frames for
  // execution
  // The steps taken by this frame are as follows:
  //   - push a fake "unpack_frame"
  //   - call the C routine Deoptimization::uncommon_trap (this function
  //     packs the current compiled frame into vframe arrays and returns
  //     information about the number and size of interpreter frames which
  //     are equivalent to the frame which is being deoptimized)
  //   - deallocate the "unpack_frame"
  //   - deallocate the deoptimization frame
  //   - in a loop using the information returned in the previous step
  //     push interpreter frames;
  //   - create a dummy "unpack_frame"
  //   - call the C routine: Deoptimization::unpack_frames (this function
  //     lays out values on the interpreter frame which was just created)
  //   - deallocate the dummy unpack_frame
  //   - return to the interpreter entry point
  //
  //  Refer to the following methods for more information:
  //   - Deoptimization::uncommon_trap
  //   - Deoptimization::unpack_frame

  enum frame_layout {
    F8_off,
    F8_nat_off,
    framesize
  };

  // the unloaded class index is in O0 (first parameter to this blob)

  // push a dummy "unpack_frame"
  // and call Deoptimization::uncommon_trap to pack the compiled frame into
  // vframe array and return the UnrollBlock information

  // push a dummy "unpack_frame" taking care to preserve float return values
  // and call Deoptimization::fetch_unroll_info to get the UnrollBlock

  // Weird on sparc this frame isn't really a dummy at all


  const Register dummy  = GR3_SCRATCH;

  // This is lame but true. We need to allocate space on the stack so that deoptimization
  // will walk passed this frame and find the stack space above this one because
  // the code for deopt does a while loop looking for an sp that matches and then expects
  // the pc to be for the frame we want deoptimized. Since on ia64 we can have a frame
  // that uses no stack space (but register stack space) the code fails. Simplest thing
  // is to just allocate some dummy space and go with the flow.

  __ mov(dummy, RP);            // Make the return address be the sender
  __ save_full_frame(framesize * sizeof(intptr_t));

  // Spill the live registers

  __ stfspill(SP, FR8);

  // Set an oopmap for the call site
  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map =  new OopMap( framesize, 0 );

  int offset;
  offset = __ set_last_Java_frame(SP);

  __ mov(GR15_klass_index, GR_I0);
  __ call_VM_leaf(CAST_FROM_FN_PTR(address, Deoptimization::uncommon_trap), GR4_thread, GR15_klass_index);
  __ reset_last_Java_frame();

  // Add an oopmap that describes the above call

  map->set_callee_saved( VMRegImpl::stack2reg(F8_off   ), FR8->as_VMReg() );
  oop_maps->add_gc_map(offset, map);

  // Local register assignment

  const Register GR15_frame_sizes   = GR15;   // first the address, then the value
  const Register GR16_frame_count   = GR16;   // first the address, then the value
  const Register GR17_caller_adj   =  GR17;   // first the address, then the value
  const Register GR18_frame_pcs     = GR18;   // first the address, then the value

  __ add(GR15_frame_sizes,  GR_RET /* info */, Deoptimization::UnrollBlock::frame_sizes_offset_in_bytes());
  __ add(GR18_frame_pcs,    GR_RET /* info */, Deoptimization::UnrollBlock::frame_pcs_offset_in_bytes());
  __ add(GR17_caller_adj,   GR_RET /* info */, Deoptimization::UnrollBlock::caller_adjustment_offset_in_bytes());
  __ add(GR16_frame_count,  GR_RET /* info */, Deoptimization::UnrollBlock::number_of_frames_offset_in_bytes());
  __ pop_dummy_full_frame();
  // our frame is gone now.

  __ ld8(GR15_frame_sizes, GR15_frame_sizes);
  __ ld8(GR18_frame_pcs, GR18_frame_pcs);
  __ ld4(GR16_frame_count, GR16_frame_count);
  __ ld4(GR17_caller_adj,   GR17_caller_adj);

  // %%%%%%% may have to setup some oop maps for the call site if G/O
  //         registers are used in code

  // Now we must pop the frame we are deoptimizing

  __ pop_dummy_thin_frame();  // Not really a dummy, but junk now.

  // We now have in LC and UNAT the values that we there on entry to the deoptimized frame
  // We will propagate them to all the frames we create only the original caller of
  // the deoptimized frame will care.
  // Seems like things like r4-r7 are ok because Java never touches them so they should
  // already be saved/restored by non-Java code and their values here should be correct
  // already.

  // We are now in the frame that will become an interpreter frame
  // the RP is either the call_stub or the frame manager

  // loop through the UnrollBlock info and create interpreter frames
  // Everything must be done in global registers because the windows
  // will be changing constantly

  #ifdef ASSERT
  // make sure that there is at least one entry in the array
  __ cmp4(PR6, PR0, 0, GR16_frame_count, Assembler::equal);
  __ breaki(PR6, 1);
  #endif

  // We could use LC but then we'd have to shuffle it to save the preserved value.
  // Doesn't seem to be worth the bother

  const Register GR19_pc  = GR19_SCRATCH;

  __ bind(loop);

  __ ld8(GR8, GR15_frame_sizes, sizeof(intptr_t));     // Get size of next memory frame to allocate
  __ ld8(GR19_pc, GR18_frame_pcs, sizeof(address));    // Get return address we want to store in the new frame
  __ sub(GR16_frame_count, GR16_frame_count, 1);
  __ push_dummy_full_frame(GR19_pc);            // Allocate register stack side of the frame

  #ifdef ASSERT
  // trash local registers to show a clear pattern in backtraces
  // __ mov(GR_L0, 0xDEAD00); GR_Lsave_SP
  // __ mov(GR_L1, 0xDEAD01); GR_Lsave_caller_BSP
  // __ mov(GR_L2, 0xDEAD02); GR_Lsave_PFS
  // __ mov(GR_L3, 0xDEAD03); GR_Lsave_RP
  // __ mov(GR_L4, 0xDEAD04); GR_Lsave_LC
  // __ mov(GR_L5, 0xDEAD05); GR_Lsave_GP
  // __ mov(GR_L6, 0xDEAD06); GR_Lsave_UNAT
  // __ mov(GR_L7, 0xDEAD07); GR_Lsave_PR
  __ mov(GR_L8, 0xDEAD08);  //GR_Lstate
  __ mov(GR_L9, 0xDEAD09);  //GR_Lscratch0
  __ mov(GR_L10, 0xDEAD10); //GR_Lmethod_addr
  __ mov(GR_L11, 0xDEAD11);
  __ mov(GR_L12, 0xDEAD12);
  __ mov(GR_L13, 0xDEAD13);
  __ mov(GR_L14, 0xDEAD14);
  __ mov(GR_L15, 0xDEAD15);
  #endif

  __ sub(SP, SP, GR8);                                 // Allocate memory stack side of the frame
  // The first frame we push gets adjusted by an amount that
  // can hold the parameters the initial frame got from it's
  // caller.
  __ sub(SP, SP, GR17_caller_adj);
  // No more frames will need the "last_frame_adjustment"
  __ mov(GR17_caller_adj, GR0);

  __ cmp4(PR0, PR6, 0, GR16_frame_count, Assembler::equal);
  __ br(PR6, loop, Assembler::dptk);

  // push a dummy "unpack_frame" taking care of float/int return values and
  // call Deoptimization::unpack_frames to have the unpacker layout
  // information in the interpreter frames just created and then return
  // to the interpreter entry point

  // Flush the windows to the stack. For Itanium2 we will have to also turn off eager filling
  // so that the windows don't get filled while we're in the middle of changing them!

  __ ld8(GR19_pc, GR18_frame_pcs, sizeof(address));    // Get return address we want to store in the new frame

  __ push_dummy_full_frame(GR19_pc);                    // Will return to uncommon trap entry point in frame manager
  __ sub(SP, SP, framesize*wordSize);                  // Allocate the proper frame size for this blob
                                                       // so stack walking will work properly.

  // We must be sure that the current windows flush so flushrs inline
  // will not be sufficient. The fact that the current window refills
  // is not important since we just want to be certain that the
  // stack is walkable and it will be.
  __ call_VM_leaf((address)StubRoutines::ia64::flush_register_stack());

  __ set_last_Java_frame(SP);
  __ mov(GR2, Deoptimization::Unpack_uncommon_trap);
  __ call_VM_leaf(CAST_FROM_FN_PTR(address, Deoptimization::unpack_frames), GR4_thread, GR2);
  __ reset_last_Java_frame();

  // We should be in the newly constructed top frame of the interpreter (frame_manager)

  __ pop_full_frame();
  __ ret();

  __ flush();
  return UncommonTrapBlob::create(&buffer, oop_maps, framesize);

}

//------------------------------generate_handler_blob-------------------
//
// Generate a special Compile2Runtime blob that saves all registers, and sets
// up an OopMap.

//
// This blob is jumped to (via a breakpoint and the signal handler) from a
// safepoint in compiled code.  On entry to this blob, RP contains the
// address in the original nmethod at which we should resume normal execution.
// Thus, this blob looks like a subroutine which must preserve lots of
// registers and return normally.  Note that RP is never register-allocated,
// so it is guaranteed to be free here.
//

static SafepointBlob* generate_handler_blob(address call_ptr, bool cause_return) {
  assert (StubRoutines::forward_exception_entry() != NULL, "must be generated before");

  const Register pending_exception             = GR2_SCRATCH;
  const Register pending_exception_addr        = pending_exception;
  const Register continuation                  = pending_exception;
  const PredicateRegister is_pending_exception = PR6_SCRATCH;
  const PredicateRegister is_null              = is_pending_exception;
  const BranchRegister continuation_br         = BR6_SCRATCH;

  // allocate space for the code
  ResourceMark rm;
  CodeBuffer buffer("handler_blob", 3000, 512);
  MacroAssembler* masm                = new MacroAssembler(&buffer);
  int             frame_size_words;
  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map = NULL;


  // If this causes a return before the processing, pop the frame off
  if (cause_return) {
    __ pop_dummy_thin_frame();
  }

  map = RegisterSaver::save_live_registers(masm, &frame_size_words);

  // setup last_Java_sp and last_Java_pc
  int offset;
  offset = __ set_last_Java_frame(SP);

  // call into the runtime to handle the safepoint poll
  __ call_VM_leaf(call_ptr, GR4_thread);

  // Set an oopmap for the call site.
  // We need this not only for callee-saved registers, but also for volatile
  // registers that the compiler might be keeping live across a safepoint.

  oop_maps->add_gc_map(offset, map);

  // clear last_Java_sp
  __ reset_last_Java_frame();

  // If the VM result is non-zero then it is the destination to continue at.
  // This happens when the original nmethod was deoptimized and we safepointed
  // at a call (i.e. non polling safepoint ) since the original call instruction
  // could have been overwritten the vm provides us with the destination
  // address. However if there is an exception pending it takes precedence
  // and we don't execute call.

  // Check for exceptions
  Label pending;

  __ add(pending_exception_addr, thread_(pending_exception));
  __ ld8(pending_exception, pending_exception_addr);
  __ cmp(PR0, is_pending_exception, 0, pending_exception, Assembler::equal);
  __ br(is_pending_exception, pending);

  Label skip;

  RegisterSaver::restore_live_registers(masm);

  // We are back the the original state on entry and ready to go.

  __ br(RP);

  // Pending exception after the safepoint

  __ bind(pending);

  RegisterSaver::restore_live_registers(masm);

  // We are back the the original state on entry.

  // Tail-call forward_exception_entry, with the issuing PC in GR_Lsave_RP
  // so it looks like the original nmethod called forward_exception_entry.
  __ mova(continuation, StubRoutines::forward_exception_entry());
  __ mov(continuation_br, continuation);
  __ br(continuation_br);

  // -------------
  // make sure all code is generated
  __ flush();

  // return exception blob
  return SafepointBlob::create(&buffer, oop_maps, frame_size_words);
}

//
// generate_resolve_blob - call resolution (static/virtual/opt-virtual/ic-miss
//
// Generate a stub that calls into vm to find out the proper destination
// of a java call. All the argument registers are live at this point
// but since this is generic code we don't know what they are and the caller
// must do any gc of the args.
//
static RuntimeStub* generate_resolve_blob(address destination, const char*name) {
  assert (StubRoutines::forward_exception_entry() != NULL, "must be generated before");

  const Register pending_exception             = GR2_SCRATCH;
  const Register pending_exception_addr        = pending_exception;
  const Register continuation                  = pending_exception;
  const PredicateRegister is_pending_exception = PR6_SCRATCH;
  const PredicateRegister is_null              = is_pending_exception;
  const BranchRegister continuation_br         = BR6_SCRATCH;

  // allocate space for the code
  ResourceMark rm;
  CodeBuffer buffer(name, 3000, 512);
  MacroAssembler* masm                = new MacroAssembler(&buffer);
  int             frame_size_words;
  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map = NULL;


  map = RegisterSaver::save_live_registers(masm, &frame_size_words);

  int frame_complete = __ offset();

  // setup last_Java_sp and last_Java_pc
  int offset;
  offset = __ set_last_Java_frame(SP);

  // call the resolve site.
  __ call_VM_leaf(destination, GR4_thread);

  __ stop("death");

  // Set an oopmap for the call site.
  // We need this not only for callee-saved registers, but also for volatile
  // registers that the compiler might be keeping live across a safepoint.

  oop_maps->add_gc_map(offset, map);

  // clear last_Java_sp
  __ reset_last_Java_frame();

  // If the VM result is non-zero then it is the destination to continue at.
  // This happens when the original nmethod was deoptimized and we safepointed
  // at a call (i.e. non polling safepoint ) since the original call instruction
  // could have been overwritten the vm provides us with the destination
  // address. However if there is an exception pending it takes precedence
  // and we don't execute call.

  // Check for exceptions
  Label pending;

  __ add(pending_exception_addr, thread_(pending_exception));
  __ ld8(pending_exception, pending_exception_addr);
  __ cmp(PR0, is_pending_exception, 0, pending_exception, Assembler::equal);
  __ br(is_pending_exception, pending);

  Label skip;

  RegisterSaver::restore_live_registers(masm);

  // We are back the the original state on entry and ready to go.

  __ br(RP);

  // Pending exception after the safepoint

  __ bind(pending);

  RegisterSaver::restore_live_registers(masm);

  // We are back the the original state on entry.

  // Tail-call forward_exception_entry, with the issuing PC in GR_Lsave_RP
  // so it looks like the original nmethod called forward_exception_entry.
  __ mova(continuation, StubRoutines::forward_exception_entry());
  __ mov(continuation_br, continuation);
  __ br(continuation_br);

  // -------------
  // make sure all code is generated
  __ flush();

  // return resolve stub
   return RuntimeStub::new_runtime_stub(name, &buffer, frame_complete, frame_size_words, oop_maps, true);
}

void SharedRuntime::generate_stubs() {

  _wrong_method_blob = generate_resolve_blob(CAST_FROM_FN_PTR(address, SharedRuntime::handle_wrong_method),
                                        "wrong_method_stub");

  _ic_miss_blob = generate_resolve_blob(CAST_FROM_FN_PTR(address, SharedRuntime::handle_wrong_method_ic_miss),
                                        "ic_miss_stub");

  _resolve_opt_virtual_call_blob = generate_resolve_blob(CAST_FROM_FN_PTR(address, SharedRuntime::resolve_opt_virtual_call_C),
                                        "resolve_opt_virtual_call");

  _resolve_virtual_call_blob = generate_resolve_blob(CAST_FROM_FN_PTR(address, SharedRuntime::resolve_virtual_call_C),
                                        "resolve_virtual_call");

  _resolve_static_call_blob = generate_resolve_blob(CAST_FROM_FN_PTR(address, SharedRuntime::resolve_static_call_C),
                                        "resolve_static_call");
  _polling_page_safepoint_handler_blob =
    generate_handler_blob(CAST_FROM_FN_PTR(address,
                   SafepointSynchronize::handle_polling_page_exception), false);

  _polling_page_return_handler_blob =
    generate_handler_blob(CAST_FROM_FN_PTR(address,
                   SafepointSynchronize::handle_polling_page_exception), true);

  generate_deopt_blob();
#ifdef COMPILER2
  _uncommon_trap_blob = generate_uncommon_trap_blob();
#endif // COMPILER2
}


extern "C" int SpinPause() {return 0;}
extern "C" int SafeFetch32 (int * adr, int errValue) {return 0;} ;
extern "C" intptr_t SafeFetchN (intptr_t * adr, intptr_t errValue) {return *adr; } ;
