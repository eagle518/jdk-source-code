/*
 * @(#)FntInstructions.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
	Copyright ©1987-1993 Apple Computer, Inc.  All rights reserved.
*/
#ifndef fntInstructionsDefined
#define fntInstructionsDefined

#include "Fnt.h"

void fnt_SVTCA_0(fnt_LocalGraphicStateType *gs);
void fnt_SVTCA_1(fnt_LocalGraphicStateType *gs);
void fnt_SPVTCA(fnt_LocalGraphicStateType *gs);
void fnt_SFVTCA(fnt_LocalGraphicStateType *gs);
void fnt_SPVTL(fnt_LocalGraphicStateType *gs);
void fnt_SDPVTL(fnt_LocalGraphicStateType *gs);
void fnt_SFVTL(fnt_LocalGraphicStateType *gs);
void fnt_SPVFS(fnt_LocalGraphicStateType *gs);
void fnt_SFVFS(fnt_LocalGraphicStateType *gs);
void fnt_GPV(fnt_LocalGraphicStateType *gs);
void fnt_GFV(fnt_LocalGraphicStateType *gs);
void fnt_SFVTPV(fnt_LocalGraphicStateType *gs);
void fnt_ISECT(fnt_LocalGraphicStateType *gs);
void fnt_SRP0(register fnt_LocalGraphicStateType *gs);
void fnt_SRP1(register fnt_LocalGraphicStateType *gs);
void fnt_SRP2(register fnt_LocalGraphicStateType *gs);
void fnt_SLOOP(register fnt_LocalGraphicStateType *gs);
void fnt_POP(register fnt_LocalGraphicStateType *gs);
void fnt_SetElementPtr(fnt_LocalGraphicStateType *gs);
void fnt_SetRoundState(fnt_LocalGraphicStateType *gs);
void fnt_SROUND(fnt_LocalGraphicStateType *gs);
void fnt_S45ROUND(fnt_LocalGraphicStateType *gs);
void fnt_SMD(fnt_LocalGraphicStateType *gs);
void fnt_RAW(fnt_LocalGraphicStateType *gs);
void fnt_WLSB(fnt_LocalGraphicStateType *gs);
void fnt_SCVTCI(fnt_LocalGraphicStateType *gs);
void fnt_SSWCI(fnt_LocalGraphicStateType *gs);
void fnt_SSW(fnt_LocalGraphicStateType *gs);
void fnt_DUP(fnt_LocalGraphicStateType *gs);
void fnt_POP(fnt_LocalGraphicStateType *gs);
void fnt_CLEAR(fnt_LocalGraphicStateType *gs);
void fnt_SWAP(fnt_LocalGraphicStateType *gs);
void fnt_DEPTH(fnt_LocalGraphicStateType *gs);
void fnt_CINDEX(fnt_LocalGraphicStateType *gs);
void fnt_MINDEX(fnt_LocalGraphicStateType *gs);
void fnt_ROLL( fnt_LocalGraphicStateType* gs );
void fnt_MDAP(fnt_LocalGraphicStateType *gs);
void fnt_MIAP(fnt_LocalGraphicStateType *gs);
void fnt_IUP(fnt_LocalGraphicStateType *gs);
void fnt_SHP(fnt_LocalGraphicStateType *gs);
void fnt_SHC(fnt_LocalGraphicStateType *gs);
void fnt_SHZ(fnt_LocalGraphicStateType *gs);
void fnt_SHPIX(fnt_LocalGraphicStateType *gs);
void fnt_IP(fnt_LocalGraphicStateType *gs);
void fnt_MSIRP(fnt_LocalGraphicStateType *gs);
void fnt_ALIGNRP(fnt_LocalGraphicStateType *gs);
void fnt_MSIRP( fnt_LocalGraphicStateType* gs );
void fnt_ALIGNPTS(fnt_LocalGraphicStateType *gs);
void fnt_SANGW(fnt_LocalGraphicStateType *gs);
void fnt_FLIPPT(fnt_LocalGraphicStateType *gs);
void fnt_FLIPRGON(fnt_LocalGraphicStateType *gs);
void fnt_FLIPRGOFF(fnt_LocalGraphicStateType *gs);
void fnt_SCANCTRL(fnt_LocalGraphicStateType *gs);
void fnt_SCANTYPE(fnt_LocalGraphicStateType *gs);
void fnt_INSTCTRL(fnt_LocalGraphicStateType *gs);
void fnt_AA(fnt_LocalGraphicStateType *gs);
void fnt_NPUSHB(fnt_LocalGraphicStateType *gs);
void fnt_NPUSHW(fnt_LocalGraphicStateType *gs);
void fnt_WS(fnt_LocalGraphicStateType *gs);
void fnt_RS(fnt_LocalGraphicStateType *gs);
void fnt_WCVTP(fnt_LocalGraphicStateType *gs);
void fnt_WCVTF(fnt_LocalGraphicStateType *gs);
void fnt_RCVT(fnt_LocalGraphicStateType *gs);
void fnt_GC(fnt_LocalGraphicStateType *gs);
void fnt_SCFS(fnt_LocalGraphicStateType *gs);
void fnt_MD(fnt_LocalGraphicStateType *gs);
void fnt_MPPEM(fnt_LocalGraphicStateType *gs);
void fnt_MPS(fnt_LocalGraphicStateType *gs);
void fnt_GETINFO(fnt_LocalGraphicStateType* gs);
void fnt_FLIPON(fnt_LocalGraphicStateType *gs);
void fnt_FLIPOFF(fnt_LocalGraphicStateType *gs);
void fnt_DEBUG(fnt_LocalGraphicStateType *gs);
void fnt_SkipPushCrap(fnt_LocalGraphicStateType *gs);
void fnt_IF(fnt_LocalGraphicStateType *gs);
void fnt_ELSE( fnt_LocalGraphicStateType* gs );
void fnt_EIF(fnt_LocalGraphicStateType *gs);
void fnt_JMPR( fnt_LocalGraphicStateType* gs );
void fnt_JROT(fnt_LocalGraphicStateType *gs);
void fnt_JROF(fnt_LocalGraphicStateType *gs);
void fnt_BinaryOperand(fnt_LocalGraphicStateType*);
void fnt_UnaryOperand(fnt_LocalGraphicStateType*);
void fnt_ROUND(fnt_LocalGraphicStateType *gs);
void fnt_NROUND(fnt_LocalGraphicStateType *gs);
void fnt_PUSHB(fnt_LocalGraphicStateType *gs);
void fnt_PUSHB0(fnt_LocalGraphicStateType *gs);
void fnt_PUSHW(fnt_LocalGraphicStateType *gs);
void fnt_PUSHW0(fnt_LocalGraphicStateType *gs);
void fnt_MDRP(fnt_LocalGraphicStateType *gs);
void fnt_MIRP(fnt_LocalGraphicStateType *gs);
void fnt_CALL(fnt_LocalGraphicStateType *gs);
void fnt_FDEF(fnt_LocalGraphicStateType *gs);
void fnt_LOOPCALL(fnt_LocalGraphicStateType *gs);
void fnt_IDefPatch( fnt_LocalGraphicStateType* gs );
void fnt_IDEF( fnt_LocalGraphicStateType* gs );
void fnt_UTP(fnt_LocalGraphicStateType *gs);
void fnt_SDB(fnt_LocalGraphicStateType *gs);
void fnt_SDS(fnt_LocalGraphicStateType *gs);
void fnt_DELTAP1(fnt_LocalGraphicStateType *gs);
void fnt_DELTAP2(fnt_LocalGraphicStateType *gs);
void fnt_DELTAP3(fnt_LocalGraphicStateType *gs);
void fnt_DELTAC1(fnt_LocalGraphicStateType *gs);
void fnt_DELTAC2(fnt_LocalGraphicStateType *gs);
void fnt_DELTAC3(fnt_LocalGraphicStateType *gs);
void fnt_ADJUST( fnt_LocalGraphicStateType *gs );	/* <13> */

/* added in GX 1.0 */
void fnt_GETVARIATION( fnt_LocalGraphicStateType *gs );
void fnt_GETDATA( fnt_LocalGraphicStateType *gs );

#endif
