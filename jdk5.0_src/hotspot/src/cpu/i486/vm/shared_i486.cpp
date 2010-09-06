#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)shared_i486.cpp	1.7 03/12/23 16:36:26 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

#include "incls/_precompiled.incl"
#include "incls/_shared_i486.cpp.incl"



void SharedInfo::set_regName() {
  regName[eax->encoding()] = "EAX";
  regName[ecx->encoding()] = "ECX";
  regName[ebx->encoding()] = "EBX";
  regName[edx->encoding()] = "EDX";
  regName[edi->encoding()] = "EDI";
  regName[esi->encoding()] = "ESI";
  regName[ebp->encoding()] = "EBP";
  regName[esp->encoding()] = "ESP";
#ifdef COMPILER1
  regName[ 8] = "FPU ST[0]lo";
  regName[ 9] = "FPU ST[0]hi";
  regName[10] = "FPU ST[1]lo";
  regName[11] = "FPU ST[1]hi";
  regName[12] = "FPU ST[2]lo";
  regName[13] = "FPU ST[2]hi";
  regName[14] = "FPU ST[3]lo";
  regName[15] = "FPU ST[3]hi";
  regName[16] = "FPU ST[4]lo";
  regName[17] = "FPU ST[4]hi";
  regName[18] = "FPU ST[5]lo";
  regName[19] = "FPU ST[5]hi";
  regName[20] = "FPU ST[6]lo";
  regName[21] = "FPU ST[6]hi";
  regName[22] = "FPU ST[7]lo";
  regName[23] = "FPU ST[7]hi";
#endif
}

