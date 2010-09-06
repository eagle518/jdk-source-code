#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)assembler_i486.cpp	1.190 04/02/25 22:13:37 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_assembler_i486.cpp.incl"


// Implementation of Address
//
// Note: For now an address simply stores all its attributes in individual fields. A better solution would
//       do the conversion into a byte stream (currently performed by Assembler::emit_operand) when creating
//       an address. The byte stream & some management information could be easily compressed into a two word
//       object, making the use of addresses both faster & more lightweight (addresses are used frequently &
//       often the same address is used several times. Doing the conversion when creating the address saves
//       repeated conversion by the assembler).

Address::Address(int disp, relocInfo::relocType rtype) {
  _base  = noreg;
  _index = noreg;
  _scale = no_scale;
  _disp  = disp;
  switch (rtype) {
    case relocInfo::external_word_type:
      _rspec = external_word_Relocation::spec((address)disp);
      break;
    case relocInfo::internal_word_type:
      _rspec = internal_word_Relocation::spec((address)disp);
      break;
    case relocInfo::none:
      break;
    default:
      ShouldNotReachHere();
  }
}


// Implementation of Displacement

// Using me, bind instruction to be backpatched (addr in unbound label L)
// to position in pos argument, using assembler masm.

void Displacement::bind(Label &L, int pos, AbstractAssembler* masm) {
  int fixup_pos = L.pos();
  int imm32 = 0;
  switch (type()) {
    case call:
      { assert(masm->byte_at(fixup_pos - 1) == 0xE8, "call expected");
        imm32 = pos - (fixup_pos + sizeof(int));
      }
      break;
    case absolute_jump:
      { assert(masm->byte_at(fixup_pos - 1) == 0xE9, "jmp expected");
        imm32 = pos - (fixup_pos + sizeof(int));
      }
      break;
    case conditional_jump:
      { assert(masm->byte_at(fixup_pos - 2) == 0x0F, "jcc expected");
        assert(masm->byte_at(fixup_pos - 1) == (0x80 | info()), "jcc expected");
        imm32 = pos - (fixup_pos + sizeof(int));
      }
      break;
    default:
      ShouldNotReachHere();
  }
  masm->long_at_put(fixup_pos, imm32);
  next(L);
}


// Implementation of Assembler

int AbstractAssembler::code_fill_byte() {
  return (u_char)'\xF4'; // hlt
}


void Assembler::emit_data(jint data, relocInfo::relocType rtype, int format) {
  if (rtype == relocInfo::none)
        emit_long(data);
  else  emit_data(data, Relocation::spec_simple(rtype), format);
}


void Assembler::emit_data(jint data, RelocationHolder const& rspec, int format) {
  assert(imm32_operand == 0, "default format must be imm32 in this file");
  assert(_inst_mark != NULL, "must be inside InstructionMark");
  // Do not use AbstractAssembler::relocate, which is not intended for
  // embedded words.  Instead, relocate to the enclosing instruction.
  code()->relocate(_inst_mark, rspec, format);
  #ifdef ASSERT
    check_relocation(rspec, format);
  #endif
  emit_long(data);
}


void Assembler::emit_arith_b(int op1, int op2, Register dst, int imm8) {
  assert(dst->has_byte_register(), "must have byte register");
  assert(isByte(op1) && isByte(op2), "wrong opcode");
  assert(isByte(imm8), "not a byte");
  assert((op1 & 0x01) == 0, "should be 8bit operation");
  emit_byte(op1);
  emit_byte(op2 | dst->encoding());
  emit_byte(imm8);
}


void Assembler::emit_arith(int op1, int op2, Register dst, int imm32) {
  assert(isByte(op1) && isByte(op2), "wrong opcode");
  assert((op1 & 0x01) == 1, "should be 32bit operation");
  assert((op1 & 0x02) == 0, "sign-extension bit should not be set");
  if (is8bit(imm32)) {
    emit_byte(op1 | 0x02); // set sign bit
    emit_byte(op2 | dst->encoding());
    emit_byte(imm32 & 0xFF);
  } else {
    emit_byte(op1);
    emit_byte(op2 | dst->encoding());
    emit_long(imm32);
  }
}

// immediate-to-memory forms
void Assembler::emit_arith_operand(int op1, Register rm, Address adr, int imm32) {
  assert((op1 & 0x01) == 1, "should be 32bit operation");
  assert((op1 & 0x02) == 0, "sign-extension bit should not be set");
  if (is8bit(imm32)) {
    emit_byte(op1 | 0x02); // set sign bit
    emit_operand(rm,adr);
    emit_byte(imm32 & 0xFF);
  } else {
    emit_byte(op1);
    emit_operand(rm,adr);
    emit_long(imm32);
  }
}

void Assembler::emit_arith(int op1, int op2, Register dst, jobject obj) {
  assert(isByte(op1) && isByte(op2), "wrong opcode");
  assert((op1 & 0x01) == 1, "should be 32bit operation");
  assert((op1 & 0x02) == 0, "sign-extension bit should not be set");
  InstructionMark im(this);
  emit_byte(op1);
  emit_byte(op2 | dst->encoding());
  emit_data((int)obj, relocInfo::oop_type);
}


void Assembler::emit_arith(int op1, int op2, Register dst, Register src) {
  assert(isByte(op1) && isByte(op2), "wrong opcode");
  emit_byte(op1);
  emit_byte(op2 | dst->encoding() << 3 | src->encoding());
}


void Assembler::emit_operand(Register reg, Register base, Register index, Address::ScaleFactor scale, int disp, RelocationHolder const& rspec) {
  relocInfo::relocType rtype = (relocInfo::relocType) rspec.type();
  if (base->is_valid()) {
    if (index->is_valid()) {
      assert(scale != Address::no_scale, "inconsistent address");
      // [base + index*scale + disp]
      if (disp == 0 && rtype == relocInfo::none) {
        // [base + index*scale]
        // [00 reg 100][ss index base]
        assert(index != esp && base != ebp, "illegal addressing mode");
        emit_byte(0x04 | reg->encoding() << 3);
        emit_byte(scale << 6 | index->encoding() << 3 | base->encoding());
      } else if (is8bit(disp) && rtype == relocInfo::none) {
        // [base + index*scale + imm8]
        // [01 reg 100][ss index base] imm8
	assert(index != esp, "illegal addressing mode");
        emit_byte(0x44 | reg->encoding() << 3);
        emit_byte(scale << 6 | index->encoding() << 3 | base->encoding());
        emit_byte(disp & 0xFF);
      } else {
        // [base + index*scale + imm32]
        // [10 reg 100][ss index base] imm32
	assert(index != esp, "illegal addressing mode");
	emit_byte(0x84 | reg->encoding() << 3);
        emit_byte(scale << 6 | index->encoding() << 3 | base->encoding());
	emit_data(disp, rspec, disp32_operand);
      }
    } else if (base == esp) {
      // [esp + disp]
      if (disp == 0 && rtype == relocInfo::none) {
        // [esp]
        // [00 reg 100][00 100 100]
        emit_byte(0x04 | reg->encoding() << 3);
        emit_byte(0x24);
      } else if (is8bit(disp) && rtype == relocInfo::none) {
        // [esp + imm8]
        // [01 reg 100][00 100 100] imm8
        emit_byte(0x44 | reg->encoding() << 3);
        emit_byte(0x24);
        emit_byte(disp & 0xFF);
      } else {
        // [esp + imm32]
        // [10 reg 100][00 100 100] imm32
	emit_byte(0x84 | reg->encoding() << 3);
        emit_byte(0x24);
	emit_data(disp, rspec, disp32_operand);
      }
    } else {
      // [base + disp]
      assert(base != esp, "illegal addressing mode");
      if (disp == 0 && rtype == relocInfo::none && base != ebp) {
        // [base]
        // [00 reg base]
	assert(base != ebp, "illegal addressing mode");
        emit_byte(0x00 | reg->encoding() << 3 | base->encoding());
      } else if (is8bit(disp) && rtype == relocInfo::none) {
        // [base + imm8]
        // [01 reg base] imm8
        emit_byte(0x40 | reg->encoding() << 3 | base->encoding());
        emit_byte(disp & 0xFF);
      } else {
        // [base + imm32]
        // [10 reg base] imm32
        emit_byte(0x80 | reg->encoding() << 3 | base->encoding());
        emit_data(disp, rspec, disp32_operand);
      }
    }
  } else {
    if (index->is_valid()) {
      assert(scale != Address::no_scale, "inconsistent address");
      // [index*scale + disp]
      // [00 reg 100][ss index 101] imm32
      assert(index != esp, "illegal addressing mode");
      emit_byte(0x04 | reg->encoding() << 3);
      emit_byte(scale << 6 | index->encoding() << 3 | 0x05);
      emit_data(disp, rspec, disp32_operand);
    } else {
      // [disp]
      // [00 reg 101] imm32
      emit_byte(0x05 | reg->encoding() << 3);
      emit_data(disp, rspec, disp32_operand);
    }
  }
}

// Secret local extension to Assembler::WhichOperand:
#define end_pc_operand (_WhichOperand_limit)

address Assembler::locate_operand(address inst, WhichOperand which) {
  // Decode the given instruction, and return the address of
  // an embedded 32-bit operand word.

  // If "which" is disp32_operand, selects the displacement portion
  // of an effective address specifier.
  // If "which" is imm32_operand, selects the trailing immediate constant.
  // If "which" is call32_operand, selects the displacement of a call or jump.
  // Caller is responsible for ensuring that there is such an operand,
  // and that it is 32 bits wide.

  // If "which" is end_pc_operand, find the end of the instruction.

  address ip = inst;

  debug_only(bool has_imm32 = false);
  int tail_size = 0;    // other random bytes (#32, #16, etc.) at end of insn

 again_after_prefix:
  switch (0xFF & *ip++) {

  // These convenience macros generate groups of "case" labels for the switch.
  #define REP4(x) (x)+0: case (x)+1: case (x)+2: case (x)+3
  #define REP8(x) (x)+0: case (x)+1: case (x)+2: case (x)+3: \
             case (x)+4: case (x)+5: case (x)+6: case (x)+7
  #define REP16(x) REP8((x)+0): \
              case REP8((x)+8)

  case CS_segment:
  case SS_segment:
  case DS_segment:
  case ES_segment:
  case FS_segment:
  case GS_segment:
    assert(ip == inst+1, "only one prefix allowed");
    goto again_after_prefix;

  case 0xFF: // pushl a; decl a; incl a; call a; jmp a
  case 0x88: // movb a, r
  case 0x89: // movl a, r
  case 0x8A: // movb r, a
  case 0x8B: // movl r, a
  case 0x8F: // popl a
    break;

  case 0x68: // pushl #32(oop?)
    if (which == end_pc_operand)  return ip + 4;
    assert(which == imm32_operand, "pushl has no disp32");
    return ip;			// not produced by emit_operand

  case 0x66: // movw ... (size prefix)
    switch (0xFF & *ip++) {
    case 0x8B: // movw r, a
    case 0x89: // movw a, r
      break;
    case 0xC7: // movw a, #16
      tail_size = 2;  // the imm16
      break;
    case 0x0F: // several SSE/SSE2 variants
      ip--;    // reparse the 0x0F
      goto again_after_prefix;
    default:
      ShouldNotReachHere();
    }
    break;

  case REP8(0xB8): // movl r, #32(oop?)
    if (which == end_pc_operand)  return ip + 4;
    assert(which == imm32_operand || which == disp32_operand, "");
    return ip;

  case 0x69: // imul r, a, #32
  case 0xC7: // movl a, #32(oop?)
    tail_size = 4;
    debug_only(has_imm32 = true); // has both kinds of operands!
    break;

  case 0x0F: // movx..., etc.
    switch (0xFF & *ip++) {
    case 0x2F: // comiss
    case 0x57: // xorps
    case 0xAD: // shrd r, a, %cl
    case 0xAF: // imul r, a
    case 0xBE: // movsxb r, a
    case 0xBF: // movsxw r, a
    case 0xB6: // movzxb r, a
    case 0xB7: // movzxw r, a
    case REP16(0x40): // cmovl cc, r, a
    case 0xB0: // cmpxchgb
    case 0xB1: // cmpxchg
    case 0xC1: // xaddl
    case 0xC7: // cmpxchg8
    case REP16(0x90): // setcc a
      // fall out of the switch to decode the address
      break;
    case 0xAC: // shrd r, a, #8
      tail_size = 1;  // the imm8
      break;
    case REP16(0x80): // jcc rdisp32
      if (which == end_pc_operand)  return ip + 4;
      assert(which == call32_operand, "jcc has no disp32 or imm32");
      return ip;
    default:
      ShouldNotReachHere();
    }
    break;

  case 0x81: // addl a, #32; addl r, #32
    // also: orl, adcl, sbbl, andl, subl, xorl, cmpl
    // in the case of cmpl, the imm32 might be an oop
    tail_size = 4;
    debug_only(has_imm32 = true); // has both kinds of operands!
    break;

  case 0x83: // addl a, #8; addl r, #8
    // also: orl, adcl, sbbl, andl, subl, xorl, cmpl
    tail_size = 1;
    break;

  case 0x9B:
    switch (0xFF & *ip++) {
    case 0xD9: // fnstcw a
      break;
    default:
      ShouldNotReachHere();
    }
    break;

  case REP4(0x00): // addb a, r; addl a, r; addb r, a; addl r, a
  case REP4(0x10): // adc...
  case REP4(0x20): // and...
  case REP4(0x30): // xor...
  case REP4(0x08): // or...
  case REP4(0x18): // sbb...
  case REP4(0x28): // sub...
  case REP4(0x38): // cmp...
  case 0x8D: // leal r, a
  case 0xF7: // mull a
  case 0x87: // xchg r, a
    break;

  case 0xC6: // movb a, #8 
  case 0x80: // cmpb a, #8
  case 0x6B: // imul r, a, #8
    tail_size = 1; // the imm8
    break;

  case 0xE8: // call rdisp32
  case 0xE9: // jmp  rdisp32
    if (which == end_pc_operand)  return ip + 4;
    assert(which == call32_operand, "call has no disp32 or imm32");
    return ip;

  case 0xD1: // sal a, 1; sar a, 1; shl a, 1; shr a, 1
  case 0xD3: // sal a, %cl; sar a, %cl; shl a, %cl; shr a, %cl
  case 0xD9: // fld_s a; fst_s a; fstp_s a; fldcw a
  case 0xDD: // fld_d a; fst_d a; fstp_d a
  case 0xDB: // fild_s a; fistp_s a; fld_x a; fstp_x a
  case 0xDF: // fild_d a; fistp_d a
  case 0xD8: // fadd_s a; fsubr_s a; fmul_s a; fdivr_s a; fcomp_s a
  case 0xDC: // fadd_d a; fsubr_d a; fmul_d a; fdivr_d a; fcomp_d a
  case 0xDE: // faddp_d a; fsubrp_d a; fmulp_d a; fdivrp_d a; fcompp_d a
    break;

  case 0xF3:                    // For SSE
  case 0xF2:                    // For SSE2
    ip++; ip++;
    break;

  default:
    ShouldNotReachHere();

  #undef REP8
  #undef REP16
  }

  assert(which != call32_operand, "instruction is not a call, jmp, or jcc");
  assert(which != imm32_operand || has_imm32, "instruction has no imm32 field");

  // parse the output of emit_operand
  int op2 = 0xFF & *ip++;
  int base = op2 & 0x07;
  int op3 = -1;
  const int b100 = 4;
  const int b101 = 5;
  if (base == b100 && (op2 >> 6) != 3) {
    op3 = 0xFF & *ip++;
    base = op3 & 0x07;   // refetch the base
  }
  // now ip points at the disp (if any)

  switch (op2 >> 6) {
  case 0:
    // [00 reg  100][ss index base]
    // [00 reg  100][00   100  esp]
    // [00 reg base]
    // [00 reg  100][ss index  101][disp32]
    // [00 reg  101]               [disp32]
    
    if (base == b101) {
      if (which == disp32_operand)
	return ip;		// caller wants the disp32
      ip += 4;			// skip the disp32
    }
    break;

  case 1:
    // [01 reg  100][ss index base][disp8]
    // [01 reg  100][00   100  esp][disp8]
    // [01 reg base]               [disp8]
    ip += 1;			// skip the disp8
    break;

  case 2:
    // [10 reg  100][ss index base][disp32]
    // [10 reg  100][00   100  esp][disp32]
    // [10 reg base]               [disp32]
    if (which == disp32_operand)
      return ip;		// caller wants the disp32
    ip += 4;			// skip the disp32
    break;

  case 3:
    // [11 reg base]  (not a memory addressing mode)
    break;
  }

  if (which == end_pc_operand) {
    return ip + tail_size;
  }

  assert(which == imm32_operand, "instruction has only an imm32 field");
  return ip;
}

address Assembler::locate_next_instruction(address inst) {
  // Secretly share code with locate_operand:
  return locate_operand(inst, end_pc_operand);
}


#ifdef ASSERT
void Assembler::check_relocation(RelocationHolder const& rspec, int format) {
  address inst = _inst_mark;
  assert(inst != NULL && inst < pc(), "must point to beginning of instruction");
  address opnd;

  Relocation* r = rspec.reloc();
  if (r->type() == relocInfo::none) {
    return;
  } else if (r->is_call()) {
    assert(format == 0, "cannot specify a nonzero format");
    opnd = locate_operand(inst, call32_operand);
  } else if (r->is_data()) {
    assert(format == imm32_operand || format == disp32_operand, "format ok");
    opnd = locate_operand(inst, (WhichOperand)format);
  } else {
    assert(format == 0, "cannot specify a format");
    return;
  }
  assert(opnd == pc(), "must put operand where relocs can find it");
}
#endif



void Assembler::emit_operand(Register reg, Address adr) {
  emit_operand(reg, adr._base, adr._index, adr._scale, adr._disp, adr._rspec);
}


void Assembler::emit_farith(int b1, int b2, int i) {
  assert(isByte(b1) && isByte(b2), "wrong opcode");
  assert(0 <= i &&  i < 8, "illegal stack offset");
  emit_byte(b1);
  emit_byte(b2 + i);
}


void Assembler::pushad() {
  emit_byte(0x60);
}

void Assembler::popad() {
  emit_byte(0x61);
}

void Assembler::pushfd() {
  emit_byte(0x9C);
}

void Assembler::popfd() {
  emit_byte(0x9D);
}

void Assembler::pushl(int imm32) {
  emit_byte(0x68);
  emit_long(imm32);  
}

void Assembler::pushl(int imm32, relocInfo::relocType rtype) {
  InstructionMark im(this);
  emit_byte(0x68);
  emit_data(imm32, rtype);  
}
  
void Assembler::pushl(int imm32, RelocationHolder const& rspec) {
  InstructionMark im(this);
  emit_byte(0x68);
  emit_data(imm32, rspec);  
}
  
void Assembler::pushl(jobject obj) {
  InstructionMark im(this);
  emit_byte(0x68);
  emit_data((int)obj, relocInfo::oop_type);  
}

  
void Assembler::pushl(Register src) {
  emit_byte(0x50 | src->encoding());  
}


void Assembler::pushl(Address src) {
  InstructionMark im(this);
  emit_byte(0xFF);
  emit_operand(esi, src);  
}


void Assembler::pushl(Label& L, relocInfo::relocType rtype) {
  if (L.is_bound()) {
    int offs = (int)_code_begin + L.pos();
    InstructionMark im(this);
    emit_byte(0x68);
    emit_data(offs, rtype);
  } else {
    ShouldNotReachHere();
  }  
}


void Assembler::popl(Register dst) {
  emit_byte(0x58 | dst->encoding());  
}


void Assembler::popl(Address dst) {
  InstructionMark im(this);
  emit_byte(0x8F);
  emit_operand(eax, dst);
}


void Assembler::prefix(Prefix p) {
  a_byte(p);
}


void Assembler::movb(Register dst, Address src) {
  assert(dst->has_byte_register(), "must have byte register"); 
  InstructionMark im(this);
  emit_byte(0x8A);
  emit_operand(dst, src);
}


void Assembler::movb(Address dst, int imm8) {
  InstructionMark im(this);
  emit_byte(0xC6);
  emit_operand(eax, dst);
  emit_byte(imm8);
}


void Assembler::movb(Address dst, Register src) {
  assert(src->has_byte_register(), "must have byte register"); 
  InstructionMark im(this);
  emit_byte(0x88);
  emit_operand(src, dst);
}


void Assembler::movw(Address dst, int imm16) {
  InstructionMark im(this);

  emit_byte(0x66); // switch to 16-bit mode
  emit_byte(0xC7);
  emit_operand(eax, dst);
  emit_word(imm16);
}

  
void Assembler::movw(Register dst, Address src) {
  InstructionMark im(this);
  emit_byte(0x66);
  emit_byte(0x8B);
  emit_operand(dst, src);
}


void Assembler::movw(Address dst, Register src) {
  InstructionMark im(this);
  emit_byte(0x66);
  emit_byte(0x89);
  emit_operand(src, dst);
}


void Assembler::movl(Register dst, int imm32) {
  emit_byte(0xB8 | dst->encoding());
  emit_long(imm32);
}


void Assembler::movl(Register dst, jobject obj) {
  InstructionMark im(this);
  emit_byte(0xB8 | dst->encoding());
  emit_data((int)obj, relocInfo::oop_type);
}


void Assembler::movl(Register dst, Register src) {
  emit_byte(0x8B);
  emit_byte(0xC0 | (dst->encoding() << 3) | src->encoding());
}


void Assembler::movl(Register dst, Address src) {
  InstructionMark im(this);
  emit_byte(0x8B);
  emit_operand(dst, src);
}


void Assembler::movl(Address dst, int imm32) {
  InstructionMark im(this);
  emit_byte(0xC7);
  emit_operand(eax, dst);
  emit_long(imm32);
}


void Assembler::movl(Address dst, jobject obj) {
  InstructionMark im(this);	// Note:  This insn might have two relocations.
  emit_byte(0xC7);
  emit_operand(eax, dst);
  emit_data((int)obj, relocInfo::oop_type);
}


void Assembler::movl(Address dst, Register src) {
  InstructionMark im(this);
  emit_byte(0x89);
  emit_operand(src, dst);
}


void Assembler::movsxb(Register dst, Address src) {
  InstructionMark im(this);
  emit_byte(0x0F);
  emit_byte(0xBE);
  emit_operand(dst, src);
}


void Assembler::movsxb(Register dst, Register src) {
  assert(src->has_byte_register(), "must have byte register"); 
  emit_byte(0x0F);
  emit_byte(0xBE);
  emit_byte(0xC0 | (dst->encoding() << 3) | src->encoding());
}


void Assembler::movsxw(Register dst, Address src) {
  InstructionMark im(this);
  emit_byte(0x0F);
  emit_byte(0xBF);
  emit_operand(dst, src);
}

  
void Assembler::movsxw(Register dst, Register src) {
  emit_byte(0x0F);
  emit_byte(0xBF);
  emit_byte(0xC0 | (dst->encoding() << 3) | src->encoding());
}

  
void Assembler::movzxb(Register dst, Address src) {
  InstructionMark im(this);
  emit_byte(0x0F);
  emit_byte(0xB6);
  emit_operand(dst, src);
}


void Assembler::movzxb(Register dst, Register src) {
  assert(src->has_byte_register(), "must have byte register"); 
  emit_byte(0x0F);
  emit_byte(0xB6);
  emit_byte(0xC0 | (dst->encoding() << 3) | src->encoding());
}


void Assembler::movzxw(Register dst, Address src) {
  InstructionMark im(this);
  emit_byte(0x0F);
  emit_byte(0xB7);
  emit_operand(dst, src);
}

  
void Assembler::movzxw(Register dst, Register src) {
  emit_byte(0x0F);
  emit_byte(0xB7);
  emit_byte(0xC0 | (dst->encoding() << 3) | src->encoding());
}


void Assembler::cmovl(Condition cc, Register dst, Register src) {
  guarantee(VM_Version::supports_cmov(), "illegal instruction");
  emit_byte(0x0F);
  emit_byte(0x40 | cc);
  emit_byte(0xC0 | (dst->encoding() << 3) | src->encoding());
}


void Assembler::cmovl(Condition cc, Register dst, Address src) {
  guarantee(VM_Version::supports_cmov(), "illegal instruction");
  // The code below seems to be wrong - however the manual is inconclusive
  // do not use for now (remember to enable all callers when fixing this)
  Unimplemented();
  // wrong bytes?
  InstructionMark im(this);
  emit_byte(0x0F);
  emit_byte(0x40 | cc);
  emit_operand(dst, src);
}


void Assembler::adcl(Register dst, int imm32) {
  emit_arith(0x81, 0xD0, dst, imm32);
}


void Assembler::adcl(Register dst, Address src) {
  InstructionMark im(this);
  emit_byte(0x13);
  emit_operand(dst, src);
}


void Assembler::adcl(Register dst, Register src) {
  emit_arith(0x13, 0xC0, dst, src);
}


void Assembler::addl(Address dst, int imm32) {
  InstructionMark im(this);
  emit_arith_operand(0x81,eax,dst,imm32);
}


void Assembler::addl(Address dst, Register src) {
  InstructionMark im(this);
  emit_byte(0x01);
  emit_operand(src, dst);
}


void Assembler::addl(Register dst, int imm32) {
  emit_arith(0x81, 0xC0, dst, imm32);
}


void Assembler::addl(Register dst, Address src) {
  InstructionMark im(this);
  emit_byte(0x03);
  emit_operand(dst, src);
}


void Assembler::addl(Register dst, Register src) {
  emit_arith(0x03, 0xC0, dst, src);
}


void Assembler::andl(Register dst, int imm32) {
  emit_arith(0x81, 0xE0, dst, imm32);
}


void Assembler::andl(Register dst, Address src) {
  InstructionMark im(this);
  emit_byte(0x23);
  emit_operand(dst, src);
}


void Assembler::andl(Register dst, Register src) {
  emit_arith(0x23, 0xC0, dst, src);
}


void Assembler::cmpb(Address dst, int imm8) {
  InstructionMark im(this);
  emit_byte(0x80);
  emit_operand(edi, dst);
  emit_byte(imm8);
}

void Assembler::cmpl(Address dst, int imm32) {
  InstructionMark im(this);
  emit_byte(0x81);
  emit_operand(edi, dst);
  emit_long(imm32);
}


void Assembler::cmpl(Address dst, jobject obj) {
  InstructionMark im(this);	// Note:  This insn might have two relocations.
  emit_byte(0x81);
  emit_operand(edi, dst);
  emit_data((int)obj, relocInfo::oop_type);
}


void Assembler::cmpl(Register dst, int imm32) {
  emit_arith(0x81, 0xF8, dst, imm32);
}


void Assembler::cmpl(Register dst, jobject obj) {
  emit_arith(0x81, 0xF8, dst, obj);
}


void Assembler::cmpl(Register dst, Register src) {
  emit_arith(0x3B, 0xC0, dst, src);
}


void Assembler::cmpl(Register dst, Address  src) {
  InstructionMark im(this);
  emit_byte(0x3B);
  emit_operand(dst, src);
}


void Assembler::decb(Register dst) {
  assert(dst->has_byte_register(), "must have byte register"); 
  emit_byte(0xFE);
  emit_byte(0xC8 | dst->encoding());
}


void Assembler::decl(Register dst) {
  emit_byte(0x48 | dst->encoding());
}


void Assembler::decl(Address dst) {
  InstructionMark im(this);
  emit_byte(0xFF);
  emit_operand(ecx, dst);
}


void Assembler::idivl(Register src) {
  emit_byte(0xF7);
  emit_byte(0xF8 | src->encoding());
}


void Assembler::cdql() {
  emit_byte(0x99);
}


void Assembler::imull(Register dst, Register src) {
  emit_byte(0x0F);
  emit_byte(0xAF);
  emit_byte(0xC0 | dst->encoding() << 3 | src->encoding());
}


void Assembler::imull(Register dst, Register src, int value) {
  if (is8bit(value)) {
    emit_byte(0x6B);
    emit_byte(0xC0 | dst->encoding() << 3 | src->encoding());
    emit_byte(value);
  } else {
    emit_byte(0x69);
    emit_byte(0xC0 | dst->encoding() << 3 | src->encoding());
    emit_long(value);
  }
}


void Assembler::incl(Register dst) {
  emit_byte(0x40 | dst->encoding());
}


void Assembler::incl(Address dst) {
  InstructionMark im(this);
  emit_byte(0xFF);
  emit_operand(eax, dst);
}


void Assembler::leal(Register dst, Address src) {
  InstructionMark im(this);
  emit_byte(0x8D);
  emit_operand(dst, src);
}


void Assembler::mull(Address src) {
  InstructionMark im(this);
  emit_byte(0xF7);
  emit_operand(esp, src);
}


void Assembler::mull(Register src) {
  emit_byte(0xF7);
  emit_byte(0xE0 | src->encoding());
}


void Assembler::negl(Register dst) {
  emit_byte(0xF7);
  emit_byte(0xD8 | dst->encoding());
}


void Assembler::notl(Register dst) {
  emit_byte(0xF7);
  emit_byte(0xD0 | dst->encoding());
}


void Assembler::orl(Address dst, int imm32) {
  InstructionMark im(this);
  emit_byte(0x81);
  emit_operand(ecx, dst);
  emit_long(imm32);
}

void Assembler::orl(Register dst, int imm32) {
  emit_arith(0x81, 0xC8, dst, imm32);
}


void Assembler::orl(Register dst, Address src) {
  InstructionMark im(this);
  emit_byte(0x0B);
  emit_operand(dst, src);
}


void Assembler::orl(Register dst, Register src) {
  emit_arith(0x0B, 0xC0, dst, src);
}


void Assembler::rcll(Register dst, int imm8) {
  assert(isShiftCount(imm8), "illegal shift count");
  if (imm8 == 1) {
    emit_byte(0xD1);
    emit_byte(0xD0 | dst->encoding());
  } else {
    emit_byte(0xC1);
    emit_byte(0xD0 | dst->encoding());
    emit_byte(imm8);
  }
}


void Assembler::sarl(Register dst, int imm8) {
  assert(isShiftCount(imm8), "illegal shift count");
  if (imm8 == 1) {
    emit_byte(0xD1);
    emit_byte(0xF8 | dst->encoding());
  } else {
    emit_byte(0xC1);
    emit_byte(0xF8 | dst->encoding());
    emit_byte(imm8);
  }
}


void Assembler::sarl(Register dst) {
  emit_byte(0xD3);
  emit_byte(0xF8 | dst->encoding());
}


void Assembler::sbbl(Address dst, int imm32) {
  InstructionMark im(this);
  emit_arith_operand(0x81,ebx,dst,imm32);
}


void Assembler::sbbl(Register dst, int imm32) {
  emit_arith(0x81, 0xD8, dst, imm32);
}


void Assembler::sbbl(Register dst, Address src) {
  InstructionMark im(this);
  emit_byte(0x1B);
  emit_operand(dst, src);
}


void Assembler::sbbl(Register dst, Register src) {
  emit_arith(0x1B, 0xC0, dst, src);
}


void Assembler::shldl(Register dst, Register src) {
  emit_byte(0x0F);
  emit_byte(0xA5);
  emit_byte(0xC0 | src->encoding() << 3 | dst->encoding());
}


void Assembler::shll(Register dst, int imm8) {
  assert(isShiftCount(imm8), "illegal shift count");
  if (imm8 == 1 ) {
    emit_byte(0xD1);
    emit_byte(0xE0 | dst->encoding());
  } else {
    emit_byte(0xC1);
    emit_byte(0xE0 | dst->encoding());
    emit_byte(imm8);
  }
}


void Assembler::shll(Register dst) {
  emit_byte(0xD3);
  emit_byte(0xE0 | dst->encoding());
}


void Assembler::shrdl(Register dst, Register src) {
  emit_byte(0x0F);
  emit_byte(0xAD);
  emit_byte(0xC0 | src->encoding() << 3 | dst->encoding());
}


void Assembler::shrl(Register dst, int imm8) {
  assert(isShiftCount(imm8), "illegal shift count");
  emit_byte(0xC1);
  emit_byte(0xE8 | dst->encoding());
  emit_byte(imm8);
}


void Assembler::shrl(Register dst) {
  emit_byte(0xD3);
  emit_byte(0xE8 | dst->encoding());
}

  
void Assembler::subl(Address dst, int imm32) {
  if (is8bit(imm32)) {
    InstructionMark im(this);
    emit_byte(0x83);
    emit_operand(ebp, dst);
    emit_byte(imm32 & 0xFF);
  } else {
    InstructionMark im(this);
    emit_byte(0x81);
    emit_operand(ebp, dst);
    emit_long(imm32);
  }
}


void Assembler::subl(Register dst, int imm32) {
  emit_arith(0x81, 0xE8, dst, imm32);
}


void Assembler::subl(Address dst, Register src) {
  InstructionMark im(this);
  emit_byte(0x29);
  emit_operand(src, dst);
}


void Assembler::subl(Register dst, Address src) {
  InstructionMark im(this);
  emit_byte(0x2B);
  emit_operand(dst, src);
}


void Assembler::subl(Register dst, Register src) {
  emit_arith(0x2B, 0xC0, dst, src);
}


void Assembler::testb(Register dst, int imm8) {
  assert(dst->has_byte_register(), "must have byte register"); 
  emit_arith_b(0xF6, 0xC0, dst, imm8);
}


void Assembler::testl(Register dst, int imm32) {
  // not using emit_arith because test
  // doesn't support sign-extension of
  // 8bit operands
  if (dst->encoding() == 0) {
    emit_byte(0xA9);
  } else {
    emit_byte(0xF7);
    emit_byte(0xC0 | dst->encoding());
  }
  emit_long(imm32);
}


void Assembler::testl(Register dst, Register src) {
  emit_arith(0x85, 0xC0, dst, src);
}

void Assembler::testl(Register dst, Address  src) {
  InstructionMark im(this);
  emit_byte(0x85);
  emit_operand(dst, src);
}

void Assembler::xaddl(Address dst, Register src) {
  InstructionMark im(this);
  emit_byte(0x0F);
  emit_byte(0xC1);
  emit_operand(src, dst);
}

void Assembler::xorl(Register dst, int imm32) {
  emit_arith(0x81, 0xF0, dst, imm32);
}


void Assembler::xorl(Register dst, Address src) {
  InstructionMark im(this);
  emit_byte(0x33);
  emit_operand(dst, src);
}


void Assembler::xorl(Register dst, Register src) {
  emit_arith(0x33, 0xC0, dst, src);
}


void Assembler::bswap(Register reg) {
  emit_byte(0x0F);
  emit_byte(0xC8 | reg->encoding());
}


void Assembler::lock() {
  emit_byte(0xF0);
}


void Assembler::xchg(Register reg, Address adr) {
  InstructionMark im(this);
  emit_byte(0x87);
  emit_operand(reg, adr);
}


void Assembler::xchgl(Register dst, Register src) {
  InstructionMark im(this);
  emit_byte(0x87);
  emit_byte(0xc0 | dst->encoding() << 3 | src->encoding());
}


// The 32-bit cmpxchg compares the value at adr with the contents of eax,
// and stores reg into adr if so; otherwise, the value at adr is loaded into eax.
// The ZF is set if the compared values were equal, and cleared otherwise.
void Assembler::cmpxchg(Register reg, Address adr) {
  InstructionMark im(this);
  emit_byte(0x0F);
  emit_byte(0xB1);
  emit_operand(reg, adr);
}

// The 64-bit cmpxchg compares the value at adr with the contents of edx:eax,
// and stores ecx:ebx into adr if so; otherwise, the value at adr is loaded
// into edx:eax.  The ZF is set if the compared values were equal, and cleared otherwise.
void Assembler::cmpxchg8(Address adr) {
  InstructionMark im(this);
  emit_byte(0x0F);
  emit_byte(0xc7);
  emit_operand(ecx, adr);
}

void Assembler::hlt() {
  emit_byte(0xF4);
}


void Assembler::nop() {
  emit_byte(0x90);
}

void Assembler::ret(int imm16) {
  if (imm16 == 0) {
    emit_byte(0xC3);
  } else {
    emit_byte(0xC2);
    emit_word(imm16);
  }
}


void Assembler::set_byte_if_not_zero(Register dst) {
  emit_byte(0x0F);
  emit_byte(0x95);
  emit_byte(0xE0 | dst->encoding());
}


// copies data from [esi] to [edi] using ecx double words (m32)
void Assembler::rep_move() {
  emit_byte(0xF3);
  emit_byte(0xA5);
}


// sets ecx double words (m32) with eax value at [edi]
void Assembler::rep_set() {
  emit_byte(0xF3);
  emit_byte(0xAB);
}

// scans ecx double words (m32) at [edi] for occurance of eax
void Assembler::repne_scan() {
  emit_byte(0xF2);
  emit_byte(0xAF);
}


void Assembler::setb(Condition cc, Register dst) {
  assert(0 <= cc && cc < 16, "illegal cc");
  emit_byte(0x0F);
  emit_byte(0x90 | cc);
  emit_byte(0xC0 | dst->encoding());
}

// Serializes memory.
void Assembler::membar() {
    // Memory barriers are only needed on multiprocessors
  if (os::is_MP()) {
    if( VM_Version::supports_sse2() ) {
      emit_byte( 0x0F );		// MFENCE; faster blows no regs
      emit_byte( 0xAE );
      emit_byte( 0xF0 );
    } else {
      // All usable chips support "locked" instructions which suffice
      // as barriers, and are much faster than the alternative of
      // using cpuid instruction. We use here a locked add [esp],0.
      // This is conveniently otherwise a no-op except for blowing
      // flags (which we save and restore.)
      pushfd();                // Save eflags register 
      lock();                   
      addl(Address(esp), 0);   // Assert the lock# signal here 
      popfd();                 // Restore eflags register 
    }
  }
}

// Identify processor type and features
void Assembler::cpuid() {
  // Note: we can't assert VM_Version::supports_cpuid() here
  //       because this instruction is used in the processor
  //       identification code.
  emit_byte( 0x0F );
  emit_byte( 0xA2 );
}

void Assembler::call(Label& L, relocInfo::relocType rtype) {
  if (L.is_bound()) {
    const int long_size = 5;
    int offs = L.pos() - offset();
    assert(offs <= 0, "assembler error");
    InstructionMark im(this);
    // 1110 1000 #32-bit disp
    emit_byte(0xE8);
    emit_data(offs - long_size, rtype);
  } else {
    InstructionMark im(this);
    // 1110 1000 #32-bit disp
    emit_byte(0xE8);
    // copied from emit_disp(L,type,info)
    Displacement disp(L, Displacement::call, 0);
    L.link_to(offset());
    emit_data(int(disp.data()), rtype);
  }
}


void Assembler::call(address entry, relocInfo::relocType rtype) {
  assert(rtype != relocInfo::virtual_call_type, "must use virtual_call_Relocation::spec");
  assert(entry != NULL, "call most probably wrong");
  InstructionMark im(this);
  emit_byte(0xE8);
  emit_data((int)entry - ((int)_code_pos + sizeof(long)), rtype);
}


void Assembler::call(address entry, RelocationHolder const& rspec) {
  assert(entry != NULL, "call most probably wrong");
  InstructionMark im(this);
  emit_byte(0xE8);
  emit_data((int)entry - ((int)_code_pos + sizeof(long)), rspec);
}


void Assembler::call(Register dst, relocInfo::relocType rtype) {
  relocate(rtype);
  emit_byte(0xFF);
  emit_byte(0xD0 | dst->encoding());
}


void Assembler::call(Address adr) {
  InstructionMark im(this);
  emit_byte(0xFF);
  emit_operand(edx, adr);
}


void Assembler::jmp(address entry, relocInfo::relocType rtype) {
  assert(entry != NULL, "jmp most probably wrong");
  InstructionMark im(this);
  emit_byte(0xE9);
  emit_data((int)entry - ((int)_code_pos + sizeof(long)), rtype);
}


void Assembler::jmp(Register reg, relocInfo::relocType rtype) {
  relocate(rtype);
  emit_byte(0xFF);
  emit_byte(0xE0 | reg->encoding());
}


void Assembler::jmp(Address adr) {
  InstructionMark im(this);
  emit_byte(0xFF);
  emit_operand(esp, adr);
}


void Assembler::jmp(Label& L, relocInfo::relocType rtype) {
  relocate(rtype);

  if (L.is_bound()) {
    const int short_size = 2;
    const int long_size  = 5;
    int offs = L.pos() - offset();
    assert(offs <= 0, "assembler error");
    if (isByte(offs - short_size)) {
      // 1110 1011 #8-bit disp
      emit_byte(0xEB);
      emit_byte((offs - short_size) & 0xFF);
    } else {
      // 1110 1001 #32-bit disp
      emit_byte(0xE9);
      emit_long(offs - long_size);
    }
  } else {
    // 1110 1001 #32-bit disp
    emit_byte(0xE9);
    emit_disp(L, Displacement::absolute_jump, 0);
  }
}

void Assembler::jcc(Condition cc, Label& L, relocInfo::relocType rtype) {
  relocate(rtype);
  assert((0 <= cc) && (cc < 16), "illegal cc");
  if (L.is_bound()) {
    const int short_size = 2;
    const int long_size  = 6;
    int offs = L.pos() - offset();
    assert(offs <= 0, "assembler error");
    if (isByte(offs - short_size)) {
      // 0111 tttn #8-bit disp
      emit_byte(0x70 | cc);
      emit_byte((offs - short_size) & 0xFF);
    } else {
      // 0000 1111 1000 tttn #32-bit disp
      emit_byte(0x0F);
      emit_byte(0x80 | cc);
      emit_long(offs - long_size);
    }
  } else {
    // 0000 1111 1000 tttn #32-bit disp
    // Note: could eliminate cond. jumps to this jump if condition
    //       is the same however, seems to be rather unlikely case.
    emit_byte(0x0F);
    emit_byte(0x80 | cc);
    emit_disp(L, Displacement::conditional_jump, cc);
  }
}

void Assembler::jcc(Condition cc, address dst, relocInfo::relocType rtype) {
  assert((0 <= cc) && (cc < 16), "illegal cc");
  assert(dst != NULL, "jcc most probably wrong");
  // 0000 1111 1000 tttn #32-bit disp
  InstructionMark im(this);
  emit_byte(0x0F);
  emit_byte(0x80 | cc);
  emit_data((int)dst - ((int)_code_pos + sizeof(long)), rtype);
}


// FPU instructions

void Assembler::fld1() {
  emit_byte(0xD9);
  emit_byte(0xE8);
}


void Assembler::fldz() {
  emit_byte(0xD9);
  emit_byte(0xEE);
}


void Assembler::fld_s(Address adr) {
  InstructionMark im(this);
  emit_byte(0xD9);
  emit_operand(eax, adr);
}


void Assembler::fld_s (int index) {
  emit_farith(0xD9, 0xC0, index);
}


void Assembler::fld_d(Address adr) {
  InstructionMark im(this);
  emit_byte(0xDD);
  emit_operand(eax, adr);
}


void Assembler::fld_x(Address adr) {
  InstructionMark im(this);
  emit_byte(0xDB);
  emit_operand(ebp, adr);
}


void Assembler::fst_s(Address adr) {
  InstructionMark im(this);
  emit_byte(0xD9);
  emit_operand(edx, adr);
}


void Assembler::fst_d(Address adr) {
  InstructionMark im(this);
  emit_byte(0xDD);
  emit_operand(edx, adr);
}


void Assembler::fstp_s(Address adr) {
  InstructionMark im(this);
  emit_byte(0xD9);
  emit_operand(ebx, adr);
}


void Assembler::fstp_d(Address adr) {
  InstructionMark im(this);
  emit_byte(0xDD);
  emit_operand(ebx, adr);
}


void Assembler::fstp_x(Address adr) {
  InstructionMark im(this);
  emit_byte(0xDB);
  emit_operand(edi, adr);
}


void Assembler::fstp_d(int index) {
  emit_farith(0xDD, 0xD8, index);
}


void Assembler::fild_s(Address adr) {
  InstructionMark im(this);
  emit_byte(0xDB);
  emit_operand(eax, adr);
}


void Assembler::fild_d(Address adr) {
  InstructionMark im(this);
  emit_byte(0xDF);
  emit_operand(ebp, adr);
}


void Assembler::fistp_s(Address adr) {
  InstructionMark im(this);
  emit_byte(0xDB);
  emit_operand(ebx, adr);
}


void Assembler::fistp_d(Address adr) {
  InstructionMark im(this);
  emit_byte(0xDF);
  emit_operand(edi, adr);
}


void Assembler::fist_s(Address adr) {
  InstructionMark im(this);
  emit_byte(0xDB);
  emit_operand(edx, adr);
}


void Assembler::fabs() {
  emit_byte(0xD9);
  emit_byte(0xE1);
}


void Assembler::fsin() {
  emit_byte(0xD9);
  emit_byte(0xFE);
}


void Assembler::fcos() {
  emit_byte(0xD9);
  emit_byte(0xFF);
}


void Assembler::fsqrt() {
  emit_byte(0xD9);
  emit_byte(0xFA);
}


void Assembler::fchs() {
  emit_byte(0xD9);
  emit_byte(0xE0);
}


void Assembler::fadd_s(Address src) {
  InstructionMark im(this);
  emit_byte(0xD8);
  emit_operand(eax, src);
}


void Assembler::fadd_d(Address src) {
  InstructionMark im(this);
  emit_byte(0xDC);
  emit_operand(eax, src);
}


void Assembler::fadd(int i) {
  emit_farith(0xD8, 0xC0, i);
}


void Assembler::fsub_d(Address src) {
  InstructionMark im(this);
  emit_byte(0xDC);
  emit_operand(esp, src);
}


void Assembler::fsub_s(Address src) {
  InstructionMark im(this);
  emit_byte(0xD8);
  emit_operand(esp, src);
}


void Assembler::fsubr_s(Address src) {
  InstructionMark im(this);
  emit_byte(0xD8);
  emit_operand(ebp, src);
}


void Assembler::fsubr_d(Address src) {
  InstructionMark im(this);
  emit_byte(0xDC);
  emit_operand(ebp, src);
}


void Assembler::fmul_s(Address src) {
  InstructionMark im(this);
  emit_byte(0xD8);
  emit_operand(ecx, src);
}


void Assembler::fmul_d(Address src) {
  InstructionMark im(this);
  emit_byte(0xDC);
  emit_operand(ecx, src);
}


void Assembler::fmul(int i) {
  emit_farith(0xD8, 0xC8, i);
}


void Assembler::fdiv_s(Address src) {
  InstructionMark im(this);
  emit_byte(0xD8);
  emit_operand(esi, src);
}


void Assembler::fdiv_d(Address src) {
  InstructionMark im(this);
  emit_byte(0xDC);
  emit_operand(esi, src);
}


void Assembler::fdivr_s(Address src) {
  InstructionMark im(this);
  emit_byte(0xD8);
  emit_operand(edi, src);
}


void Assembler::fdivr_d(Address src) {
  InstructionMark im(this);
  emit_byte(0xDC);
  emit_operand(edi, src);
}


void Assembler::fsub(int i) {
  emit_farith(0xD8, 0xE0, i);
}


void Assembler::fdiv(int i) {
  emit_farith(0xD8, 0xF0, i);
}


// Note: The Intel manual (Pentium Processor User's Manual, Vol.3, 1994)
//       is erroneous for some of the floating-point instructions below.

void Assembler::fdivp(int i) {
  emit_farith(0xDE, 0xF8, i);                    // ST(0) <- ST(0) / ST(1) and pop (Intel manual wrong)
}


void Assembler::fdivrp(int i) {
  emit_farith(0xDE, 0xF0, i);                    // ST(0) <- ST(1) / ST(0) and pop (Intel manual wrong)
}


void Assembler::fsubp(int i) {
  emit_farith(0xDE, 0xE8, i);                    // ST(0) <- ST(0) - ST(1) and pop (Intel manual wrong)
}


void Assembler::fsubrp(int i) {
  emit_farith(0xDE, 0xE0, i);                    // ST(0) <- ST(1) - ST(0) and pop (Intel manual wrong)
}


void Assembler::faddp(int i) {
  emit_farith(0xDE, 0xC0, i);
}


void Assembler::fmulp(int i) {
  emit_farith(0xDE, 0xC8, i);
}


void Assembler::fprem() {
  emit_byte(0xD9);
  emit_byte(0xF8);
}


void Assembler::fprem1() {
  emit_byte(0xD9);
  emit_byte(0xF5);
}


void Assembler::fxch(int i) {
  emit_farith(0xD9, 0xC8, i);
}


void Assembler::fincstp() {
  emit_byte(0xD9);
  emit_byte(0xF7);
}


void Assembler::fdecstp() {
  emit_byte(0xD9);
  emit_byte(0xF6);
}


void Assembler::ffree(int i) {
  emit_farith(0xDD, 0xC0, i);
}


void Assembler::fcomp_s(Address src) {
  InstructionMark im(this);
  emit_byte(0xD8);
  emit_operand(ebx, src);
}


void Assembler::fcomp_d(Address src) {
  InstructionMark im(this);
  emit_byte(0xDC);
  emit_operand(ebx, src);
}


void Assembler::fcompp() {
  emit_byte(0xDE);
  emit_byte(0xD9);
}


void Assembler::fucomi(int i) {
  // make sure the instruction is supported (introduced for P6, together with cmov)
  guarantee(VM_Version::supports_cmov(), "illegal instruction");
  emit_farith(0xDB, 0xE8, i);
}


void Assembler::fucomip(int i) {
  // make sure the instruction is supported (introduced for P6, together with cmov)
  guarantee(VM_Version::supports_cmov(), "illegal instruction");
  emit_farith(0xDF, 0xE8, i);
}


void Assembler::ftst() {
  emit_byte(0xD9);
  emit_byte(0xE4);
}


void Assembler::fnstsw_ax() {
  emit_byte(0xdF);
  emit_byte(0xE0);
}


void Assembler::fwait() {
  emit_byte(0x9B);
}


void Assembler::finit() {
  emit_byte(0x9B);
  emit_byte(0xDB);
  emit_byte(0xE3);
}


void Assembler::fldcw(Address src) {
  InstructionMark im(this);
  emit_byte(0xd9);
  emit_operand(ebp, src);  
}


void Assembler::fnstcw(Address src) {
  InstructionMark im(this);
  emit_byte(0x9B);
  emit_byte(0xD9);
  emit_operand(edi, src);
}

void Assembler::fnsave(Address dst) {
  emit_byte(0xDD);
  emit_operand(esi, dst);
}


void Assembler::frstor(Address src) {
  emit_byte(0xDD);
  emit_operand(esp, src);
}


void Assembler::fldenv(Address src) {
  emit_byte(0xD9);
  emit_operand(esp, src);
}


void Assembler::sahf() {
  emit_byte(0x9E);
}

// XMM instructions
void Assembler::emit_operand(XMMRegister reg, Address adr) {
  emit_operand((Register)reg, adr._base, adr._index, adr._scale, adr._disp, adr._rspec);
}

void Assembler::movss( XMMRegister dst, Address src ) {
  assert( VM_Version::supports_sse(), "" );
  InstructionMark im(this);
  emit_byte(0xF3); // single
  emit_byte(0x0F);
  emit_byte(0x10); // load
  emit_operand(dst,src);
}

void Assembler::movsd( XMMRegister dst, Address src ) {
  assert( VM_Version::supports_sse2(), "" );
  InstructionMark im(this);
  emit_byte(0xF2); // double
  emit_byte(0x0F);
  emit_byte(0x10); // load
  emit_operand(dst,src);
}

void Assembler::movss( Address dst, XMMRegister src ) {
  assert( VM_Version::supports_sse(), "" );
  InstructionMark im(this);
  emit_byte(0xF3); // single
  emit_byte(0x0F);
  emit_byte(0x11); // store
  emit_operand(src,dst);
}

void Assembler::movsd( Address dst, XMMRegister src ) {
  assert( VM_Version::supports_sse2(), "" );
  InstructionMark im(this);
  emit_byte(0xF2); // double
  emit_byte(0x0F);
  emit_byte(0x11); // store
  emit_operand(src,dst);
}

void Assembler::ldmxcsr( Address src) {
  InstructionMark im(this);
  emit_byte(0x0F);
  emit_byte(0xAE);
  emit_operand(edx /* 2 */, src);
}

void Assembler::stmxcsr( Address dst) {
  InstructionMark im(this);
  emit_byte(0x0F);
  emit_byte(0xAE);
  emit_operand(ebx /* 3 */, dst);
}

// Implementation of MacroAssembler

void MacroAssembler::fat_nop() {
  // A 5 byte nop that is safe for patching (see patch_verified_entry)
  emit_byte(0x26); // es:
  emit_byte(0x2e); // cs:
  emit_byte(0x64); // fs:
  emit_byte(0x65); // gs:
  emit_byte(0x90);
}

void MacroAssembler::null_check(Register reg, int offset) {
  if (needs_explicit_null_check(offset)) {
    // provoke OS NULL exception if reg = NULL by
    // accessing M[reg] w/o changing any (non-CC) registers
    cmpl(eax, Address(reg));
    // Note: should probably use testl(eax, Address(reg));
    //       may be shorter code (however, this version of
    //       testl needs to be implemented first)
  } else {
    // nothing to do, (later) access of M[reg + offset]
    // will provoke OS NULL exception if reg = NULL
  }
}


int MacroAssembler::load_unsigned_byte(Register dst, Address src) {
  // According to Intel Doc. AP-526, "Zero-Extension of Short", p.16,
  // and "3.9 Partial Register Penalties", p. 22).
  int off;
  if (VM_Version::is_P6() || src.uses(dst)) {
    off = offset();
    movzxb(dst, src);
  } else {
    xorl(dst, dst);
    off = offset();
    movb(dst, src);
  }
  return off;
}


int MacroAssembler::load_unsigned_word(Register dst, Address src) {
  // According to Intel Doc. AP-526, "Zero-Extension of Short", p.16,
  // and "3.9 Partial Register Penalties", p. 22).
  int off;
  if (VM_Version::is_P6() || src.uses(dst)) {
    off = offset();
    movzxw(dst, src);
  } else {
    xorl(dst, dst);
    off = offset();
    movw(dst, src);
  }
  return off;
}


int MacroAssembler::load_signed_byte(Register dst, Address src) {
  int off;
  if (VM_Version::is_P6()) {
    off = offset();
    movsxb(dst, src);
  } else {
    off = load_unsigned_byte(dst, src);
    shll(dst, 24);
    sarl(dst, 24);
  }
  return off;
}


int MacroAssembler::load_signed_word(Register dst, Address src) {
  int off;
  if (VM_Version::is_P6()) {
    off = offset();
    movsxw(dst, src);
  } else {
    off = load_unsigned_word(dst, src);
    shll(dst, 16);
    sarl(dst, 16);
  }
  return off;
}


void MacroAssembler::extend_sign(Register hi, Register lo) {
  // According to Intel Doc. AP-526, "Integer Divide", p.18.
  if (VM_Version::is_P6() && hi == edx && lo == eax) {
    cdql();
  } else {
    movl(hi, lo);
    sarl(hi, 31);
  }
}


void MacroAssembler::increment(Register reg, int value) {
  if (value <  0) { decrement(reg, -value); return; }
  if (value == 0) {                       ; return; }
  if (value == 1) { incl(reg)             ; return; }
  /* else */      { addl(reg, value)      ; return; }
}


void MacroAssembler::decrement(Register reg, int value) {
  if (value <  0) { increment(reg, -value); return; }
  if (value == 0) {                       ; return; }
  if (value == 1) { decl(reg)             ; return; }
  /* else */      { subl(reg, value)      ; return; }
}


void MacroAssembler::align(int modulus) {
  while (offset() % modulus != 0) nop();
}


void MacroAssembler::enter() {
  pushl(ebp);
  movl(ebp, esp);
}


void MacroAssembler::leave() {
  movl(esp, ebp);
  popl(ebp);
}


void MacroAssembler::set_last_Java_frame(Register java_thread, 
					 Register last_java_sp,
					 Register last_java_fp,
					 address  last_java_pc) {
  // determine java_thread register
  if (!java_thread->is_valid()) {
    java_thread = edi;
    get_thread(java_thread);
  }
  // determine last_java_sp register
  if (!last_java_sp->is_valid()) {
    last_java_sp = esp;
  }

  // last_java_fp is optional

  if (last_java_fp->is_valid()) {
    movl(Address(java_thread, JavaThread::last_Java_fp_offset()), last_java_fp);
  }

  // last_java_pc is optional

  if (last_java_pc != NULL) {
    movl(Address(java_thread,
                 JavaThread::frame_anchor_offset() + JavaFrameAnchor::last_Java_pc_offset()),
                 (int) last_java_pc);  

  }
#ifdef ASSERT
  { Label L;
    cmpl(Address(java_thread,
		 JavaThread::frame_anchor_offset() + JavaFrameAnchor::flags_offset()),
		 0);
    jcc(Assembler::equal, L);
    stop("MacroAssembler::set_last_Java_frame: flags not cleared");
    bind(L);
  }
#endif /* ASSERT */
  movl(Address(java_thread, JavaThread::last_Java_sp_offset()), last_java_sp);
}

void MacroAssembler::reset_last_Java_frame(Register java_thread, bool clear_pc) {
  // determine java_thread register
  if (!java_thread->is_valid()) {
    java_thread = edi;
    get_thread(java_thread);
  }
  // we must set sp to zero to clear frame
  movl(Address(java_thread, JavaThread::last_Java_sp_offset()), 0);
  // must clear fp, so that compiled frames are not confused; it is possible
  // that we need it only for debugging
  movl(Address(java_thread, JavaThread::last_Java_fp_offset()), 0);

  if (clear_pc)
    movl(Address(java_thread, JavaThread::last_Java_pc_offset()), 0);

  movl(Address(java_thread,
	       JavaThread::frame_anchor_offset() + JavaFrameAnchor::flags_offset()),
	       0);
}



// Implementation of call_VM versions

void MacroAssembler::call_VM_leaf_base(
  address entry_point,
  int     number_of_arguments
) {
  call(entry_point, relocInfo::runtime_call_type);
  increment(esp, number_of_arguments * wordSize);
}


void MacroAssembler::call_VM_base(
  Register oop_result,
  Register java_thread,
  Register last_java_sp,
  address  entry_point,
  int      number_of_arguments,
  bool     check_exceptions
) {
  // determine java_thread register
  if (!java_thread->is_valid()) {
    java_thread = edi;
    get_thread(java_thread);
  }
  // determine last_java_sp register
  if (!last_java_sp->is_valid()) {
    last_java_sp = esp;
  }
  // debugging support
  assert(number_of_arguments >= 0   , "cannot have negative number of arguments");
  assert(java_thread != oop_result  , "cannot use the same register for java_thread & oop_result");
  assert(java_thread != last_java_sp, "cannot use the same register for java_thread & last_java_sp");
  // push java thread (becomes first argument of C function)
  pushl(java_thread);
  // set last Java frame before call
  assert(last_java_sp != ebp, "this code doesn't work for last_java_sp == ebp, which currently can't portably work anyway since C2 doesn't save ebp");
  movl(Address(java_thread, JavaThread::last_Java_fp_offset()), ebp);
#ifdef ASSERT
  { Label L;
    cmpl(Address(java_thread,
		 JavaThread::frame_anchor_offset() + JavaFrameAnchor::flags_offset()),
		 0);
    jcc(Assembler::equal, L);
    stop("MacroAssembler::call_VM_base: flags not cleared");
    bind(L);
  }
#endif /* ASSERT */
  movl(Address(java_thread, JavaThread::last_Java_sp_offset()), last_java_sp);
  // do the call
  call(entry_point, relocInfo::runtime_call_type);
  // restore the thread (cannot use the pushed argument since arguments
  // may be overwritten by C code generated by an optimizing compiler);
  // however can use the register value directly if it is callee saved.
  if (java_thread == edi || java_thread == esi) {
    // edi & esi are callee saved -> nothing to do
#ifdef ASSERT
    guarantee(java_thread != eax, "change this code");
    pushl(eax);
    { Label L;
      get_thread(eax);
      cmpl(java_thread, eax);
      jcc(Assembler::equal, L);
      stop("MacroAssembler::call_VM_base: edi not callee saved?");
      bind(L);
    }
    popl(eax);
#endif
  } else {
    get_thread(java_thread);
  }  
  // reset last Java frame
  // we must set sp to zero to clear frame
  movl(Address(java_thread, JavaThread::last_Java_sp_offset()), 0);
  // must clear fp, so that compiled frames are not confused; it is possible
  // that we need it only for debugging
  movl(Address(java_thread, JavaThread::last_Java_fp_offset()), 0);
  movl(Address(java_thread,
	       JavaThread::frame_anchor_offset() + JavaFrameAnchor::flags_offset()),
	       0);
  // discard thread and arguments
  addl(esp, (1 + number_of_arguments)*wordSize);

  check_and_handle_popframe(java_thread);

  if (check_exceptions) {
    // check for pending exceptions (java_thread is set upon return)
    cmpl(Address(java_thread, Thread::pending_exception_offset()), (int)NULL);
    jcc(Assembler::notEqual, StubRoutines::forward_exception_entry(), relocInfo::runtime_call_type);
  }

  // get oop result if there is one and reset the value in the thread
  if (oop_result->is_valid()) {
    movl(oop_result, Address(java_thread, JavaThread::vm_result_offset()));
    movl(Address(java_thread, JavaThread::vm_result_offset()), (int)NULL);
    verify_oop(oop_result);
  }
}


void MacroAssembler::check_and_handle_popframe(Register java_thread) {
}

void MacroAssembler::call_VM_helper(Register oop_result, address entry_point, int number_of_arguments, bool check_exceptions) {
  leal(eax, Address(esp, (1 + number_of_arguments) * wordSize));
  call_VM_base(oop_result, noreg, eax, entry_point, number_of_arguments, check_exceptions);
}


void MacroAssembler::call_VM(Register oop_result, address entry_point, bool check_exceptions) {
  Label C, E;
  call(C, relocInfo::none);
  jmp(E);

  bind(C);
  call_VM_helper(oop_result, entry_point, 0, check_exceptions);
  ret(0);

  bind(E);
}


void MacroAssembler::call_VM(Register oop_result, address entry_point, Register arg_1, bool check_exceptions) {
  Label C, E;
  call(C, relocInfo::none);
  jmp(E);

  bind(C);
  pushl(arg_1);
  call_VM_helper(oop_result, entry_point, 1, check_exceptions);
  ret(0);

  bind(E);
}


void MacroAssembler::call_VM(Register oop_result, address entry_point, Register arg_1, Register arg_2, bool check_exceptions) {
  Label C, E;
  call(C, relocInfo::none);
  jmp(E);

  bind(C);
  pushl(arg_2);
  pushl(arg_1);
  call_VM_helper(oop_result, entry_point, 2, check_exceptions);
  ret(0);

  bind(E);
}


void MacroAssembler::call_VM(Register oop_result, address entry_point, Register arg_1, Register arg_2, Register arg_3, bool check_exceptions) {
  Label C, E;
  call(C, relocInfo::none);
  jmp(E);

  bind(C);
  pushl(arg_3);
  pushl(arg_2);
  pushl(arg_1);
  call_VM_helper(oop_result, entry_point, 3, check_exceptions);
  ret(0);

  bind(E);
}


void MacroAssembler::call_VM(Register oop_result, Register last_java_sp, address entry_point, int number_of_arguments, bool check_exceptions) {
  call_VM_base(oop_result, noreg, last_java_sp, entry_point, number_of_arguments, check_exceptions);
}


void MacroAssembler::call_VM(Register oop_result, Register last_java_sp, address entry_point, Register arg_1, bool check_exceptions) {
  pushl(arg_1);
  call_VM(oop_result, last_java_sp, entry_point, 1, check_exceptions);
}


void MacroAssembler::call_VM(Register oop_result, Register last_java_sp, address entry_point, Register arg_1, Register arg_2, bool check_exceptions) {
  pushl(arg_2);
  pushl(arg_1);
  call_VM(oop_result, last_java_sp, entry_point, 2, check_exceptions);
}


void MacroAssembler::call_VM(Register oop_result, Register last_java_sp, address entry_point, Register arg_1, Register arg_2, Register arg_3, bool check_exceptions) {
  pushl(arg_3);
  pushl(arg_2);
  pushl(arg_1);
  call_VM(oop_result, last_java_sp, entry_point, 3, check_exceptions);
}


void MacroAssembler::call_VM_leaf(address entry_point, int number_of_arguments) {
  call_VM_leaf_base(entry_point, number_of_arguments);
}


void MacroAssembler::call_VM_leaf(address entry_point, Register arg_1) {
  pushl(arg_1);
  call_VM_leaf(entry_point, 1);
}


void MacroAssembler::call_VM_leaf(address entry_point, Register arg_1, Register arg_2) {
  pushl(arg_2);
  pushl(arg_1);
  call_VM_leaf(entry_point, 2);
}


void MacroAssembler::call_VM_leaf(address entry_point, Register arg_1, Register arg_2, Register arg_3) {
  pushl(arg_3);
  pushl(arg_2);
  pushl(arg_1);
  call_VM_leaf(entry_point, 3);
}


// Calls to C land
//
// When entering C land, the ebp & esp of the last Java frame have to be recorded
// in the (thread-local) JavaThread object. When leaving C land, the last Java fp
// has to be reset to 0. This is required to allow proper stack traversal.

void MacroAssembler::store_check(Register obj) {
  // Does a store check for the oop in register obj. The content of
  // register obj is destroyed afterwards.
  store_check_part_1(obj);
  store_check_part_2(obj);
}


void MacroAssembler::store_check(Register obj, Address dst) {
  store_check(obj);
}


// split the store check operation so that other instructions can be scheduled inbetween
void MacroAssembler::store_check_part_1(Register obj) {
  BarrierSet* bs = Universe::heap()->barrier_set();
  assert(bs->kind() == BarrierSet::CardTableModRef, "Wrong barrier set kind");
  shrl(obj, CardTableModRefBS::card_shift);
}


void MacroAssembler::store_check_part_2(Register obj) {
  BarrierSet* bs = Universe::heap()->barrier_set();
  assert(bs->kind() == BarrierSet::CardTableModRef, "Wrong barrier set kind");
  CardTableModRefBS* ct = (CardTableModRefBS*)bs;
  assert(sizeof(*ct->byte_map_base) == sizeof(jbyte), "adjust this code");
  movb(Address(noreg, obj, Address::times_1, (int)ct->byte_map_base), 0);
}


void MacroAssembler::c2bool(Register x) {
  // implements x == 0 ? 0 : 1
  // note: must only look at least-significant byte of x
  //       since C-style booleans are stored in one byte
  //       only! (was bug)
  andl(x, 0xFF);
  setb(Assembler::notZero, x);
}


int MacroAssembler::corrected_idivl(Register reg) {
  // Full implementation of Java idiv and irem; checks for
  // special case as described in JVM spec., p.243 & p.271.
  // The function returns the (pc) offset of the idivl
  // instruction - may be needed for implicit exceptions.
  //
  //         normal case                           special case
  //
  // input : eax: dividend                         min_int
  //         reg: divisor   (may not be eax/edx)   -1
  //
  // output: eax: quotient  (= eax idiv reg)       min_int
  //         edx: remainder (= eax irem reg)       0
  assert(reg != eax && reg != edx, "reg cannot be eax or edx register");
  const int min_int = 0x80000000;
  Label normal_case, special_case;
  
  // check for special case
  cmpl(eax, min_int);
  jcc(Assembler::notEqual, normal_case);
  xorl(edx, edx); // prepare edx for possible special case (where remainder = 0)
  cmpl(reg, -1);
  jcc(Assembler::equal, special_case);

  // handle normal case
  bind(normal_case);
  cdql();
  int idivl_offset = offset();
  idivl(reg);

  // normal and special case exit
  bind(special_case);

  return idivl_offset;
}


void MacroAssembler::lneg(Register hi, Register lo) {
  negl(lo);
  adcl(hi, 0);
  negl(hi);
}


void MacroAssembler::lmul(int x_esp_offset, int y_esp_offset) {
  // Multiplication of two Java long values stored on the stack
  // as illustrated below. Result is in edx:eax.
  //
  // esp ---> [  ??  ] \               \
  //            ....    | y_esp_offset  |
  //          [ y_lo ] /  (in bytes)    | x_esp_offset
  //          [ y_hi ]                  | (in bytes)
  //            ....                    |
  //          [ x_lo ]                 /
  //          [ x_hi ]
  //            ....
  //
  // Basic idea: lo(result) = lo(x_lo * y_lo)
  //             hi(result) = hi(x_lo * y_lo) + lo(x_hi * y_lo) + lo(x_lo * y_hi)
  Address x_hi(esp, x_esp_offset + wordSize); Address x_lo(esp, x_esp_offset);
  Address y_hi(esp, y_esp_offset + wordSize); Address y_lo(esp, y_esp_offset);
  Label quick;
  // load x_hi, y_hi and check if quick
  // multiplication is possible
  movl(ebx, x_hi);
  movl(ecx, y_hi);
  movl(eax, ebx);
  orl(ebx, ecx);                                 // ebx = 0 <=> x_hi = 0 and y_hi = 0
  jcc(Assembler::zero, quick);                   // if ebx = 0 do quick multiply
  // do full multiplication
  // 1st step
  mull(y_lo);                                    // x_hi * y_lo
  movl(ebx, eax);                                // save lo(x_hi * y_lo) in ebx
  // 2nd step
  movl(eax, x_lo);
  mull(ecx);                                     // x_lo * y_hi
  addl(ebx, eax);                                // add lo(x_lo * y_hi) to ebx
  // 3rd step
  bind(quick);                                   // note: ebx = 0 if quick multiply!
  movl(eax, x_lo);
  mull(y_lo);                                    // x_lo * y_lo
  addl(edx, ebx);                                // correct hi(x_lo * y_lo)
}


void MacroAssembler::lshl(Register hi, Register lo) {
  // Java shift left long support (semantics as described in JVM spec., p.305)
  // (basic idea for shift counts s >= n: x << s == (x << n) << (s - n))
  // shift value is in ecx !
  assert(hi != ecx, "must not use ecx");
  assert(lo != ecx, "must not use ecx");
  const Register s = ecx;                        // shift count
  const int      n = BitsPerWord;
  Label L;
  andl(s, 0x3f);                                 // s := s & 0x3f (s < 0x40)
  cmpl(s, n);                                    // if (s < n)
  jcc(Assembler::less, L);                       // else (s >= n)
  movl(hi, lo);                                  // x := x << n
  xorl(lo, lo);
  // Note: subl(s, n) is not needed since the Intel shift instructions work ecx mod n!
  bind(L);                                       // s (mod n) < n
  shldl(hi, lo);                                 // x := x << s
  shll(lo);
}


void MacroAssembler::lshr(Register hi, Register lo, bool sign_extension) {
  // Java shift right long support (semantics as described in JVM spec., p.306 & p.310)
  // (basic idea for shift counts s >= n: x >> s == (x >> n) >> (s - n))
  assert(hi != ecx, "must not use ecx");
  assert(lo != ecx, "must not use ecx");
  const Register s = ecx;                        // shift count
  const int      n = BitsPerWord;
  Label L;
  andl(s, 0x3f);                                 // s := s & 0x3f (s < 0x40)
  cmpl(s, n);                                    // if (s < n)
  jcc(Assembler::less, L);                       // else (s >= n)
  movl(lo, hi);                                  // x := x >> n
  if (sign_extension) sarl(hi, 31);
  else                xorl(hi, hi);
  // Note: subl(s, n) is not needed since the Intel shift instructions work ecx mod n!
  bind(L);                                       // s (mod n) < n
  shrdl(lo, hi);                                 // x := x >> s
  if (sign_extension) sarl(hi);
  else                shrl(hi);
}


// Note: y_lo will be destroyed
void MacroAssembler::lcmp2int(Register x_hi, Register x_lo, Register y_hi, Register y_lo) {
  // Long compare for Java (semantics as described in JVM spec.)
  Label high, low, done;
	
  cmpl(x_hi, y_hi);
  jcc(Assembler::less, low);
  jcc(Assembler::greater, high);
  // x_hi is the return register
  xorl(x_hi, x_hi);
  cmpl(x_lo, y_lo);
  jcc(Assembler::below, low);
  jcc(Assembler::equal, done);

  bind(high);
  xorl(x_hi, x_hi);
  incl(x_hi);
  jmp(done);

  bind(low);
  xorl(x_hi, x_hi);
  decl(x_hi);

  bind(done);
}


void MacroAssembler::save_eax(Register tmp) {
  if (tmp == noreg) pushl(eax);
  else if (tmp != eax) movl(tmp, eax);
}


void MacroAssembler::restore_eax(Register tmp) {
  if (tmp == noreg) popl(eax);
  else if (tmp != eax) movl(eax, tmp);
}


void MacroAssembler::fremr(Register tmp) {
  save_eax(tmp);
  { Label L;
    bind(L);
    fprem();
    fwait(); fnstsw_ax();
    sahf();
    jcc(Assembler::parity, L);
  }
  restore_eax(tmp);
  // Result is in ST0.
  // Note: fxch & fpop to get rid of ST1
  // (otherwise FPU stack could overflow eventually)
  fxch(1);
  fpop();
}


static const double     pi_4 =  0.7853981633974483;

void MacroAssembler::sincos(bool is_sin, bool preserve_cpu_regs, int num_fpu_regs_in_use) {
  int incoming_argument_and_return_value_offset;

  //  os_breakpoint();

  // A hand-coded argument reduction for values in fabs(pi/4, pi/2)
  // was attempted in this code; unfortunately it appears that the
  // switch to 80-bit precision and back causes this to be
  // unprofitable compared with simply performing a runtime call if
  // the argument is out of the (-pi/4, pi/4) range.

  if (preserve_cpu_regs) {
    // We may destroy ebx in this routine
    pushl(ebx);
  }

  Label slow_case, done;

  // x ?<= pi/4
  fld_d(Address(int(&pi_4), relocInfo::none));
  fld_s(1);                // Stack:  X  PI/4  X
  fabs();                  // Stack: |X| PI/4  X
  fcmp(ebx);
  jcc(Assembler::above, slow_case);
    
  // fastest case: -pi/4 <= x <= pi/4
  is_sin ? fsin() : fcos();
  jmp(done);

  // slow case: runtime call
  bind(slow_case);
  // Preserve registers across runtime call
  pushad();
  if (num_fpu_regs_in_use > 1) {
    // Must preserve all other FPU regs (could alternatively convert
    // SharedRuntime::dsin and dcos into assembly routines known not to trash
    // FPU state, but can not trust C compiler)
    NEEDS_CLEANUP;
    // NOTE that in this case we also push the incoming argument to
    // the stack and restore it later; we also use this stack slot to
    // hold the return value from dsin or dcos.
    for (int i = 0; i < num_fpu_regs_in_use; i++) {
      subl(esp, wordSize*2);
      fstp_d(Address(esp));
    }
    incoming_argument_and_return_value_offset = 2*wordSize*(num_fpu_regs_in_use-1);
    fld_d(Address(esp, incoming_argument_and_return_value_offset));
  }
  subl(esp, wordSize*2);
  fstp_d(Address(esp));
  // NOTE: we must not use call_VM_leaf here because that requires a
  // complete interpreter frame in debug mode -- same bug as 4387334
  NEEDS_CLEANUP;
  // Need to add stack banging before this runtime call if it needs to
  // be taken; however, there is no generic stack banging routine at
  // the MacroAssembler level
  if (is_sin) {
    call( CAST_FROM_FN_PTR(address, SharedRuntime::dsin), relocInfo::runtime_call_type );
  } else {
    call( CAST_FROM_FN_PTR(address, SharedRuntime::dcos), relocInfo::runtime_call_type );
  }
  addl(esp, wordSize * 2);
  if (num_fpu_regs_in_use > 1) {
    // Must save return value to stack and then restore entire FPU stack
    fstp_d(Address(esp, incoming_argument_and_return_value_offset));
    for (int i = 0; i < num_fpu_regs_in_use; i++) {
      fld_d(Address(esp));
      addl(esp, wordSize*2);
    }
  }
  popad();

  // Come here with result in F-TOS
  bind(done);

  if (preserve_cpu_regs) {
    popl(ebx);
  }
}


void MacroAssembler::jC2(Register tmp, Label& L) {
  // set parity bit if FPU flag C2 is set (via eax)
  save_eax(tmp);
  fwait(); fnstsw_ax();
  sahf();
  restore_eax(tmp);
  // branch
  jcc(Assembler::parity, L);
}


void MacroAssembler::jnC2(Register tmp, Label& L) {
  // set parity bit if FPU flag C2 is set (via eax)
  save_eax(tmp);
  fwait(); fnstsw_ax();
  sahf();
  restore_eax(tmp);
  // branch
  jcc(Assembler::noParity, L);
}


void MacroAssembler::fcmp(Register tmp) {
  if (VM_Version::supports_cmov()) {
    fucomip();
    fpop();
  } else {
    fcompp();
    // convert FPU condition into eflags condition via eax
    save_eax(tmp);
    fwait(); fnstsw_ax();
    sahf();
    restore_eax(tmp);
  }
  // condition codes set as follows:
  //
  // CF (corresponds to C0) if x < y
  // PF (corresponds to C2) if unordered
  // ZF (corresponds to C3) if x = y
}


void MacroAssembler::fcmp2int(Register dst, bool unordered_is_less) {
  fcmp(dst);
  Label L;
  if (unordered_is_less) {
    movl(dst, -1);
    jcc(Assembler::parity, L);
    jcc(Assembler::below , L);
    movl(dst, 0);
    jcc(Assembler::equal , L);
    incl(dst);
  } else { // unordered is greater
    movl(dst, 1);
    jcc(Assembler::parity, L);
    jcc(Assembler::above , L);
    movl(dst, 0);
    jcc(Assembler::equal , L);
    decl(dst);
  }
  bind(L);
}


void MacroAssembler::fpop() {
  ffree();
  fincstp();
}


void MacroAssembler::sign_extend_short(Register reg) {
  if (VM_Version::is_P6()) {
    movsxw(reg, reg);
  } else {
    shll(reg, 16);
    sarl(reg, 16);
  }
}


void MacroAssembler::sign_extend_byte(Register reg) {
  if (VM_Version::is_P6() && reg->has_byte_register()) {
    movsxb(reg, reg);
  } else {
    shll(reg, 24);
    sarl(reg, 24);
  }
}


void MacroAssembler::division_with_shift (Register reg, int shift_value) {
  assert (shift_value > 0, "illegal shift value");
  Label _is_positive;
  testl (reg, reg);
  jcc (Assembler::positive, _is_positive);
  int offset = (1 << shift_value) - 1 ;

  if (offset == 1) {
    incl(reg);
  } else {
    addl(reg, offset);
  }

  bind (_is_positive);
  sarl(reg, shift_value);
}


void MacroAssembler::round_to(Register reg, int modulus) {
  addl(reg, modulus - 1);
  andl(reg, -modulus);
}


void MacroAssembler::verify_oop(Register reg, const char* s) {
  if (!VerifyOops) return;
  // Pass register number to verify_oop_subroutine
  char* b = new char[strlen(s) + 50];
  sprintf(b, "verify_oop: %s: %s", reg->name(), s);
  pushl(eax);                          // save eax
  pushl(reg);                          // pass register argument
  pushl((int)b);                       // pass msg argument
  // call indirectly to solve generation ordering problem
  movl(eax, Address((int)StubRoutines::verify_oop_subroutine_entry_address(), relocInfo::none));
  call(eax, relocInfo::none);
}


void MacroAssembler::stop(const char* msg) {
  pushl((int)msg);                                    // push msg
  { Label L; call(L, relocInfo::none); bind(L); }     // push eip
  pushad();                                           // push registers
  call(CAST_FROM_FN_PTR(address, MacroAssembler::debug), relocInfo::runtime_call_type);
  hlt();
}


void MacroAssembler::debug(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax, int eip, char* msg) {
  // In order to get locks to work, we need to fake a in_VM state
  if (ShowMessageBoxOnError) {
    JavaThread* thread = JavaThread::current();
    JavaThreadState saved_state = thread->thread_state();
    thread->set_thread_state(_thread_in_vm);
    ttyLocker ttyl;
    if (CountBytecodes || TraceBytecodes || StopInterpreterAt) {
      BytecodeCounter::print();
    }
    // To see where a verify_oop failed, get $ebx+40/X for this frame.
    // This is the value of eip which points to where verify_oop will return.
    if (os::message_box(msg, "Execution stopped, print registers?")) {
      tty->print_cr("eip = 0x%08x", eip);
      tty->print_cr("eax = 0x%08x", eax);
      tty->print_cr("ebx = 0x%08x", ebx);
      tty->print_cr("ecx = 0x%08x", ecx);
      tty->print_cr("edx = 0x%08x", edx);
      tty->print_cr("edi = 0x%08x", edi);
      tty->print_cr("esi = 0x%08x", esi);
      tty->print_cr("ebp = 0x%08x", ebp);
      tty->print_cr("esp = 0x%08x", esp);
      BREAKPOINT;
    }
    ThreadStateTransition::transition(thread, _thread_in_vm, saved_state);
  } else {
    ::tty->print_cr("=============== DEBUG MESSAGE: %s ================\n", msg);
  }
}



void MacroAssembler::os_breakpoint() {
  // instead of directly emitting a breakpoint, call os:breakpoint for better debugability
  // (e.g., MSVC can't call ps() otherwise)
  call(CAST_FROM_FN_PTR(address, os::breakpoint), relocInfo::runtime_call_type);
}


void MacroAssembler::push_fTOS() {
  subl(esp, 2 * wordSize);
  fstp_d(Address(esp));
}


void MacroAssembler::pop_fTOS() {
  fld_d(Address(esp));
  addl(esp, 2 * wordSize);
}


void MacroAssembler::empty_FPU_stack() {
  for (int i = 8; i-- > 0; ) ffree(i);
}


class ControlWord {
 public:
  int32_t _value;

  int  rounding_control() const        { return  (_value >> 10) & 3      ; }
  int  precision_control() const       { return  (_value >>  8) & 3      ; }
  bool precision() const               { return ((_value >>  5) & 1) != 0; }
  bool underflow() const               { return ((_value >>  4) & 1) != 0; }
  bool overflow() const                { return ((_value >>  3) & 1) != 0; }
  bool zero_divide() const             { return ((_value >>  2) & 1) != 0; }
  bool denormalized() const            { return ((_value >>  1) & 1) != 0; }
  bool invalid() const                 { return ((_value >>  0) & 1) != 0; }

  void print() const {
    // rounding control
    const char* rc;
    switch (rounding_control()) {
      case 0: rc = "round near"; break;
      case 1: rc = "round down"; break;
      case 2: rc = "round up  "; break;     
      case 3: rc = "chop      "; break;
    };
    // precision control
    const char* pc;
    switch (precision_control()) {
      case 0: pc = "24 bits "; break;
      case 1: pc = "reserved"; break;
      case 2: pc = "53 bits "; break;
      case 3: pc = "64 bits "; break;
    };
    // flags
    char f[9];
    f[0] = ' ';
    f[1] = ' ';
    f[2] = (precision   ()) ? 'P' : 'p';
    f[3] = (underflow   ()) ? 'U' : 'u';
    f[4] = (overflow    ()) ? 'O' : 'o';
    f[5] = (zero_divide ()) ? 'Z' : 'z';
    f[6] = (denormalized()) ? 'D' : 'd';
    f[7] = (invalid     ()) ? 'I' : 'i';
    f[8] = '\x0';
    // output
    printf("%04x  masks = %s, %s, %s", _value & 0xFFFF, f, rc, pc);
  }

};


class StatusWord {
 public:
  int32_t _value;

  bool busy() const                    { return ((_value >> 15) & 1) != 0; }
  bool C3() const                      { return ((_value >> 14) & 1) != 0; }
  bool C2() const                      { return ((_value >> 10) & 1) != 0; }
  bool C1() const                      { return ((_value >>  9) & 1) != 0; }
  bool C0() const                      { return ((_value >>  8) & 1) != 0; }
  int  top() const                     { return  (_value >> 11) & 7      ; }
  bool error_status() const            { return ((_value >>  7) & 1) != 0; }
  bool stack_fault() const             { return ((_value >>  6) & 1) != 0; }
  bool precision() const               { return ((_value >>  5) & 1) != 0; }
  bool underflow() const               { return ((_value >>  4) & 1) != 0; }
  bool overflow() const                { return ((_value >>  3) & 1) != 0; }
  bool zero_divide() const             { return ((_value >>  2) & 1) != 0; }
  bool denormalized() const            { return ((_value >>  1) & 1) != 0; }
  bool invalid() const                 { return ((_value >>  0) & 1) != 0; }

  void print() const {
    // condition codes
    char c[5];
    c[0] = (C3()) ? '3' : '-';
    c[1] = (C2()) ? '2' : '-';
    c[2] = (C1()) ? '1' : '-';
    c[3] = (C0()) ? '0' : '-';
    c[4] = '\x0';
    // flags
    char f[9];
    f[0] = (error_status()) ? 'E' : '-';
    f[1] = (stack_fault ()) ? 'S' : '-';
    f[2] = (precision   ()) ? 'P' : '-';
    f[3] = (underflow   ()) ? 'U' : '-';
    f[4] = (overflow    ()) ? 'O' : '-';
    f[5] = (zero_divide ()) ? 'Z' : '-';
    f[6] = (denormalized()) ? 'D' : '-';
    f[7] = (invalid     ()) ? 'I' : '-';
    f[8] = '\x0';
    // output
    printf("%04x  flags = %s, cc =  %s, top = %d", _value & 0xFFFF, f, c, top());
  }

};


class TagWord {
 public:
  int32_t _value;

  int tag_at(int i) const              { return (_value >> (i*2)) & 3; }

  void print() const {
    printf("%04x", _value & 0xFFFF);
  }

};


class FPU_Register {
 public:
  int32_t _m0;
  int32_t _m1;
  int16_t _ex;

  bool is_indefinite() const           { return _ex == -1 && _m1 == 0xC0000000 && _m0 == 0; }

  void print() const {
    char  sign = (_ex < 0) ? '-' : '+';
    const char* kind = (_ex == 0x7FFF || _ex == (int16_t)-1) ? "NaN" : "   ";
    printf("%c%04hx.%08x%08x  %s", sign, _ex, _m1, _m0, kind);
  };

};


class FPU_State {
 public:
  enum {
    register_size       = 10,
    number_of_registers =  8,
    register_mask       =  7
  };

  ControlWord  _control_word;
  StatusWord   _status_word;
  TagWord      _tag_word;
  int32_t      _error_offset;
  int32_t      _error_selector;
  int32_t      _data_offset;
  int32_t      _data_selector;
  int8_t       _register[register_size * number_of_registers];

  int tag_for_st(int i) const          { return _tag_word.tag_at((_status_word.top() + i) & register_mask); }
  FPU_Register* st(int i) const        { return (FPU_Register*)&_register[register_size * i]; }

  const char* tag_as_string(int tag) const {
    switch (tag) {
      case 0: return "valid";
      case 1: return "zero";
      case 2: return "special";
      case 3: return "empty";
    }
    ShouldNotReachHere()
    return NULL;
  }

  void print() const {
    // print computation registers
    { int t = _status_word.top();
      for (int i = 0; i < number_of_registers; i++) {
        int j = (i - t) & register_mask;
        printf("%c r%d = ST%d = ", (j == 0 ? '*' : ' '), i, j);
        st(j)->print();
        printf(" %s\n", tag_as_string(_tag_word.tag_at(i)));
      }
    }
    printf("\n");
    // print control registers
    printf("ctrl = "); _control_word.print(); printf("\n");
    printf("stat = "); _status_word .print(); printf("\n");
    printf("tags = "); _tag_word    .print(); printf("\n");
  }

};


class Flag_Register {
 public:
  int32_t _value;

  bool overflow() const                { return ((_value >> 11) & 1) != 0; }
  bool direction() const               { return ((_value >> 10) & 1) != 0; }
  bool sign() const                    { return ((_value >>  7) & 1) != 0; }
  bool zero() const                    { return ((_value >>  6) & 1) != 0; }
  bool auxiliary_carry() const         { return ((_value >>  4) & 1) != 0; }
  bool parity() const                  { return ((_value >>  2) & 1) != 0; }
  bool carry() const                   { return ((_value >>  0) & 1) != 0; }

  void print() const {
    // flags
    char f[8];
    f[0] = (overflow       ()) ? 'O' : '-';
    f[1] = (direction      ()) ? 'D' : '-';
    f[2] = (sign           ()) ? 'S' : '-';
    f[3] = (zero           ()) ? 'Z' : '-';
    f[4] = (auxiliary_carry()) ? 'A' : '-';
    f[5] = (parity         ()) ? 'P' : '-';
    f[6] = (carry          ()) ? 'C' : '-';
    f[7] = '\x0';
    // output
    printf("%08x  flags = %s", _value, f);
  }

};


class IU_Register {
 public:
  int32_t _value;

  void print() const {
    printf("%08x  %11d", _value, _value);
  }

};


class IU_State {
 public:
  Flag_Register _eflags;
  IU_Register   _edi;
  IU_Register   _esi;
  IU_Register   _ebp;
  IU_Register   _esp;
  IU_Register   _ebx;
  IU_Register   _edx;
  IU_Register   _ecx;
  IU_Register   _eax;

  void print() const {
    // computation registers
    printf("eax  = "); _eax.print(); printf("\n");
    printf("ebx  = "); _ebx.print(); printf("\n");
    printf("ecx  = "); _ecx.print(); printf("\n");
    printf("edx  = "); _edx.print(); printf("\n");
    printf("edi  = "); _edi.print(); printf("\n");
    printf("esi  = "); _esi.print(); printf("\n");
    printf("ebp  = "); _ebp.print(); printf("\n");
    printf("esp  = "); _esp.print(); printf("\n");
    printf("\n");
    // control registers
    printf("flgs = "); _eflags.print(); printf("\n");
  }
};


class CPU_State {
 public:
  FPU_State _fpu_state;
  IU_State  _iu_state;
  
  void print() const {
    printf("--------------------------------------------------\n");
    _iu_state .print();
    printf("\n");
    _fpu_state.print();
    printf("--------------------------------------------------\n");
  }

};


static void _print_CPU_state(CPU_State* state) {
  state->print();
};


void MacroAssembler::print_CPU_state() {
  push_CPU_state();
  pushl(esp);                // pass CPU state
  call(CAST_FROM_FN_PTR(address, _print_CPU_state), relocInfo::runtime_call_type);
  addl(esp, wordSize);       // discard argument
  pop_CPU_state();
}


static bool _verify_FPU(int stack_depth, char* s, CPU_State* state) {
  static int counter = 0;
  FPU_State* fs = &state->_fpu_state;
  counter++;
  // For leaf calls, only verify that the top few elements remain empty.
  // We only need 1 empty at the top for C2 code.
  if( stack_depth < 0 ) {
    if( fs->tag_for_st(7) != 3 ) {
      printf("FPR7 not empty\n");
      state->print();
      return false;
    }
    return true;                // All other stack states do not matter
  }

  // compute stack depth
  int i = 0;
  while (i < FPU_State::number_of_registers && fs->tag_for_st(i)  < 3) i++;
  int d = i;
  while (i < FPU_State::number_of_registers && fs->tag_for_st(i) == 3) i++;
  // verify findings
  if (i != FPU_State::number_of_registers) {
    // stack not contiguous
    printf("%s: stack not contiguous at ST%d\n", s, i);
    state->print();
    return false;
  }
  // check if computed stack depth corresponds to expected stack depth
  if (stack_depth < 0) {
    // expected stack depth is -stack_depth or less
    if (d > -stack_depth) {
      // too many elements on the stack
      printf("%s: <= %d stack elements expected but found %d\n", s, -stack_depth, d);
      state->print();
      return false;
    }
  } else {
    // expected stack depth is stack_depth
    if (d != stack_depth) {
      // wrong stack depth
      printf("%s: %d stack elements expected but found %d\n", s, stack_depth, d);
      state->print();
      return false;
    }
  }
  // check if there's any special values on the stack
  while (i-- > 0) {
    if (fs->tag_for_st(i) == 2) {
      FPU_Register* r = fs->st(i);
      if (fs->st(i)->is_indefinite()) {
        printf("%s: indefinite value found in ST%d\n", s, i);
        state->print();        
        return false;
      }
    }
  }
  // everything is cool
  return true;
}


void MacroAssembler::verify_FPU(int stack_depth, const char* s) {
  if (!VerifyFPU) return;
  push_CPU_state();
  pushl(esp);                // pass CPU state
  pushl((int)s);             // pass message string s
  pushl(stack_depth);        // pass stack depth
  call(CAST_FROM_FN_PTR(address, _verify_FPU), relocInfo::runtime_call_type);
  addl(esp, 3 * wordSize);   // discard arguments
  // check for error
  { Label L;
    testl(eax, eax);
    jcc(Assembler::notZero, L);    
    int3();                  // break if error condition
    bind(L);
  }
  pop_CPU_state();
}


void MacroAssembler::push_IU_state() {
  pushad();
  pushfd();
}


void MacroAssembler::pop_IU_state() {
  popfd();
  popad();
}


void MacroAssembler::push_FPU_state() {
  subl(esp, FPUStateSizeInWords * wordSize);
  fnsave(Address(esp));
  fwait();
}


void MacroAssembler::pop_FPU_state() {
  frstor(Address(esp));
  addl(esp, FPUStateSizeInWords * wordSize);
}


void MacroAssembler::push_CPU_state() {
  push_IU_state();
  push_FPU_state();
}


void MacroAssembler::pop_CPU_state() {
  pop_FPU_state();
  pop_IU_state();
}


void MacroAssembler::push_callee_saved_registers() {
  pushl(esi);
  pushl(edi);
  pushl(edx);
  pushl(ecx);
}


void MacroAssembler::pop_callee_saved_registers() {
  popl(ecx);
  popl(edx);
  popl(edi);
  popl(esi);
}


void MacroAssembler::set_word_if_not_zero(Register dst) {
  xorl(dst, dst);
  set_byte_if_not_zero(dst);
}


void MacroAssembler::verify_tlab() {
#ifdef ASSERT
  if (UseTLAB) {
    Label next, ok;
    Register t1 = esi;
    Register thread_reg = ebx;
    
    pushl(t1);
    pushl(thread_reg);
    get_thread(thread_reg);

    movl(t1, Address(thread_reg, in_bytes(JavaThread::tlab_top_offset())));
    cmpl(t1, Address(thread_reg, in_bytes(JavaThread::tlab_start_offset())));
    jcc(Assembler::aboveEqual, next);
    stop("assert(top >= start)");
    should_not_reach_here();

    bind(next);
    movl(t1, Address(thread_reg, in_bytes(JavaThread::tlab_end_offset())));
    cmpl(t1, Address(thread_reg, in_bytes(JavaThread::tlab_top_offset())));
    jcc(Assembler::aboveEqual, ok);
    stop("assert(top <= end)");
    should_not_reach_here();

    bind(ok);
    popl(thread_reg);
    popl(t1);
  }
#endif 
}


// Defines obj, preserves var_size_in_bytes
void MacroAssembler::eden_allocate(Register obj, Register var_size_in_bytes, int con_size_in_bytes,
                                   Register t1, Label& slow_case) {
  assert(obj == eax, "obj must be in eax for cmpxchg");
  assert_different_registers(obj, var_size_in_bytes, t1);
  Register end = t1;
  Label retry;
  bind(retry);
  movl(obj, Address((int)Universe::heap()->top_addr(), relocInfo::none));
  if (var_size_in_bytes == noreg) {
    leal(end, Address(obj, con_size_in_bytes));
  } else {
    leal(end, Address(obj, var_size_in_bytes, Address::times_1));
  }
  // if end < obj then we wrapped around => object too long => slow case
  cmpl(end, obj);
  jcc(Assembler::below, slow_case);
  cmpl(end, Address((int)Universe::heap()->end_addr(), relocInfo::none));
  jcc(Assembler::above, slow_case);
  // Compare obj with the top addr, and if still equal, store the new top addr in
  // end at the address of the top addr pointer. Sets ZF if was equal, and clears
  // it otherwise. Use lock prefix for atomicity on MPs.
  if (os::is_MP()) {
    lock();
  }
  cmpxchg(end, Address((int)Universe::heap()->top_addr(), relocInfo::none));
  // if someone beat us on the allocation, try again, otherwise continue 
  jcc(Assembler::notEqual, retry);
}


// Defines obj, preserves var_size_in_bytes, okay for t2 == var_size_in_bytes.
void MacroAssembler::tlab_allocate(Register obj, Register var_size_in_bytes, int con_size_in_bytes,
                                   Register t1, Register t2, Label& slow_case) {
  assert_different_registers(obj, t1, t2);
  assert_different_registers(obj, var_size_in_bytes, t1);
  Register end = t2;
  Register thread = t1;

  verify_tlab();

  get_thread(thread);

  movl(obj, Address(thread, JavaThread::tlab_top_offset()));
  if (var_size_in_bytes == noreg) {
    leal(end, Address(obj, con_size_in_bytes));
  } else {
    leal(end, Address(obj, var_size_in_bytes, Address::times_1));
  }
  cmpl(end, Address(thread, JavaThread::tlab_end_offset()));
  jcc(Assembler::above, slow_case);

  // update the tlab top pointer
  movl(Address(thread, JavaThread::tlab_top_offset()), end);

  // recover var_size_in_bytes if necessary
  if (var_size_in_bytes == end) {
    subl(var_size_in_bytes, obj);
  }
  verify_tlab();
}

// Preserves ebx and edx.
void MacroAssembler::tlab_refill(Label& retry, Label& try_eden, Label& slow_case) {
  Register top = eax;
  Register t1  = ecx;
  Register t2  = esi;
  Register thread_reg = edi;
  assert_different_registers(top, thread_reg, t1, t2, /* preserve: */ ebx, edx);
  Label do_refill, discard_tlab;

  if (CMSIncrementalMode) {
    // No allocation in the shared eden.
    jmp(slow_case);
  }

  get_thread(thread_reg);
        
  movl(top, Address(thread_reg, in_bytes(JavaThread::tlab_top_offset())));
  movl(t1,  Address(thread_reg, in_bytes(JavaThread::tlab_end_offset())));

  // calculate amount of free space
  subl(t1, top);
  shrl(t1, LogHeapWordSize);

  // Retain tlab and allocate object in shared space if
  // the amount free in the tlab is too large to discard.
  cmpl(t1, Address(thread_reg, in_bytes(JavaThread::tlab_refill_waste_limit_offset())));
  jcc(Assembler::lessEqual, discard_tlab);

  // Retain
  movl(t2, ThreadLocalAllocBuffer::refill_waste_limit_increment());
  addl(Address(thread_reg, in_bytes(JavaThread::tlab_refill_waste_limit_offset())), t2);
  if (TLABStats) {
    // increment number of slow_allocations
    addl(Address(thread_reg, in_bytes(JavaThread::tlab_slow_allocations_offset())), 1);
  }  
  jmp(try_eden);
 
  bind(discard_tlab);
  if (TLABStats) {
    // increment number of refills
    addl(Address(thread_reg, in_bytes(JavaThread::tlab_number_of_refills_offset())), 1);
    // accumulate wastage -- t1 is amount free in tlab
    addl(Address(thread_reg, in_bytes(JavaThread::tlab_fast_refill_waste_offset())), t1);
  }

  // if tlab is currently allocated (top or end != null) then
  // fill [top, end + alignment_reserve) with array object
  testl (top, top);
  jcc(Assembler::zero, do_refill);

  // set up the mark word
  movl(Address(top, oopDesc::mark_offset_in_bytes()), (int)markOopDesc::prototype()->copy_set_hash(0x2));
  // set the length to the remaining space
  subl(t1, typeArrayOopDesc::header_size(T_INT));
  addl(t1, ThreadLocalAllocBuffer::alignment_reserve());
  shll(t1, log2_intptr(HeapWordSize/sizeof(jint)));
  movl(Address(top, arrayOopDesc::length_offset_in_bytes()), t1);
  // set klass to intArrayKlass
  movl(t1, Address((intptr_t)Universe::intArrayKlassObj_addr(), relocInfo::none));
  movl(Address(top, oopDesc::klass_offset_in_bytes()), t1);

  // refill the tlab with an eden allocation
  bind(do_refill);
  movl(t1, Address(thread_reg, in_bytes(JavaThread::tlab_size_offset())));
  shll(t1, LogHeapWordSize);
  // add object_size ??
  eden_allocate(top, t1, 0, t2, slow_case);

  // Check that t1 was preserved in eden_allocate.
#ifdef ASSERT
  if (UseTLAB) {
    Label ok;
    Register tsize = esi;
    assert_different_registers(tsize, thread_reg, t1);
    pushl(tsize);
    movl(tsize, Address(thread_reg, in_bytes(JavaThread::tlab_size_offset())));
    shll(tsize, LogHeapWordSize);
    cmpl(t1, tsize);
    jcc(Assembler::equal, ok);
    stop("assert(t1 != tlab size)");
    should_not_reach_here();

    bind(ok);
    popl(tsize);
  }
#endif
  movl(Address(thread_reg, in_bytes(JavaThread::tlab_start_offset())), top);
  movl(Address(thread_reg, in_bytes(JavaThread::tlab_top_offset())), top);
  addl(top, t1);
  subl(top, ThreadLocalAllocBuffer::alignment_reserve_in_bytes());
  movl(Address(thread_reg, in_bytes(JavaThread::tlab_end_offset())), top);
  verify_tlab();
  jmp(retry);
}
