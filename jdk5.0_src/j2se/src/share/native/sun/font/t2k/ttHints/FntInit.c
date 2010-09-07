/*
 * @(#)FntInit.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
 /*
	Copyright ©1987-1993 Apple Computer, Inc.  All rights reserved.
*/
#include "Hint.h"
#ifdef ENABLE_TT_HINTING


#ifdef  debugging
#include <stdio.h>
#endif

 

#include "FntInstructions.h"
#include "PrivateFnt.h"

void fnt_DefaultJumpTable(register FntFunc function[])
{
	register FntFunc func;
	register fastInt count;
	DebugCode(FntFunc *functionBase = function;)

	/***** 0x00 - 0x0f *****/
	*function++ = (FntFunc) fnt_SVTCA_0;
	*function++ = (FntFunc) fnt_SVTCA_1;
	*function++ = (FntFunc) fnt_SPVTCA;
	*function++ = (FntFunc) fnt_SPVTCA;
	*function++ = (FntFunc) fnt_SFVTCA;
	*function++ = (FntFunc) fnt_SFVTCA;
	*function++ = (FntFunc) fnt_SPVTL;
	*function++ = (FntFunc) fnt_SPVTL;
	*function++ = (FntFunc) fnt_SFVTL;
	*function++ = (FntFunc) fnt_SFVTL;
	*function++ = (FntFunc) fnt_SPVFS;	      /* previously WPV <20> */
	*function++ = (FntFunc) fnt_SFVFS;     	      /* previously WFV <20> */
	*function++ = (FntFunc) fnt_GPV;	     	/* previously RPV <20> */
	*function++ = (FntFunc) fnt_GFV;		/* previously RFV <20> */
	*function++ = (FntFunc) fnt_SFVTPV;
	*function++ = (FntFunc) fnt_ISECT;

	/***** 0x10 - 0x1f *****/
	*function++ = (FntFunc) fnt_SRP0;
	*function++ = (FntFunc) fnt_SRP1;
	*function++ = (FntFunc) fnt_SRP2;
	*function++ = (FntFunc) fnt_SetElementPtr;			/* fnt_SCE0; */
	*function++ = (FntFunc) fnt_SetElementPtr;			/* fnt_SCE1; */
	*function++ = (FntFunc) fnt_SetElementPtr;			/* fnt_SCE2; */
	*function++ = (FntFunc) fnt_SetElementPtr;			/* fnt_SCES; */
	*function++ = (FntFunc) fnt_SLOOP;				/* previously fnt_LLOOP <20> */
	*function++ = (FntFunc) fnt_SetRoundState;	/* fnt_RTG; */
	*function++ = (FntFunc) fnt_SetRoundState;	/* fnt_RTHG; */
	*function++ = (FntFunc) fnt_SMD;		/* fnt_SMD previously fnt_LMD <20> */
	*function++ = (FntFunc) fnt_ELSE;	       	/* used to be fnt_RLSB */
	*function++ = (FntFunc) fnt_JMPR;		/* used to be fnt_WLSB */
	*function++ = (FntFunc) fnt_SCVTCI;		/* previously LWTCI <20> */
	*function++ = (FntFunc) fnt_SSWCI;	       	/* previously LSWCI <20> */
	*function++ = (FntFunc) fnt_SSW;		/* previously LSW <20> */

	/***** 0x20 - 0x2f *****/
	*function++ = (FntFunc) fnt_DUP;
	*function++ = (FntFunc) fnt_POP;
	*function++ = (FntFunc) fnt_CLEAR;
	*function++ = (FntFunc) fnt_SWAP;
	*function++ = (FntFunc) fnt_DEPTH;
	*function++ = (FntFunc) fnt_CINDEX;
	*function++ = (FntFunc) fnt_MINDEX;
	*function++ = (FntFunc) fnt_ALIGNPTS;
	*function++ = (FntFunc) fnt_RAW;
	*function++ = (FntFunc) fnt_UTP;
	*function++ = (FntFunc) fnt_LOOPCALL;
	*function++ = (FntFunc) fnt_CALL;
	*function++ = (FntFunc) fnt_FDEF;
	*function++ = (FntFunc) fnt_IllegalInstruction; /* fnt_ENDF; used for FDEF and IDEF */
	*function++ = (FntFunc) fnt_MDAP;
	*function++ = (FntFunc) fnt_MDAP;


	/***** 0x30 - 0x3f *****/
	*function++ = (FntFunc) fnt_IUP;
	*function++ = (FntFunc) fnt_IUP;
	*function++ = (FntFunc) fnt_SHP;
	*function++ = (FntFunc) fnt_SHP;
	*function++ = (FntFunc) fnt_SHC;
	*function++ = (FntFunc) fnt_SHC;
	*function++ = (FntFunc) fnt_SHZ;						/* previously SHE <20> */
	*function++ = (FntFunc) fnt_SHZ;						/* previously SHE <20> */
	*function++ = (FntFunc) fnt_SHPIX;
	*function++ = (FntFunc) fnt_IP;
	*function++ = (FntFunc) fnt_MSIRP;
	*function++ = (FntFunc) fnt_MSIRP;
	*function++ = (FntFunc) fnt_ALIGNRP;
	*function++ = (FntFunc) fnt_SetRoundState;	/* fnt_RTDG; */
	*function++ = (FntFunc) fnt_MIAP;
	*function++ = (FntFunc) fnt_MIAP;

	/***** 0x40 - 0x4f *****/
	*function++ = (FntFunc) fnt_NPUSHB;
	*function++ = (FntFunc) fnt_NPUSHW;
	*function++ = (FntFunc) fnt_WS;
	*function++ = (FntFunc) fnt_RS;
	*function++ = (FntFunc) fnt_WCVTP;			/* previously WCVT <20> */
	*function++ = (FntFunc) fnt_RCVT;
	*function++ = (FntFunc) fnt_GC;				/* previously RC <20> */
	*function++ = (FntFunc) fnt_GC;				/* previously RC <20> */
	*function++ = (FntFunc) fnt_SCFS;				/* previously WC <20> */
	*function++ = (FntFunc) fnt_MD;
	*function++ = (FntFunc) fnt_MD;
	*function++ = (FntFunc) fnt_MPPEM;
	*function++ = (FntFunc) fnt_MPS;
	*function++ = (FntFunc) fnt_FLIPON;
	*function++ = (FntFunc) fnt_FLIPOFF;
	*function++ = (FntFunc) fnt_DEBUG;

	/***** 0x50 - 0x5f *****/
	*function++ = (FntFunc) fnt_BinaryOperand;	/* fnt_LT; */
	*function++ = (FntFunc) fnt_BinaryOperand;	/* fnt_LTEQ; */
	*function++ = (FntFunc) fnt_BinaryOperand;	/* fnt_GT; */
	*function++ = (FntFunc) fnt_BinaryOperand;	/* fnt_GTEQ; */
	*function++ = (FntFunc) fnt_BinaryOperand;	/* fnt_EQ; */
	*function++ = (FntFunc) fnt_BinaryOperand;	/* fnt_NEQ; */
	*function++ = (FntFunc) fnt_UnaryOperand;		/* fnt_ODD; */
	*function++ = (FntFunc) fnt_UnaryOperand;		/* fnt_EVEN; */
	*function++ = (FntFunc) fnt_IF;
	*function++ = (FntFunc) fnt_EIF;		/* should this guy be an illegal instruction??? */
	*function++ = (FntFunc) fnt_BinaryOperand;	/* fnt_AND; */
	*function++ = (FntFunc) fnt_BinaryOperand;	/* fnt_OR; */
	*function++ = (FntFunc) fnt_UnaryOperand;		/* fnt_NOT; */
	*function++ = (FntFunc) fnt_DELTAP1;
	*function++ = (FntFunc) fnt_SDB;
	*function++ = (FntFunc) fnt_SDS;

	/***** 0x60 - 0x6f *****/
	*function++ = (FntFunc) fnt_BinaryOperand;	/* fnt_ADD; */
	*function++ = (FntFunc) fnt_BinaryOperand;	/* fnt_SUB; */
	*function++ = (FntFunc) fnt_BinaryOperand;	/* fnt_DIV;  */
	*function++ = (FntFunc) fnt_BinaryOperand;	/* fnt_MUL; */
	*function++ = (FntFunc) fnt_UnaryOperand;		/* fnt_ABS; */
	*function++ = (FntFunc) fnt_UnaryOperand;		/* fnt_NEG; */
	*function++ = (FntFunc) fnt_UnaryOperand;		/* fnt_FLOOR; */
	*function++ = (FntFunc) fnt_UnaryOperand;		/* fnt_CEILING */
	*function++ = (FntFunc) fnt_ROUND;
	*function++ = (FntFunc) fnt_ROUND;
	*function++ = (FntFunc) fnt_ROUND;
	*function++ = (FntFunc) fnt_ROUND;
	*function++ = (FntFunc) fnt_NROUND;
	*function++ = (FntFunc) fnt_NROUND;
	*function++ = (FntFunc) fnt_NROUND;
	*function++ = (FntFunc) fnt_NROUND;

	/***** 0x70 - 0x7f *****/
	*function++ = (FntFunc) fnt_WCVTF;			/* previously WCVTFOD <20> */
	*function++ = (FntFunc) fnt_DELTAP2;
	*function++ = (FntFunc) fnt_DELTAP3;
	*function++ = (FntFunc) fnt_DELTAC1;
	*function++ = (FntFunc) fnt_DELTAC2;
	*function++ = (FntFunc) fnt_DELTAC3;
	*function++ = (FntFunc) fnt_SROUND;
	*function++ = (FntFunc) fnt_S45ROUND;
	*function++ = (FntFunc) fnt_JROT;
	*function++ = (FntFunc) fnt_JROF;
 	*function++ = (FntFunc) fnt_SetRoundState;	/* fnt_ROFF; */
	*function++ = (FntFunc) fnt_IllegalInstruction;/* 0x7b reserved for data compression */
	*function++ = (FntFunc) fnt_SetRoundState;	/* fnt_RUTG; */
	*function++ = (FntFunc) fnt_SetRoundState;	/* fnt_RDTG; */
	*function++ = (FntFunc) fnt_SANGW;			/* <14> opcode for obsolete SANGW instruction */
	*function++ = (FntFunc) fnt_AA;				/* <14> opcode for obsolete AA instruction */

	/***** 0x80 - 0x8E *****/
	*function++ = (FntFunc) fnt_FLIPPT;
	*function++ = (FntFunc) fnt_FLIPRGON;
	*function++ = (FntFunc) fnt_FLIPRGOFF;
	*function++ = (FntFunc) fnt_IDefPatch;		/* fnt_RMVT, this space for rent */
	*function++ = (FntFunc) fnt_IDefPatch;		/* fnt_WMVT, this space for rent */
	*function++ = (FntFunc) fnt_SCANCTRL;
	*function++ = (FntFunc) fnt_SDPVTL;
	*function++ = (FntFunc) fnt_SDPVTL;
	*function++ = (FntFunc) fnt_GETINFO;			/* <7> */
	*function++ = (FntFunc) fnt_IDEF;
	*function++ = (FntFunc) fnt_ROLL;				/* previously ROTATE <20> */
	*function++ = (FntFunc) fnt_BinaryOperand;	/* fnt_MAX; */
	*function++ = (FntFunc) fnt_BinaryOperand;	/* fnt_MIN; */
	*function++ = (FntFunc) fnt_SCANTYPE;			/* <7> */
	*function++ = (FntFunc) fnt_INSTCTRL;			/* old<13> */

	/***** 0x8F - 0x90 *****/
	*function++ = (FntFunc) fnt_ADJUST;			/* <15> */
	*function++ = (FntFunc) fnt_ADJUST;			/* <15> */
	*function++ = (FntFunc) fnt_GETVARIATION;		/* 0x91 */
	*function++ = (FntFunc) fnt_GETDATA;			/* 0x92 */

	count = 0xaf - 0x92;
	func = (FntFunc) fnt_IDefPatch;
	do
		*function++ = func;
	while (--count);

	*function++ = (FntFunc) fnt_PUSHB0;
	count = 0xb7 - 0xb1 + 1;
	func = (FntFunc) fnt_PUSHB;
	do
		*function++ = func;
	while (--count);

	*function++ = (FntFunc) fnt_PUSHW0;
	count = 0xbf - 0xb9 + 1;
	func = (FntFunc) fnt_PUSHW;
	do
		*function++ = func;
	while (--count);

	count = 0xdf - 0xc0 + 1;
	func = (FntFunc) fnt_MDRP;
	do
		*function++ = func;
	while (--count);

	count = 0xff - 0xe0 + 1;
	func = (FntFunc) fnt_MIRP;
	do
		*function++ = func;
	while (--count);
	IfDebugMessage(function - functionBase != 256, "bad fnt init", function - functionBase);
}
#endif 
	/* #ifdef ENABLE_TT_HINTING */ 
