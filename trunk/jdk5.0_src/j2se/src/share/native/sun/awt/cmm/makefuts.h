/*
 * @(#)makefuts.h	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	makefuts.h

	Contains:	header file for making ICM specific futs

	Written by:	Poe

	COPYRIGHT (c) 1991-2000 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */


#ifndef _MAKEFUTS_H_
#define _MAKEFUTS_H_ 1

#include <math.h>
#include "fut.h"
#include "fut_util.h"
#include "kcmptlib.h"

/* These constants are used in logrgb.c and loguvl.c to shape shadows by adding `flare' */

#define	FLARE_YMIN		0.00392156862745	/* Y_min = 1/255, a la DM8000 */
#define FLARE_YFLARE	0.00393700787402	/* Y_min/(1 + Y_min) */
#define	FLARE_DMAX		2.40654018043395	/* D_max = -(log Y_min) */
#define	FLARE_TANPT		0.01065992873906	/* eY_min */
#define	FLARE_MAPPT		0.18046425546277	/* (log e)/D_max */
#define	FLARE_SLOPE		16.9292178100213	/* MAPPT/TANPT */

/* This constant was define in the non-ansi c compilers math.h.  It does not
 * seem to be defined by any of the compilers that we are using */

#define M_LN10				2.30258509299404568402

	/*
		These scale factors define the neutral point for the Lab8 and Lab16 color spaces.
	*/
#define KCP_LAB8_NEUTRAL (0.50196078431)	/* = 128.0/255.0 */
#define KCP_LAB16_NEUTRAL (0.50000762951)	/* = 32768.0/65535.0 */
#define KCP_LINLAB_GRID_SIZE (16)

#define KCP_LAB8_L_100 (0xffff)
#define KCP_LAB8_AB_NEUTRAL (0x8080)
#define KCP_LAB16_L_100 (0xff00)
#define KCP_LAB16_AB_NEUTRAL (0x8000)

/* these factors are used to convert between 8-bit and 16-bit encodings of uvL, Lab, etc. */
#define KCP_8_TO_16_ENCODING ((double) KCP_LAB16_L_100 / KCP_LAB8_L_100)
#define KCP_16_TO_8_ENCODING ((double) KCP_LAB8_L_100 / KCP_LAB16_L_100)

/* inverse gamma and offset for Rec 709 transfer function */
#define KCP_709_INV_GAMMA (0.45)	/* inverse gamma */
#define KCP_709_OFFSET (0.099)		/* offset */

/* D50 white point */
#define	KCP_D50_X	(0.9642)
#define	KCP_D50_Y	(1.0000)
#define	KCP_D50_Z	(0.8249)

	/* restricts t to interval [low, high] */
#define RESTRICT(t, low, high) (MAX ((low), MIN ((high), (t))))

	/* quantizes t to nearest (short) integer in given scale */
#define QUANT(t, scale)	((short)((double)(scale) * RESTRICT((t), 0.0, 1.0) + 0.5))

	/* quantizes t to nearest (mf2_tbldat_t) integer in given scale */
#define QUANT_MF2(t, scale)	((mf2_tbldat_t)((double)(scale) * RESTRICT((t), 0.0, 1.0) + 0.5))

	/* quantizes t to nearest integer in given scale */
	/* seperate the lines to fix floating point precision error on 68k */
#define QUANT1(t, scale)	\
			t = (double)RESTRICT((t), 0.0, 1.0); \
			t *= (double)scale; \
			t += (double)0.5

	/* (approximate) inverse of QUANT() */
#define DEQUANT(t, scale) ((double)(t)/(double)(scale))

#define	POW(x, power)	( ((x) > 0.0) ? exp ((power) * log ((x))) : pow ((x), (power)) )
#define	ANTILOG(x)	( exp ((x) * M_LN10) )

	/* constants for CIE visual-response function */
typedef struct lensityConst_s {
	double	gamma;		/* "forward" exponent (p) */
	double	inv_gamma;	/* "backward" exponent (1/p) */
	double	offset;		/* offset term (A) */
	double	x_tan;		/* x-coordinate of tangency point (x_b) */
	double	y_tan;		/* y-coordinate of tangency point (f(x_b)) */
	double	slope;		/* "forward" slope of tangent (B) */
	double	inv_slope;	/* "backward" slope of tangent (1/B) */
} lensityConst_t, *lensityConst_p;

	/* constants for uvL<->Lab conversions */
typedef struct uvLLabConst_s {
	double	bu0;		/* "forward" exponent (p) */
	double	bu1;		/* "backward" exponent (1/p) */
	double	bv0;		/* offset term (A) */
	double	bv1;		/* x-coordinate of tangency point (x_b) */
} uvLLabConst_t, *uvLLabConst_p;

	/* constants for Lab<->uvL conversions */
typedef struct LabuvLConst_s {
	double	u_angle_neutral;	/* arctan u'_n */
	double	v_angle_neutral;	/* arctan v'_n */
	double	u_neutral;			/* u'_n */
	double	v_neutral;			/* v'_n */
} LabuvLConst_t, *LabuvLConst_p;

typedef struct auxData_s {
	fut_calcData_t	std;
	lensityConst_t	lc;
	uvLLabConst_t	uvLLabC;
	LabuvLConst_t	LabuvLC;
} auxData_t, FAR* auxData_p;


#define NUMFINE 25

typedef struct xfer_s {			/* transfer-table object */
	double		nonlinear[NUMFINE];	/* nonlinear (e.g., device) coordinates */
	double		linear[NUMFINE];	/* linear (e.g., radiant flux) coordinates */
	double_p	from;				/* source selector */
	double_p	to;					/* destination selector */
} xfer_t, FAR* xfer_p;

#define ALLOC(number,size)	(allocBufferPtr((KpInt32_t)((number)*(size))))
#define	DALLOC(ptr)	(freeBufferPtr((KpChar_p)(ptr)), (ptr) = NULL)	/* deallocate memory */

#define	SLOPE_COUNT 128		/* limit the slope if less than SLOPE_COUNT */
#define	SLOPE_LIMIT 16		/* limit the slope to SLOPE_LIMIT */

#define	SCALEFIXED	65536.0			/* 2^16 */
#define	SCALEDOT16	65536.0			/* 2^16 */
#define	SCALEDOT15	32768.0			/* 2^15 */
#define	SCALEDOT8	  256.0			/* 2^8  */

	/*	This scale factor is applied to the X, Y, and Z values prior to
		the 12-bit quantization so that, when shifted left 4 bits,
		it will produce an unsigned 16-bit number with one integer bit,
		as per the ICC profile format.  It is *approximately* 
		1/2:  the exact number is 2^15/(4080 << 4), since 1.0 is represented 
		as 2^15 and FUT_MAX_PEL12 (= 4080) is the quantization scale factor.
	*/
#define	XYZSCALE_V0	0.50196078431373	/* = 2^15 / (FUT_MAX_PEL12 << 4) */

/* but now that's gone */
#define	XYZSCALE	0.5	/* = 2^15 / 2^16 */

typedef struct tagMATRIXDATA {
	KpUInt16_t			dim;			/* dimension (= 3) */
	double_h			matrix;			/* i.e., matrix[dim][dim] */
	ResponseRecord_h	inResponse;		/* i.e., response[dim] */
	ResponseRecord_h	outResponse;	/* i.e., response[dim] */
} MATRIXDATA, FAR* LPMATRIXDATA;

#define RRECORD_DATA_SIZE 65536


PTErr_t makeForwardXformMono (ResponseRecord_p, fut_p);
PTErr_t makeInverseXformMono (ResponseRecord_p, fut_p);
double otblFunc (double, fut_calcData_p);
PTErr_t makeOutputMatrixXform (Fixed_p matrix, KpUInt32_t gridsize, fut_p *theFut);
PTErr_t makeFutFromMatrix (Fixed_p matrix, ResponseRecord_p inRedTRC, ResponseRecord_p inGreenTRC,
							ResponseRecord_p inBlueTRC, ResponseRecord_p outRedTRC, 
							ResponseRecord_p outGreenTRC, ResponseRecord_p outBlueTRC, 
							KpUInt32_t gridsize, PTDataClass_t iClass, PTDataClass_t oClass, fut_p *theFut);
PTErr_t makeForwardXformFromMatrix (LPMATRIXDATA, KpUInt32_t, KpInt32_p, fut_p);
PTErr_t makeInverseXformFromMatrix (LPMATRIXDATA, KpUInt32_t, KpInt32_p, fut_p);
void makeMonotonic (KpUInt32_t count, mf2_tbldat_p table);
void makeInverseMonotonic (KpUInt32_t count, mf2_tbldat_p table);
void makeCurveFromPara (KpUInt16_t, Fixed_p, mab_tbldat_p, KpInt32_t);
KpInt32_t getNumParaParams(KpUInt16_t);

double f4l (double x, double xtab[], double ytab[], KpInt32_t n, KpInt32_p hint);
PTErr_t calcItblN (mf2_tbldat_p table, KpInt32_t tableSize, ResponseRecord_p rrp, KpUInt32_t interpMode);
void calcGtbl3 (mf2_tbldat_p*, KpInt32_p, double_h, double_p);
void calcOtbl0 (mf2_tbldat_p);
void calcOtbl1 (mf2_tbldat_p, double);
PTErr_t calcOtblN (mf2_tbldat_p table, ResponseRecord_p rrp, KpUInt32_t interpMode);
PTErr_t calcOtblL1 (mf2_tbldat_p, double);
PTErr_t calcOtblLN (mf2_tbldat_p, ResponseRecord_p);
PTErr_t calcOtblLS1 (mf2_tbldat_p, double);
PTErr_t calcOtblLSN (mf2_tbldat_p, ResponseRecord_p);
double calcInvertTRC (double p, mf2_tbldat_p data, KpUInt32_t length);
KpInt32_t solvemat (KpInt32_t, double_h, double_p);
PTErr_t init_xfer (xfer_p, ResponseRecord_p);
PTErr_t set_xfer (xfer_p, KpInt32_t, KpInt32_t);
double xfer (xfer_p, double, KpInt32_p);
fut_p get_lab2xyz (KpInt32_t);
fut_p get_xyz2lab (KpInt32_t);
fut_p get_linlab_fut (KpInt32_t, PTDataClass_t, PTDataClass_t);
fut_p get_idenMonCurv_fut (KpInt32_t, double, double);

double Hfunc (double arg, lensityConst_p lc);
double Hinverse (double arg, lensityConst_p lc);
void lensityInit (lensityConst_p c);
void uvLLabInit (uvLLabConst_p c);
void LabuvLInit (LabuvLConst_p c);

#define MAX_AUX_FILENAME	(12)
#define MAX_AUX_PATHNAME	(256)

/* Functions defined in fxnull.c
	They are used to create the fxnull (CP04) PT */

double			fxnull_iFunc_x (double, fut_calcData_p); 
double			fxnull_iFunc_y (double, fut_calcData_p); 
double			fxnull_iFunc_z (double, fut_calcData_p); 
double			fxnull_oFunc_x (double, fut_calcData_p);
double			fxnull_oFunc_y (double, fut_calcData_p);
double			fxnull_oFunc_z (double, fut_calcData_p);

/* Functions defined in logrgb.c
	They are used to create the logrgb (CP07) PT */

double			logrgb_iFunc (double, fut_calcData_p); 
double			logrgb_oFunc (double, fut_calcData_p);

/* Functions defined in loguvl.c
	They are used to create the loguvl (CP08) PT */

double			loguvl_iFunc_x (double, fut_calcData_p); 
double			loguvl_iFunc_y (double, fut_calcData_p); 
double			loguvl_iFunc_z (double, fut_calcData_p); 
double			loguvl_oFunc_x (double, fut_calcData_p);
double			loguvl_oFunc_y (double, fut_calcData_p);
double			loguvl_oFunc_z (double, fut_calcData_p);

/* Functions defined in cmyklin.c
	They are used to create the cmyklin (CP10i) and the 
	cmyklin invert (CP10) PTs */

double			cmyklin_iFunc (double, fut_calcData_p); 
double			cmyklini_iFunc (double, fut_calcData_p); 
double			cmyklin_oFunc (double, fut_calcData_p);
double			cmyklini_oFunc (double, fut_calcData_p);

/* Functions defined in xyzmap.c
	They are used to create the xyzmap (CP22) PT */

double			xyzmap_iFunc (double, fut_calcData_p); 
double			xyzmap_oFunc (double, fut_calcData_p);

/* Functions defined in uvl2lab.c
	They are used to create the uvL->Lab (CP31) PT */
double uvLLab_iu (double u, fut_calcData_p dataP);
double uvLLab_iv (double v, fut_calcData_p dataP);
double uvLLab_iL (double l, fut_calcData_p dataP);
double uvLLab_gFun (double_p dP, fut_calcData_p dataP);
double uvLLab_oFun (double d, fut_calcData_p dataP);

/* Functions defined in lab2uvl.c
	They are used to create the Lab->uvL (CP32) PT */
double LabuvL_iL (double l, fut_calcData_p dataP);
double LabuvL_ia (double u, fut_calcData_p dataP);
double LabuvL_ib (double v, fut_calcData_p dataP);
double LabuvL_gFun (double_p dP, fut_calcData_p dataP);
double LabuvL_ou (double d, fut_calcData_p dataP);
double LabuvL_ov (double d, fut_calcData_p dataP);
double LabuvL_oL (double d, fut_calcData_p dataP);

#endif	/* _MAKEFUTS_H_ */
