/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Inline interpreter functions for ia64

#include <math.h>

inline jfloat cInterpreter::VMfloatAdd(jfloat op1, jfloat op2) { return op1 + op2; }
inline jfloat cInterpreter::VMfloatSub(jfloat op1, jfloat op2) { return op1 - op2; }
inline jfloat cInterpreter::VMfloatMul(jfloat op1, jfloat op2) { return op1 * op2; }
// Windows IA64 Aug 2002 compiler produces bad code for fdiv in debug build.
#if  defined(_M_IA64) && !defined(PRODUCT)
inline jfloat cInterpreter::VMfloatDiv(jfloat op1, jfloat op2) { return (float)((double)op1 / (double)op2); }
#else
inline jfloat cInterpreter::VMfloatDiv(jfloat op1, jfloat op2) { return op1 / op2; }
#endif
inline jfloat cInterpreter::VMfloatRem(jfloat op1, jfloat op2) { return (jfloat)fmod((double)op1, (double)op2); }

inline jfloat cInterpreter::VMfloatNeg(jfloat op) { return -op; }

inline int32_t cInterpreter::VMfloatCompare(jfloat op1, jfloat op2, int32_t direction) {
  return ( op1 < op2 ? -1 :
               op1 > op2 ? 1 :
                   op1 == op2 ? 0 :
                       (direction == -1 || direction == 1) ? direction : 0);

}

inline void cInterpreter::VMmemCopy64(uint32_t to[2], const uint32_t from[2]) {
  // x86 can do unaligned copies but not 64bits at a time
  to[0] = from[0]; to[1] = from[1];
}

// The long operations depend on compiler support for "long long" on x86

inline jlong cInterpreter::VMlongAdd(jlong op1, jlong op2) {
  return op1 + op2;
}

inline jlong cInterpreter::VMlongAnd(jlong op1, jlong op2) {
  return op1 & op2;
}

inline jlong cInterpreter::VMlongDiv(jlong op1, jlong op2) {
  // QQQ what about check and throw...
  if (op1 == min_jlong && op2 == -1) return op1;
  return op1 / op2;
}

inline jlong cInterpreter::VMlongMul(jlong op1, jlong op2) {
  return op1 * op2;
}

inline jlong cInterpreter::VMlongOr(jlong op1, jlong op2) {
  return op1 | op2;
}

inline jlong cInterpreter::VMlongSub(jlong op1, jlong op2) {
  return op1 - op2;
}

inline jlong cInterpreter::VMlongXor(jlong op1, jlong op2) {
  return op1 ^ op2;
}

inline jlong cInterpreter::VMlongRem(jlong op1, jlong op2) {
  if (op1 == min_jlong && op2 == -1) return 0;
  return op1 % op2;
}

inline jlong cInterpreter::VMlongUshr(jlong op1, jint op2) {
  return ((uint64_t) op1) >> (op2 & 0x3F);
}

inline jlong cInterpreter::VMlongShr(jlong op1, jint op2) {
  return op1 >> (op2 & 0x3F);
}

inline jlong cInterpreter::VMlongShl(jlong op1, jint op2) {
  return op1 << (op2 & 0x3F);
}

inline jlong cInterpreter::VMlongNeg(jlong op) {
  return -op;
}

inline jlong cInterpreter::VMlongNot(jlong op) {
  return ~op;
}

inline int32_t cInterpreter::VMlongLtz(jlong op) {
  return (op <= 0);
}

inline int32_t cInterpreter::VMlongGez(jlong op) {
  return (op >= 0);
}

inline int32_t cInterpreter::VMlongEqz(jlong op) {
  return (op == 0);
}

inline int32_t cInterpreter::VMlongEq(jlong op1, jlong op2) {
  return (op1 == op2);
}

inline int32_t cInterpreter::VMlongNe(jlong op1, jlong op2) {
  return (op1 != op2);
}

inline int32_t cInterpreter::VMlongGe(jlong op1, jlong op2) {
  return (op1 >= op2);
}

inline int32_t cInterpreter::VMlongLe(jlong op1, jlong op2) {
  return (op1 <= op2);
}

inline int32_t cInterpreter::VMlongLt(jlong op1, jlong op2) {
  return (op1 < op2);
}

inline int32_t cInterpreter::VMlongGt(jlong op1, jlong op2) {
  return (op1 > op2);
}

inline int32_t cInterpreter::VMlongCompare(jlong op1, jlong op2) {
  return (VMlongLt(op1, op2) ? -1 : VMlongGt(op1, op2) ? 1 : 0);
}

// Long conversions

inline jdouble cInterpreter::VMlong2Double(jlong val) {
  return (jdouble) val;
}

inline jfloat cInterpreter::VMlong2Float(jlong val) {
  return (jfloat) val;
}

inline jint cInterpreter::VMlong2Int(jlong val) {
  return (jint) val;
}

// Double Arithmetic

inline jdouble cInterpreter::VMdoubleAdd(jdouble op1, jdouble op2) {
  return op1 + op2;
}

inline jdouble cInterpreter::VMdoubleDiv(jdouble op1, jdouble op2) {
  // Divide by zero... QQQ
  return op1 / op2;
}

inline jdouble cInterpreter::VMdoubleMul(jdouble op1, jdouble op2) {
  return op1 * op2;
}

inline jdouble cInterpreter::VMdoubleNeg(jdouble op) {
  return -op;
}

inline jdouble cInterpreter::VMdoubleRem(jdouble op1, jdouble op2) {
  return fmod(op1, op2);
}

inline jdouble cInterpreter::VMdoubleSub(jdouble op1, jdouble op2) {
  return op1 - op2;
}

inline int32_t cInterpreter::VMdoubleCompare(jdouble op1, jdouble op2, int32_t direction) {
  return ( op1 < op2 ? -1 :
               op1 > op2 ? 1 :
                   op1 == op2 ? 0 :
                       (direction == -1 || direction == 1) ? direction : 0);
}

// Double Conversions

inline jfloat cInterpreter::VMdouble2Float(jdouble val) {
  return (jfloat) val;
}

// Float Conversions

inline jdouble cInterpreter::VMfloat2Double(jfloat op) {
#ifdef IA64
  // IA64 gcc bug
  return ((op == 0.0f) ? (jdouble)op : (jdouble)op + ia64_double_zero); // Addition works around gcc bug
#else
  return ((jdouble)op);
#endif
}

// Integer Arithmetic

inline jint cInterpreter::VMintAdd(jint op1, jint op2) {
  return op1 + op2;
}

inline jint cInterpreter::VMintAnd(jint op1, jint op2) {
  return op1 & op2;
}

inline jint cInterpreter::VMintDiv(jint op1, jint op2) {
  /* it's possible we could catch this special case implicitly */
  if (op1 == (jint)0x80000000 && op2 == -1) return op1;
  else return op1 / op2;
}

inline jint cInterpreter::VMintMul(jint op1, jint op2) {
  return op1 * op2;
}

inline jint cInterpreter::VMintNeg(jint op) {
  return -op;
}

inline jint cInterpreter::VMintOr(jint op1, jint op2) {
  return op1 | op2;
}

inline jint cInterpreter::VMintRem(jint op1, jint op2) {
  /* it's possible we could catch this special case implicitly */
  if (op1 == (jint)0x80000000 && op2 == -1) return 0;
  else return op1 % op2;
}

inline jint cInterpreter::VMintShl(jint op1, jint op2) {
  return op1 <<  (op2 & 0x1f);
}

inline jint cInterpreter::VMintShr(jint op1, jint op2) {
  return op1 >>  (op2 & 0x1f);
}

inline jint cInterpreter::VMintSub(jint op1, jint op2) {
  return op1 - op2;
}

inline jint cInterpreter::VMintUshr(jint op1, jint op2) {
  return ((juint) op1) >> (op2 & 0x1f);
}

inline jint cInterpreter::VMintXor(jint op1, jint op2) {
  return op1 ^ op2;
}

inline jdouble cInterpreter::VMint2Double(jint val) {
  return (jdouble) val;
}

inline jfloat cInterpreter::VMint2Float(jint val) {
  return (jfloat) val;
}

inline jlong cInterpreter::VMint2Long(jint val) {
  return (jlong) val;
}

inline jchar cInterpreter::VMint2Char(jint val) {
  return (jchar) val;
}

inline jshort cInterpreter::VMint2Short(jint val) {
  return (jshort) val;
}

inline jbyte cInterpreter::VMint2Byte(jint val) {
  return (jbyte) val;
}
