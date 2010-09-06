#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)register_ia64.hpp	1.20 03/12/23 16:36:48 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


// Use Register as shortcut
class RegisterImpl;
typedef RegisterImpl* Register;


// The implementation of integer registers for the ia64 architecture.

class RegisterImpl: public AbstractRegisterImpl {
public:
  // constants
  enum { number_of_registers = 128 };  // total number of registers

  // IA64 general register usage from IA-64 Software Conventions and Runtime Architecture Guide p. 5-1..5-4

  // GR0        -> 0
  // GR1        -> gp (global data pointer)
  // GR2,GR3    -> use with 22-bit immediate add
  // GR4-GR7    -> callee saved
  // GR8-GR11   -> procedure return value
  // GR12       -> stack pointer (sp)
  // GR13       -> thread pointer (tp)
  // GR14-GR31  -> caller saved
  // GR32-GR39  -> (in0-in7) input registers; remaining input on the stack
  // GR32-GR127 -> stacked registers, divided into input registers, local registers, and output registers

  // general construction
#define as_Register(encoding) ((Register)(encoding))

  // accessors
  int   encoding() const                    { assert(is_valid(), "invalid register"); return value(); }
  const char* name() const;

  // testers
  bool is_valid() const                     { return 0 <= value() && value() < number_of_registers; }

  bool is_stacked() const                   { return value() >= 32 && value() <= 127; }
  bool is_callee_saved() const              { return value() >= 4 && value() <= 7;    }
  bool is_caller_saved() const              { return !is_stacked() && !is_callee_saved(); }

  // derived registers, offsets, and addresses
  Register successor() const                { return as_Register(encoding() + 1); }
};

// The general registers of the IA64 architecture

const Register gnoreg = as_Register(-1);
const Register noreg  = gnoreg;

const Register GR0    = as_Register(0);
const Register GR1    = as_Register(1);
const Register GR2    = as_Register(2);
const Register GR3    = as_Register(3);
const Register GR4    = as_Register(4);
const Register GR5    = as_Register(5);
const Register GR6    = as_Register(6);
const Register GR7    = as_Register(7);
const Register GR8    = as_Register(8);
const Register GR9    = as_Register(9);
const Register GR10   = as_Register(10);
const Register GR11   = as_Register(11);
const Register GR12   = as_Register(12);
const Register GR13   = as_Register(13);
const Register GR14   = as_Register(14);
const Register GR15   = as_Register(15);
const Register GR16   = as_Register(16);
const Register GR17   = as_Register(17);
const Register GR18   = as_Register(18);
const Register GR19   = as_Register(19);
const Register GR20   = as_Register(20);
const Register GR21   = as_Register(21);
const Register GR22   = as_Register(22);
const Register GR23   = as_Register(23);
const Register GR24   = as_Register(24);
const Register GR25   = as_Register(25);
const Register GR26   = as_Register(26);
const Register GR27   = as_Register(27);
const Register GR28   = as_Register(28);
const Register GR29   = as_Register(29);
const Register GR30   = as_Register(30);
const Register GR31   = as_Register(31);
const Register GR32   = as_Register(32);
const Register GR33   = as_Register(33);
const Register GR34   = as_Register(34);
const Register GR35   = as_Register(35);
const Register GR36   = as_Register(36);
const Register GR37   = as_Register(37);
const Register GR38   = as_Register(38);
const Register GR39   = as_Register(39);
const Register GR40   = as_Register(40);
const Register GR41   = as_Register(41);
const Register GR42   = as_Register(42);
const Register GR43   = as_Register(43);
const Register GR44   = as_Register(44);
const Register GR45   = as_Register(45);
const Register GR46   = as_Register(46);
const Register GR47   = as_Register(47);
const Register GR48   = as_Register(48);
const Register GR49   = as_Register(49);
const Register GR50   = as_Register(50);
const Register GR51   = as_Register(51);
const Register GR52   = as_Register(52);
const Register GR53   = as_Register(53);
const Register GR54   = as_Register(54);
const Register GR55   = as_Register(55);
const Register GR56   = as_Register(56);
const Register GR57   = as_Register(57);
const Register GR58   = as_Register(58);
const Register GR59   = as_Register(59);
const Register GR60   = as_Register(60);
const Register GR61   = as_Register(61);
const Register GR62   = as_Register(62);
const Register GR63   = as_Register(63);
const Register GR64   = as_Register(64);
const Register GR65   = as_Register(65);
const Register GR66   = as_Register(66);
const Register GR67   = as_Register(67);
const Register GR68   = as_Register(68);
const Register GR69   = as_Register(69);
const Register GR70   = as_Register(70);
const Register GR71   = as_Register(71);
const Register GR72   = as_Register(72);
const Register GR73   = as_Register(73);
const Register GR74   = as_Register(74);
const Register GR75   = as_Register(75);
const Register GR76   = as_Register(76);
const Register GR77   = as_Register(77);
const Register GR78   = as_Register(78);
const Register GR79   = as_Register(79);
const Register GR80   = as_Register(80);
const Register GR81   = as_Register(81);
const Register GR82   = as_Register(82);
const Register GR83   = as_Register(83);
const Register GR84   = as_Register(84);
const Register GR85   = as_Register(85);
const Register GR86   = as_Register(86);
const Register GR87   = as_Register(87);
const Register GR88   = as_Register(88);
const Register GR89   = as_Register(89);
const Register GR90   = as_Register(90);
const Register GR91   = as_Register(91);
const Register GR92   = as_Register(92);
const Register GR93   = as_Register(93);
const Register GR94   = as_Register(94);
const Register GR95   = as_Register(95);
const Register GR96   = as_Register(96);
const Register GR97   = as_Register(97);
const Register GR98   = as_Register(98);
const Register GR99   = as_Register(99);
const Register GR100  = as_Register(100);
const Register GR101  = as_Register(101);
const Register GR102  = as_Register(102);
const Register GR103  = as_Register(103);
const Register GR104  = as_Register(104);
const Register GR105  = as_Register(105);
const Register GR106  = as_Register(106);
const Register GR107  = as_Register(107);
const Register GR108  = as_Register(108);
const Register GR109  = as_Register(109);
const Register GR110  = as_Register(110);
const Register GR111  = as_Register(111);
const Register GR112  = as_Register(112);
const Register GR113  = as_Register(113);
const Register GR114  = as_Register(114);
const Register GR115  = as_Register(115);
const Register GR116  = as_Register(116);
const Register GR117  = as_Register(117);
const Register GR118  = as_Register(118);
const Register GR119  = as_Register(119);
const Register GR120  = as_Register(120);
const Register GR121  = as_Register(121);
const Register GR122  = as_Register(122);
const Register GR123  = as_Register(123);
const Register GR124  = as_Register(124);
const Register GR125  = as_Register(125);
const Register GR126  = as_Register(126);
const Register GR127  = as_Register(127);

// The RegisterImplMask class acts as a mask for the
// various registers
class RegisterImplMask {

private:
  // Mask
  uint64_t _mask[2];

public:
  // Initialize to empty
  RegisterImplMask() { clear(); };

  // Set a specific register, but ignore gr0
  void set(uint _reg) {
    if (_reg > 0)
      _mask[_reg >> 6] |= ((uint64_t)1) << (_reg & 63);
  }

  // Clear a specific register
  void clear(uint _reg) {
    _mask[_reg >> 6] &= ~((uint64_t)1) << (_reg & 63);
  }

  // Clear all the registers
  void clear() {
    _mask[0] = 0;
    _mask[1] = 0;
  }

  // Check a set of registers
  bool is_set(RegisterImplMask &_arg) {
    return ((_mask[0] & _arg._mask[0]) | (_mask[1] & _arg._mask[1])) != 0;
  }

  // Check a specific register
  bool is_set(uint _reg) {
    return (_mask[_reg >> 6] >> (_reg & 63)) & 0x1;
  }

  // Check if any register is set
  uint64_t is_set() {
    return _mask[0] | _mask[1];
  }

  // Union with another mask
  void or_mask(RegisterImplMask &_arg) {
    _mask[0] |= _arg._mask[0];
    _mask[1] |= _arg._mask[1];
  }

  // Intersection with another mask
  void and_mask(RegisterImplMask &_arg) {
    _mask[0] &= _arg._mask[0];
    _mask[1] &= _arg._mask[1];
  }

  // Determine if there is an overlap
  bool overlaps(RegisterImplMask &_arg) { return is_set(_arg); }
};


// Use FloatRegister as shortcut
class FloatRegisterImpl;
typedef FloatRegisterImpl* FloatRegister;


// The implementation of float registers for the IA64 architecture

class FloatRegisterImpl: public AbstractRegisterImpl {
public:
  // constants
  enum { number_of_registers = 128 };  // total number of registers

  // IA64 floating register usage from IA-64 Software Conventions and Runtime Architecture Guide p. 5-1..5-4

  // FR0        -> 0.0
  // FR1        -> 1.0
  // FR2-FR5    -> callee saved
  // FR6-FR7    -> caller saved
  // FR8-FR15   -> argument/return values
  // FR16-FR31  -> callee saved
  // FR32-FR127 -> caller saved (and rotating)

  // general construction
#define as_FloatRegister(encoding) ((FloatRegister)(encoding))

  // accessors
  int   encoding() const                              { assert(is_valid(), "invalid register"); return value(); }
  const char* name() const;

  // testers
  bool is_valid() const                               { return 0 <= value() && value() < number_of_registers; }

  bool is_stacked() const                             { return false; }
  bool is_callee_saved() const                        { return value() >= 2 && value() <= 5;    }
  bool is_caller_saved() const                        { return !is_callee_saved(); }


  // derived registers, offsets, and addresses
  FloatRegister successor() const                     { return as_FloatRegister(encoding() + 1); }
};

// The float registers of the IA64 architecture

const FloatRegister fnoreg = as_FloatRegister(-1);

const FloatRegister FR0    = as_FloatRegister(0);
const FloatRegister FR1    = as_FloatRegister(1);
const FloatRegister FR2    = as_FloatRegister(2);
const FloatRegister FR3    = as_FloatRegister(3);
const FloatRegister FR4    = as_FloatRegister(4);
const FloatRegister FR5    = as_FloatRegister(5);
const FloatRegister FR6    = as_FloatRegister(6);
const FloatRegister FR7    = as_FloatRegister(7);
const FloatRegister FR8    = as_FloatRegister(8);
const FloatRegister FR9    = as_FloatRegister(9);
const FloatRegister FR10   = as_FloatRegister(10);
const FloatRegister FR11   = as_FloatRegister(11);
const FloatRegister FR12   = as_FloatRegister(12);
const FloatRegister FR13   = as_FloatRegister(13);
const FloatRegister FR14   = as_FloatRegister(14);
const FloatRegister FR15   = as_FloatRegister(15);
const FloatRegister FR16   = as_FloatRegister(16);
const FloatRegister FR17   = as_FloatRegister(17);
const FloatRegister FR18   = as_FloatRegister(18);
const FloatRegister FR19   = as_FloatRegister(19);
const FloatRegister FR20   = as_FloatRegister(20);
const FloatRegister FR21   = as_FloatRegister(21);
const FloatRegister FR22   = as_FloatRegister(22);
const FloatRegister FR23   = as_FloatRegister(23);
const FloatRegister FR24   = as_FloatRegister(24);
const FloatRegister FR25   = as_FloatRegister(25);
const FloatRegister FR26   = as_FloatRegister(26);
const FloatRegister FR27   = as_FloatRegister(27);
const FloatRegister FR28   = as_FloatRegister(28);
const FloatRegister FR29   = as_FloatRegister(29);
const FloatRegister FR30   = as_FloatRegister(30);
const FloatRegister FR31   = as_FloatRegister(31);
const FloatRegister FR32   = as_FloatRegister(32);
const FloatRegister FR33   = as_FloatRegister(33);
const FloatRegister FR34   = as_FloatRegister(34);
const FloatRegister FR35   = as_FloatRegister(35);
const FloatRegister FR36   = as_FloatRegister(36);
const FloatRegister FR37   = as_FloatRegister(37);
const FloatRegister FR38   = as_FloatRegister(38);
const FloatRegister FR39   = as_FloatRegister(39);
const FloatRegister FR40   = as_FloatRegister(40);
const FloatRegister FR41   = as_FloatRegister(41);
const FloatRegister FR42   = as_FloatRegister(42);
const FloatRegister FR43   = as_FloatRegister(43);
const FloatRegister FR44   = as_FloatRegister(44);
const FloatRegister FR45   = as_FloatRegister(45);
const FloatRegister FR46   = as_FloatRegister(46);
const FloatRegister FR47   = as_FloatRegister(47);
const FloatRegister FR48   = as_FloatRegister(48);
const FloatRegister FR49   = as_FloatRegister(49);
const FloatRegister FR50   = as_FloatRegister(50);
const FloatRegister FR51   = as_FloatRegister(51);
const FloatRegister FR52   = as_FloatRegister(52);
const FloatRegister FR53   = as_FloatRegister(53);
const FloatRegister FR54   = as_FloatRegister(54);
const FloatRegister FR55   = as_FloatRegister(55);
const FloatRegister FR56   = as_FloatRegister(56);
const FloatRegister FR57   = as_FloatRegister(57);
const FloatRegister FR58   = as_FloatRegister(58);
const FloatRegister FR59   = as_FloatRegister(59);
const FloatRegister FR60   = as_FloatRegister(60);
const FloatRegister FR61   = as_FloatRegister(61);
const FloatRegister FR62   = as_FloatRegister(62);
const FloatRegister FR63   = as_FloatRegister(63);
const FloatRegister FR64   = as_FloatRegister(64);
const FloatRegister FR65   = as_FloatRegister(65);
const FloatRegister FR66   = as_FloatRegister(66);
const FloatRegister FR67   = as_FloatRegister(67);
const FloatRegister FR68   = as_FloatRegister(68);
const FloatRegister FR69   = as_FloatRegister(69);
const FloatRegister FR70   = as_FloatRegister(70);
const FloatRegister FR71   = as_FloatRegister(71);
const FloatRegister FR72   = as_FloatRegister(72);
const FloatRegister FR73   = as_FloatRegister(73);
const FloatRegister FR74   = as_FloatRegister(74);
const FloatRegister FR75   = as_FloatRegister(75);
const FloatRegister FR76   = as_FloatRegister(76);
const FloatRegister FR77   = as_FloatRegister(77);
const FloatRegister FR78   = as_FloatRegister(78);
const FloatRegister FR79   = as_FloatRegister(79);
const FloatRegister FR80   = as_FloatRegister(80);
const FloatRegister FR81   = as_FloatRegister(81);
const FloatRegister FR82   = as_FloatRegister(82);
const FloatRegister FR83   = as_FloatRegister(83);
const FloatRegister FR84   = as_FloatRegister(84);
const FloatRegister FR85   = as_FloatRegister(85);
const FloatRegister FR86   = as_FloatRegister(86);
const FloatRegister FR87   = as_FloatRegister(87);
const FloatRegister FR88   = as_FloatRegister(88);
const FloatRegister FR89   = as_FloatRegister(89);
const FloatRegister FR90   = as_FloatRegister(90);
const FloatRegister FR91   = as_FloatRegister(91);
const FloatRegister FR92   = as_FloatRegister(92);
const FloatRegister FR93   = as_FloatRegister(93);
const FloatRegister FR94   = as_FloatRegister(94);
const FloatRegister FR95   = as_FloatRegister(95);
const FloatRegister FR96   = as_FloatRegister(96);
const FloatRegister FR97   = as_FloatRegister(97);
const FloatRegister FR98   = as_FloatRegister(98);
const FloatRegister FR99   = as_FloatRegister(99);
const FloatRegister FR100  = as_FloatRegister(100);
const FloatRegister FR101  = as_FloatRegister(101);
const FloatRegister FR102  = as_FloatRegister(102);
const FloatRegister FR103  = as_FloatRegister(103);
const FloatRegister FR104  = as_FloatRegister(104);
const FloatRegister FR105  = as_FloatRegister(105);
const FloatRegister FR106  = as_FloatRegister(106);
const FloatRegister FR107  = as_FloatRegister(107);
const FloatRegister FR108  = as_FloatRegister(108);
const FloatRegister FR109  = as_FloatRegister(109);
const FloatRegister FR110  = as_FloatRegister(110);
const FloatRegister FR111  = as_FloatRegister(111);
const FloatRegister FR112  = as_FloatRegister(112);
const FloatRegister FR113  = as_FloatRegister(113);
const FloatRegister FR114  = as_FloatRegister(114);
const FloatRegister FR115  = as_FloatRegister(115);
const FloatRegister FR116  = as_FloatRegister(116);
const FloatRegister FR117  = as_FloatRegister(117);
const FloatRegister FR118  = as_FloatRegister(118);
const FloatRegister FR119  = as_FloatRegister(119);
const FloatRegister FR120  = as_FloatRegister(120);
const FloatRegister FR121  = as_FloatRegister(121);
const FloatRegister FR122  = as_FloatRegister(122);
const FloatRegister FR123  = as_FloatRegister(123);
const FloatRegister FR124  = as_FloatRegister(124);
const FloatRegister FR125  = as_FloatRegister(125);
const FloatRegister FR126  = as_FloatRegister(126);
const FloatRegister FR127  = as_FloatRegister(127);

// The FloatRegisterImplMask class acts as a mask for the
// floating point registers
class FloatRegisterImplMask {

private:
  // Mask
  uint64_t _mask[2];

public:
  // Initialize to empty
  FloatRegisterImplMask() { clear(); };

  // Set a specific register, but ignore fr0 and fr1
  void set(uint _reg) {
    if (_reg > 1)
      _mask[_reg >> 6] |= ((uint64_t)1) << (_reg & 63);
  }

  // Clear a specific register
  void clear(uint _reg) {
    _mask[_reg >> 6] &= ~((uint64_t)1) << (_reg & 63);
  }

  // Clear all the registers
  void clear() {
    _mask[0] = 0;
    _mask[1] = 0;
  }

  // Check a set of registers
  bool is_set(FloatRegisterImplMask &_arg) {
    return ((_mask[0] & _arg._mask[0]) | (_mask[1] & _arg._mask[1])) != 0;
  }

  // Check a specific register
  bool is_set(uint _reg) {
    return (_mask[_reg >> 6] >> (_reg & 63)) & 0x1;
  }

  // Check is any register is set
  uint64_t is_set() {
    return _mask[0] | _mask[1];
  }

  // Union with another mask
  void or_mask(FloatRegisterImplMask &_arg) {
    _mask[0] |= _arg._mask[0];
    _mask[1] |= _arg._mask[1];
  }

  // Intersection with another mask
  void and_mask(FloatRegisterImplMask &_arg) {
    _mask[0] &= _arg._mask[0];
    _mask[1] &= _arg._mask[1];
  }

  // Determine if there is an overlap
  bool overlaps(FloatRegisterImplMask &_arg) { return is_set(_arg); }
};


// Use PredicateRegister as shortcut
class PredicateRegisterImpl;
typedef PredicateRegisterImpl* PredicateRegister;

// The implementation of predicate registers for the IA64 architecture

class PredicateRegisterImpl: public AbstractRegisterImpl {
public:
  // constants
  enum { number_of_registers = 64 };  // total number of registers

  // IA64 predicate register usage from IA-64 Software Conventions and Runtime Architecture Guide p. 5-1..5-4

  // PR0        -> 1 (true)
  // PR1-PR5    -> callee saved
  // PR6-PR15   -> caller saved
  // PR16-PR13  -> callee saved (rotating)

  // general construction
#define as_PredicateRegister(encoding) ((PredicateRegister)(encoding))

  // accessors
  int   encoding() const                                      { assert(is_valid(), "invalid register"); return value(); }
  const char* name() const;

  // testers
  bool is_valid() const                                       { return 0 <= value() && value() < number_of_registers; }

  bool is_stacked() const                                     { return false; }
  bool is_callee_saved() const                                { return value() >= 1 && value() <= 5 ||
								       value() >= 16 && value() <= 63; }
  bool is_caller_saved() const                                { return !is_callee_saved(); }

  // derived registers, offsets, and addresses
  PredicateRegister successor() const                         { return as_PredicateRegister(encoding() + 1); }
};

// The predicate registers of the IA64 architecture

const PredicateRegister pnoreg = as_PredicateRegister(-1);

const PredicateRegister PR0    = as_PredicateRegister(0);
const PredicateRegister PR1    = as_PredicateRegister(1);
const PredicateRegister PR2    = as_PredicateRegister(2);
const PredicateRegister PR3    = as_PredicateRegister(3);
const PredicateRegister PR4    = as_PredicateRegister(4);
const PredicateRegister PR5    = as_PredicateRegister(5);
const PredicateRegister PR6    = as_PredicateRegister(6);
const PredicateRegister PR7    = as_PredicateRegister(7);
const PredicateRegister PR8    = as_PredicateRegister(8);
const PredicateRegister PR9    = as_PredicateRegister(9);
const PredicateRegister PR10   = as_PredicateRegister(10);
const PredicateRegister PR11   = as_PredicateRegister(11);
const PredicateRegister PR12   = as_PredicateRegister(12);
const PredicateRegister PR13   = as_PredicateRegister(13);
const PredicateRegister PR14   = as_PredicateRegister(14);
const PredicateRegister PR15   = as_PredicateRegister(15);
const PredicateRegister PR16   = as_PredicateRegister(16);
const PredicateRegister PR17   = as_PredicateRegister(17);
const PredicateRegister PR18   = as_PredicateRegister(18);
const PredicateRegister PR19   = as_PredicateRegister(19);
const PredicateRegister PR20   = as_PredicateRegister(20);
const PredicateRegister PR21   = as_PredicateRegister(21);
const PredicateRegister PR22   = as_PredicateRegister(22);
const PredicateRegister PR23   = as_PredicateRegister(23);
const PredicateRegister PR24   = as_PredicateRegister(24);
const PredicateRegister PR25   = as_PredicateRegister(25);
const PredicateRegister PR26   = as_PredicateRegister(26);
const PredicateRegister PR27   = as_PredicateRegister(27);
const PredicateRegister PR28   = as_PredicateRegister(28);
const PredicateRegister PR29   = as_PredicateRegister(29);
const PredicateRegister PR30   = as_PredicateRegister(30);
const PredicateRegister PR31   = as_PredicateRegister(31);
const PredicateRegister PR32   = as_PredicateRegister(32);
const PredicateRegister PR33   = as_PredicateRegister(33);
const PredicateRegister PR34   = as_PredicateRegister(34);
const PredicateRegister PR35   = as_PredicateRegister(35);
const PredicateRegister PR36   = as_PredicateRegister(36);
const PredicateRegister PR37   = as_PredicateRegister(37);
const PredicateRegister PR38   = as_PredicateRegister(38);
const PredicateRegister PR39   = as_PredicateRegister(39);
const PredicateRegister PR40   = as_PredicateRegister(40);
const PredicateRegister PR41   = as_PredicateRegister(41);
const PredicateRegister PR42   = as_PredicateRegister(42);
const PredicateRegister PR43   = as_PredicateRegister(43);
const PredicateRegister PR44   = as_PredicateRegister(44);
const PredicateRegister PR45   = as_PredicateRegister(45);
const PredicateRegister PR46   = as_PredicateRegister(46);
const PredicateRegister PR47   = as_PredicateRegister(47);
const PredicateRegister PR48   = as_PredicateRegister(48);
const PredicateRegister PR49   = as_PredicateRegister(49);
const PredicateRegister PR50   = as_PredicateRegister(50);
const PredicateRegister PR51   = as_PredicateRegister(51);
const PredicateRegister PR52   = as_PredicateRegister(52);
const PredicateRegister PR53   = as_PredicateRegister(53);
const PredicateRegister PR54   = as_PredicateRegister(54);
const PredicateRegister PR55   = as_PredicateRegister(55);
const PredicateRegister PR56   = as_PredicateRegister(56);
const PredicateRegister PR57   = as_PredicateRegister(57);
const PredicateRegister PR58   = as_PredicateRegister(58);
const PredicateRegister PR59   = as_PredicateRegister(59);
const PredicateRegister PR60   = as_PredicateRegister(60);
const PredicateRegister PR61   = as_PredicateRegister(61);
const PredicateRegister PR62   = as_PredicateRegister(62);
const PredicateRegister PR63   = as_PredicateRegister(63);

// The PredicateRegisterImplMask class acts as a mask for the
// floating point registers
class PredicateRegisterImplMask {

private:
  // Mask
  uint64_t _mask;

public:
  // Initialize to empty
  PredicateRegisterImplMask() { clear(); };

  // Initialize to a set of registers
  PredicateRegisterImplMask(uint64_t mask) : _mask(mask) {}

  // Set a specific register, but ignore PR0
  void set(uint _reg) {
    if (_reg > 0)
      _mask |= ((uint64_t)1) << _reg;
  }

  // Clear a specific register
  void clear(uint _reg) {
    _mask &= ~((uint64_t)1 << _reg);
  }

  // Clear all the registers
  void clear() {
    _mask = 0;
  }

  // Check a set of registers
  bool is_set(PredicateRegisterImplMask &_arg) {
    return (_mask & _arg._mask) != 0;
  }

  // Check a specific register
  bool is_set(uint _reg) {
    return (_mask >> _reg) & 0x1;
  }

  // Check is any register is set
  uint64_t is_set() {
    return _mask;
  }

  // Union with another mask
  void or_mask(PredicateRegisterImplMask &_arg) {
    _mask |= _arg._mask;
  }

  // Intersection with another mask
  void and_mask(PredicateRegisterImplMask &_arg) {
    _mask &= _arg._mask;
  }

  // Determine if there is an overlap
  bool overlaps(PredicateRegisterImplMask &_arg) { return is_set(_arg); }
};


// Use BranchRegister as shortcut
class BranchRegisterImpl;
typedef BranchRegisterImpl* BranchRegister;

// The implementation of branch registers for the IA64 architecture

class BranchRegisterImpl: public AbstractRegisterImpl {
public:
  // constants
  enum { number_of_registers = 8 };  // total number of registers

  // IA64 branch register usage from IA-64 Software Conventions and Runtime Architecture Guide p. 5-1..5-4

  // BR0         -> return link
  // BR1-BR5     -> callee saved
  // BR6-BR7     -> caller saved

  // general construction
#define as_BranchRegister(encoding) ((BranchRegister)(encoding))

  // accessors
  int   encoding() const                                { assert(is_valid(), "invalid register"); return value(); }
  const char* name() const;

  // testers
  bool is_valid() const                                 { return 0 <= value() && value() < number_of_registers; }
  bool is_stacked() const                               { return false; }
  bool is_callee_saved() const                          { return value() >= 1 && value() <= 5;    }
  bool is_caller_saved() const                          { return !is_callee_saved(); }


  // derived registers, offsets, and addresses
  BranchRegister successor() const                      { return as_BranchRegister(encoding() + 1); }
};

// The branch registers of the IA64 architecture

const BranchRegister bnoreg = as_BranchRegister(-1);

const BranchRegister BR0    = as_BranchRegister(0);
const BranchRegister BR1    = as_BranchRegister(1);
const BranchRegister BR2    = as_BranchRegister(2);
const BranchRegister BR3    = as_BranchRegister(3);
const BranchRegister BR4    = as_BranchRegister(4);
const BranchRegister BR5    = as_BranchRegister(5);
const BranchRegister BR6    = as_BranchRegister(6);
const BranchRegister BR7    = as_BranchRegister(7);

// The BranchRegisterImplMask class acts as a mask for the
// floating point registers
class BranchRegisterImplMask {

private:
  // Mask
  uint8_t _mask;

public:
  // Initialize to empty
  BranchRegisterImplMask() { clear(); };

  // Set a specific register
  void set(uint _reg) {
    _mask |= ((uint64_t)1 << _reg);
  }

  // Clear a specific register
  void clear(uint _reg) {
    _mask &= ~((uint64_t)1 << _reg);
  }

  // Clear all the registers
  void clear() {
    _mask = 0;
  }

  // Check a set of registers
  bool is_set(BranchRegisterImplMask &_arg) {
    return (_mask & _arg._mask) != 0;
  }

  // Check a specific register
  bool is_set(uint _reg) {
    return (_mask >> _reg) & 0x1;
  }

  // Check is any register is set
  uint64_t is_set() {
    return _mask;
  }

  // Union with another mask
  void or_mask(BranchRegisterImplMask &_arg) {
    _mask |= _arg._mask;
  }

  // Intersection with another mask
  void and_mask(BranchRegisterImplMask &_arg) {
    _mask &= _arg._mask;
  }

  // Determine if there is an overlap
  bool overlaps(BranchRegisterImplMask &_arg) { return is_set(_arg); }
};


// Use ApplicationRegister as shortcut
class ApplicationRegisterImpl;
typedef ApplicationRegisterImpl* ApplicationRegister;

// The implementation of application registers for the IA64 architecture

class ApplicationRegisterImpl: public AbstractRegisterImpl {
public:
  // constants
  enum { number_of_registers = 128 };  // total number of registers

  // IA64 application register usage from IA-64 Software Conventions and Runtime Architecture Guide p. 5-1..5-4

  // RSC       -> RSE control register
  // BSP       -> backing store pointer for current gr32
  // BSPSTORE  -> backing store stack pointer
  // RNAT      -> RSE NaT collection register
  // CCV       -> caller save: compare-exchange comparison value
  // UNAT      -> callee save: user NaT collection register
  // FPSR      -> floating point status register
  // PFS       -> caller saved: (previous frame state)
  // LC        -> loop count
  // EC        -> epilog count

  // general construction
#define as_ApplicationRegister(encoding) ((ApplicationRegister)(encoding))

  // accessors
  int   encoding() const                                          { assert(is_valid(), "invalid register"); return value(); }
  const char* name() const;

  // testers
  bool is_valid() const                                           { return 0 <= value() && value() < number_of_registers; }

  bool is_stacked() const                                         { return false; }
  bool is_callee_saved() const                                    { return value() == 36 || value() == 65;    }
  bool is_caller_saved() const                                    { return !is_callee_saved(); }

};

// The application registers of the IA64 architecture

const ApplicationRegister anoreg      = as_ApplicationRegister(-1);

const ApplicationRegister AR0         = as_ApplicationRegister(0);
const ApplicationRegister AR_RSC      = as_ApplicationRegister(16);
const ApplicationRegister AR_BSP      = as_ApplicationRegister(17);
const ApplicationRegister AR_BSPSTORE = as_ApplicationRegister(18);
const ApplicationRegister AR_RNAT     = as_ApplicationRegister(19);
const ApplicationRegister AR_CCV      = as_ApplicationRegister(32);
const ApplicationRegister AR_UNAT     = as_ApplicationRegister(36);
const ApplicationRegister AR_FPSR     = as_ApplicationRegister(40);
const ApplicationRegister AR_PFS      = as_ApplicationRegister(64);
const ApplicationRegister AR_LC       = as_ApplicationRegister(65);
const ApplicationRegister AR_EC       = as_ApplicationRegister(66);

// application registers
class ApplicationRegisterImplMask {

private:
  // Mask
  uint64_t _mask[2];

public:
  // Initialize to empty
  ApplicationRegisterImplMask() { clear(); };

  // Set a specific register
  void set(uint _reg) {
    _mask[_reg >> 6] |= ((uint64_t)1) << (_reg & 63);
  }

  // Clear a specific register
  void clear(uint _reg) {
    _mask[_reg >> 6] &= ~((uint64_t)1) << (_reg & 63);
  }

  // Clear all the registers
  void clear() {
    _mask[0] = 0;
    _mask[1] = 0;
  }

  // Check a set of registers
  bool is_set(ApplicationRegisterImplMask &_arg) {
    return ((_mask[0] & _arg._mask[0]) | (_mask[1] & _arg._mask[1])) != 0;
  }

  // Check a specific register
  bool is_set(uint _reg) {
    return (_mask[_reg >> 6] >> (_reg & 63)) & 0x1;
  }

  // Check is any register is set
  uint64_t is_set() {
    return _mask[0] | _mask[1];
  }

  // Union with another mask
  void or_mask(ApplicationRegisterImplMask &_arg) {
    _mask[0] |= _arg._mask[0];
    _mask[1] |= _arg._mask[1];
  }

  // Intersection with another mask
  void and_mask(ApplicationRegisterImplMask &_arg) {
    _mask[0] &= _arg._mask[0];
    _mask[1] &= _arg._mask[1];
  }

  // Determine if there is an overlap
  bool overlaps(ApplicationRegisterImplMask &_arg) { return is_set(_arg); }
};


// This class represents the set of registers for the machine
class RegisterState {

private:
  RegisterImplMask            _gr;
  FloatRegisterImplMask       _fr;
  PredicateRegisterImplMask   _pr;
  BranchRegisterImplMask      _br;
  ApplicationRegisterImplMask _ar;

public:
  RegisterState() { clear(); }

  void clear() { _gr.clear(); _fr.clear(); _pr.clear(); _br.clear(); _ar.clear(); }

  // Set various registers
  void set_gr(uint gr) { _gr.set(gr); }
  void set_fr(uint fr) { _fr.set(fr); }
  void set_pr(uint pr) { _pr.set(pr); }
  void set_br(uint br) { _br.set(br); }
  void set_ar(uint ar) { _ar.set(ar); }

  void set(Register            gr) { set_gr(gr->encoding()); }
  void set(FloatRegister       fr) { set_fr(fr->encoding()); }
  void set(PredicateRegister   pr) { set_pr(pr->encoding()); }
  void set(BranchRegister      br) { set_br(br->encoding()); }
  void set(ApplicationRegister ar) { set_ar(ar->encoding()); }

  // Clear various registers
  void clear_gr(uint gr) { _gr.clear(gr); }
  void clear_fr(uint fr) { _fr.clear(fr); }
  void clear_pr(uint pr) { _pr.clear(pr); }
  void clear_br(uint br) { _br.clear(br); }
  void clear_ar(uint ar) { _ar.clear(ar); }

  void clear(Register            gr) { clear_gr(gr->encoding()); }
  void clear(FloatRegister       fr) { clear_fr(fr->encoding()); }
  void clear(PredicateRegister   pr) { clear_pr(pr->encoding()); }
  void clear(BranchRegister      br) { clear_br(br->encoding()); }
  void clear(ApplicationRegister ar) { clear_ar(ar->encoding()); }

  // Check if any register set
  bool is_set_pr()   { return _pr.is_set(); }

  // Check if register set or clear
  bool is_set_gr(uint gr) { return _gr.is_set(gr); }
  bool is_set_fr(uint fr) { return _fr.is_set(fr); }
  bool is_set_pr(uint pr) { return _pr.is_set(pr); }
  bool is_set_br(uint br) { return _br.is_set(br); }
  bool is_set_ar(uint ar) { return _ar.is_set(ar); }

  bool is_set(Register            gr) { return is_set_gr(gr->encoding()); }
  bool is_set(FloatRegister       fr) { return is_set_fr(fr->encoding()); }
  bool is_set(PredicateRegister   pr) { return is_set_pr(pr->encoding()); }
  bool is_set(BranchRegister      br) { return is_set_br(br->encoding()); }
  bool is_set(ApplicationRegister ar) { return is_set_ar(ar->encoding()); }

  bool is_clear_gr(uint gr) { return !is_set_gr(gr); }
  bool is_clear_fr(uint fr) { return !is_set_fr(fr); }
  bool is_clear_pr(uint pr) { return !is_set_pr(pr); }
  bool is_clear_br(uint br) { return !is_set_br(br); }
  bool is_clear_ar(uint ar) { return !is_set_ar(ar); }

  bool is_clear(Register            gr) { return !is_set(gr); }
  bool is_clear(FloatRegister       fr) { return !is_set(fr); }
  bool is_clear(PredicateRegister   pr) { return !is_set(pr); }
  bool is_clear(BranchRegister      br) { return !is_set(br); }
  bool is_clear(ApplicationRegister ar) { return !is_set(ar); }

  // Union this state with another state
  void or_mask(RegisterState state) {
    _gr.or_mask(state._gr);
    _fr.or_mask(state._fr);
    _pr.or_mask(state._pr);
    _br.or_mask(state._br);
    _ar.or_mask(state._ar);
  }

  void or_mask(PredicateRegisterImplMask& mask) {
    _pr.or_mask(mask);
  }

  // Intersect this state with another state
  void and_mask(RegisterState state) {
    _gr.and_mask(state._gr);
    _fr.and_mask(state._fr);
    _pr.and_mask(state._pr);
    _br.and_mask(state._br);
    _ar.and_mask(state._ar);
  }

  // Determine if this register state Overlaps with a given state
  bool overlaps(RegisterState state, bool omit_predicates=false, bool omit_branches=false) {
    return (_gr.overlaps(state._gr) ||
            _fr.overlaps(state._fr) ||
            (!omit_predicates && _pr.overlaps(state._pr)) ||
            (!omit_branches   && _br.overlaps(state._br)) ||
            _ar.overlaps(state._ar) );
  }
};


// Need to know the total number of registers of all sorts for SharedInfo.
// Define a class that exports it.

class ConcreteRegisterImpl : public AbstractRegisterImpl {
 public:
  enum {
    number_of_registers = RegisterImpl::number_of_registers +
                          FloatRegisterImpl::number_of_registers +
                          PredicateRegisterImpl::number_of_registers +
                          BranchRegisterImpl::number_of_registers +
                          ApplicationRegisterImpl::number_of_registers
  };
};
