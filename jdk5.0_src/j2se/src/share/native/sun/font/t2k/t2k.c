/*
 * @(#)t2k.c	1.36 04/05/02
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * T2K.C
 * Copyright (C) 1989-1998 all rights reserved by Type Solutions, Inc. Plaistow, NH, USA.
 * http://www.typesolutions.com/
 * Author: Sampo Kaasila
 *
 * This software is the property of Type Solutions, Inc. and it is furnished
 * under a license and may be used and copied only in accordance with the
 * terms of such license and with the inclusion of the above copyright notice.
 * This software or any other copies thereof may not be provided or otherwise
 * made available to any other person or entity except as allowed under license.
 * No title to and ownership of the software or intellectual property
 * therewithin is hereby transferred.
 *
 * This information in this software is subject to change without notice
 */
#include "syshead.h"

#include "t2k.h"
#include "util.h"
#include "autogrid.h"
#include "t2ksc.h"
#ifdef ENABLE_T2KE
#include "t2kclrsc.h"
#endif
#include "ghints.h"
#include "HintCalls.h"
								
/*  ORIENTBOLD_STYLES */
#include "OrientDB.h"
#ifdef UIDebug
extern tt_int32 gridMagnify;
extern int isCompositePreFlipHintFlag;
extern int isSbitsFlag,isCompHintFlag;
#endif

#define T2K_MAGIC1 0x5a1234a5
#define T2K_MAGIC2 0xa5fedc5a

#ifdef ENABLE_AUTO_GRIDDING
/*
 *
 */
T2K *NewT2K( tsiMemObject *mem, sfntClass *font, int *errCode  )
{
	assert( errCode != NULL );
	if ( mem == NULL ) {
		*errCode = T2K_ERR_MEM_IS_NULL;
	} else if ( (*errCode = setjmp(mem->env)) == 0 ) {
		/* try */
		register T2K *t = (T2K *)tsi_AllocMem( mem, sizeof( T2K ) );
		t->mem = mem;
		
		t->stamp1 = T2K_MAGIC1;
		t->font	  = font;
		t->stamp2 = T2K_MAGIC2;
		
		
		t->glyph = NULL;
		t->hintHandle = NULL;
		t->baseAddr = NULL;
#ifdef ENABLE_T2KE
		t->baseARGB = NULL;
#endif /* ENABLE_T2KE */
		
		t->ag_xPixelsPerEm = t->ag_yPixelsPerEm = -1;
		/* t->globalHintsCache = NULL; */
		
		
#ifdef LAYOUT_CACHE_SIZE
		{
			int i;
			
			for ( i = 0; i < LAYOUT_CACHE_SIZE; i++ ) {
				t->tag[i] = 0xffffffff;
			}
		}
#endif
		t->font->preferedPlatformID 			= 0xffff;
		t->font->preferedPlatformSpecificID 	= 0xffff;
#ifdef ENABLE_TT_HINTING
 		t->TTHintFontData=0;
 		t->TTHintTranData=0;
 		/* Setup structures for TT hinting. */
	    NewTTHintFontForT2K(t); /* Setup the font for hinting.*/ 
	    InitTTHintTranForT2K(t); /* Setup the transformation for hinting.*/ 
 #endif


		/*  ORIENTBOLD_STYLES */
		t->theContourData.initializedContour=0;
		t->theContourData.active=0;
 		{
 			 sfntClass 		 *font;
			tt_int16 maxContours;				/* in an individual glyph */
			tt_int16 maxCompositeContours;		/* in an composite glyph */

 			maxpClass 			*maxp;
 			font= t->font;
			if (font)
			{
				maxp= font->maxp;	/* get a pointer to the maxprofile table.	*/
				if(maxp)
				{
					maxContours=maxp->maxContours;
					maxCompositeContours=maxp->maxCompositeContours;
					if (maxCompositeContours>maxContours)
						maxContours=maxCompositeContours;
					InitContourData(t->mem, maxContours,&t->theContourData);
				}
				else
				InitContourDataEmpty(&t->theContourData);
			}
		}
 		return t; /*****/
	} else {
		/* catch */
		tsi_EmergencyShutDown( mem );
	}

	return NULL; /*****/
}

static void T2KDoGriddingSetUp( T2K *t )
{
	int err;
	
	if ( t->hintHandle == NULL ) {
		int maxPointCount;
		short unitsPerEm;
		/* ag_FontCategory fontCat; */

/* longjmp( t->mem->env, 9999 ); */

		maxPointCount = GetMaxPoints( t->font );
		unitsPerEm	  = GetUPEM( t->font );
	
		err = ag_HintInit( t->mem, maxPointCount, unitsPerEm, &t->hintHandle );
		tsi_Assert( t->mem, err == 0, err );
		t->fontCategory = GetNumGlyphs_sfntClass( t->font ) < 80000 ? ag_ROMAN : ag_KANJI; /* ag_ROMAN/ag_KANJI guess */
		if ( t->font->globalHintsCache == NULL ) {
			InputStream *in = NULL;
			ag_GlobalDataType globalHints;
			
			#ifdef ENABLE_T1
			if ( t->font->T1 != NULL ) {
				; /* Always do recomputation for T1 */
			} else 
			#endif
			#ifdef ENABLE_CFF
			if ( t->font->T2 != NULL ) {
				; /* Always do recomputation for T2 */
			} else 
			#endif
			/* Force recomputation if algorithmic styling is on */
			if ( t->font->StyleFuncPost == NULL ) {
				in = GetStreamForTable( t->font, tag_T2KG );
			}
			if ( in != NULL ) {
				ReadGHints( &globalHints, in);
				Delete_InputStream( in, NULL );
#ifdef OLD_TEST
				if ( true ) {
					ag_GlobalDataType globalHints2;
					int i;
					
					ComputeGlobalHints( t->font, t->hintHandle, &globalHints2 );
					for ( i = 0; i < ag_MAX_HEIGHTS_IN; i++ ) {
						assert( globalHints.heights[i].flat == globalHints2.heights[i].flat );
						assert( globalHints.heights[i].round == globalHints2.heights[i].round );
						assert( globalHints.heights[i].overLap == globalHints2.heights[i].overLap );
					}
					for ( i = 0; i < ag_MAXWEIGHTS; i++ ) {
						assert( globalHints.xWeight[i] == globalHints2.xWeight[i] );
						assert( globalHints.yWeight[i] == globalHints2.yWeight[i] );
					}
					printf("OK\n");
				}
#endif /* OLD_TEST */
			} else {
				ComputeGlobalHints( t->font, t->hintHandle, &globalHints, t->fontCategory == ag_KANJI  );
			}
			t->font->globalHintsCache = tsi_AllocMem( t->mem, sizeof(ag_GlobalDataType) );
			memcpy(t->font->globalHintsCache, &globalHints, sizeof(ag_GlobalDataType));
		}
		err = ag_SetHintInfo( t->hintHandle, (ag_GlobalDataType *)t->font->globalHintsCache, t->fontCategory ); 
		tsi_Assert( t->mem, err == 0, err );
	}
	if ( ((t->ag_xPixelsPerEm != t->xPixelsPerEm) || (t->ag_yPixelsPerEm != t->yPixelsPerEm))  ) {
		err = ag_SetScale( t->hintHandle, t->xPixelsPerEm, t->yPixelsPerEm, &t->xWeightIsOne );
		t->ag_xPixelsPerEm = t->xPixelsPerEm;
		t->ag_yPixelsPerEm = t->yPixelsPerEm;
		tsi_Assert( t->mem, err == 0, err );
	}	
}

/*
 *
 */
static void T2K_NewTransformationInternal( 
	T2K *t, int doSetUpNow, tt_int32 xPixelsPerEm, tt_int32 yPixelsPerEm, T2K_TRANS_MATRIX *trans
	,VectorSet *unitVectors
	)
{
	t->t00 = trans->t00;
	t->t01 = trans->t01;
	t->t10 = trans->t10;
	t->t11 = trans->t11;
	t->is_Identity = 	t->t00 == ONE16Dot16	&& t->t01 == 0 &&
		 				t->t10 == 0				&& t->t11 == ONE16Dot16;
	t->xPixelsPerEm = xPixelsPerEm;
	t->yPixelsPerEm = yPixelsPerEm;
	t->ttd.unitVectors= *unitVectors;
    if (doSetUpNow) {
		T2KDoGriddingSetUp(t);
	}
}

void T2K_TransformXFunits( T2K *t, short xValueInFUnits, F16Dot16 *x, F16Dot16 *y )
{
	F16Dot16 x16Dot16, y16Dot16;
	F16Dot16 tmpX;

	x16Dot16 = xValueInFUnits; x16Dot16 <<= 16;
	y16Dot16 = 0;
	
	x16Dot16 = util_FixMul( x16Dot16, t->xMul );
	if ( !t->is_Identity ) {
		tmpX = x16Dot16; /* tmpY = 0; */
		x16Dot16 = util_FixMul( t->t00, tmpX ) /* + util_FixMul( t->t01, tmpY ) */;
		y16Dot16 = util_FixMul( t->t10, tmpX ) /* + util_FixMul( t->t11, tmpY ) */;
	}
	*x = x16Dot16;
	*y = y16Dot16;
}

void T2K_TransformYFunits( T2K *t, short yValueInFUnits, F16Dot16 *x, F16Dot16 *y )
{
	F16Dot16 x16Dot16, y16Dot16;
	F16Dot16 tmpY;

	x16Dot16 = 0;
	y16Dot16 = yValueInFUnits; y16Dot16 <<= 16;
	
	y16Dot16 = util_FixMul( y16Dot16, t->yMul );
	if ( !t->is_Identity ) {
		tmpY = y16Dot16; /* tmpX = 0; */
		x16Dot16 = /* util_FixMul( t->t00, tmpX ) */ + util_FixMul( t->t01, tmpY );
		y16Dot16 = /* util_FixMul( t->t10, tmpX ) */ + util_FixMul( t->t11, tmpY );
	}
	*x = x16Dot16;
	*y = y16Dot16;
}

/*
 *
 */
 
#ifdef AllowDropoutControl

	/* returns zero or one of the following flags, defined in config.h:
				DisableScannerDropoutFlag, EnableScannerDropoutFlag	
	*/
	/* NOTE: this code is only executed when the transformation matrix is set. */
	/* Therefore speed is not critical... */
	
	 static tt_int32 EvaluateDropoutState(
	 	tt_int32 xPixelsPerEm, tt_int32 yPixelsPerEm, 
	 	tt_int32 minValue, tt_int32 maxCriticalValue, tt_int32 maxValue)
	 {
	 	/* if either is too small, then no dropout control */
	 	if ( (xPixelsPerEm<minValue) || (yPixelsPerEm<minValue) )
	 		return(DisableScannerDropoutFlag); /* never enable */
	 	/* if either is too big, then no dropout control */
	 	if  ((xPixelsPerEm>maxValue) || (yPixelsPerEm>maxValue) )
	 		return(DisableScannerDropoutFlag); /* never enable */
	 	/* if one is within the critical range, then do it */
	 	if (
	 		 (xPixelsPerEm<=maxCriticalValue)
	 		  ||
	 		 (yPixelsPerEm<=maxCriticalValue)
	 		)
	 			return(EnableScannerDropoutFlag);
	 	return(0); /* otherwise, let the program determine it . */
	 }


	 static DropoutControlFlags FindDropoutControlFlags( tt_int32 xPixelsPerEm, tt_int32 yPixelsPerEm)
	 {
	 	/* use the pixels per em (on each axis) to decide upon dropout control */
	 	tt_int32 hintDropoutFlags= 
	 		EvaluateDropoutState(xPixelsPerEm,yPixelsPerEm,
	 					 MinHintedDropoutControl, 
	 					 MaxCriticalHintedDropoutControl, 
	 					 MaxHintedDropoutControl)<< HintDropoutShift;
	 
	 	tt_int32 t2KDropoutFlags= 
	 		EvaluateDropoutState(xPixelsPerEm,yPixelsPerEm,
	 					 MinT2KDropoutControl, 
	 					 MaxCriticalT2KDropoutControl, 
	 					 MaxT2KDropoutControl)<< T2KDropoutShift;
	 
	 	tt_int32 antiAliasFlags= 
	 		EvaluateDropoutState(xPixelsPerEm,yPixelsPerEm,
	 					 MinAntiAliasDropoutControl, 
	 					 MaxCriticalAntiAliasDropoutControl, 
	 					 MaxAntiAliasDropoutControl)<< AntiAliasDropoutShift;
	 					 
	 	/* Now create the permanent flags */
	 	DropoutControlFlags dcf;
	 	
	 	dcf=  hintDropoutFlags + t2KDropoutFlags + antiAliasFlags;
 	 	return(dcf);
	 }
 
 
#endif

void T2K_NewTransformation( T2K *t, int doSetUpNow, tt_int32 xRes, tt_int32 yRes, T2K_TRANS_MATRIX *trans, int enableSbits, int *errCode )
{
	F16Dot16 xPointSize, yPointSize;
	F16Dot16 xResRatio, yResRatio;
	tt_int32 xPixelsPerEm, yPixelsPerEm;
	register short UPEM;
        
	assert( errCode != NULL );
	if ( (*errCode = setjmp(t->mem->env)) == 0 ) {
		/* try */
		tsi_Assert( t->mem, trans != NULL, T2K_ERR_TRANS_IS_NULL );
		tsi_Assert( t->mem, xRes > 0 && yRes > 0, T2K_ERR_RES_IS_NOT_POS );
		
		UPEM = GetUPEM( t->font );
		
		xPointSize = util_EuclidianDistance( trans->t00, trans->t10 );
		yPointSize = util_EuclidianDistance( trans->t01, trans->t11 );
		
		xResRatio = (xRes << 16) / 72;
		yResRatio = (yRes << 16) / 72;
		
		xPixelsPerEm = util_FixMul( xPointSize, xResRatio );
		yPixelsPerEm = util_FixMul( yPointSize, yResRatio );
		t->xPixelsPerEm16Dot16 = xPixelsPerEm;
		t->yPixelsPerEm16Dot16 = yPixelsPerEm;
		t->xMul = util_FixDiv( t->xPixelsPerEm16Dot16, ((tt_int32)(UPEM)) << 16);
		t->yMul = util_FixDiv( t->yPixelsPerEm16Dot16, ((tt_int32)(UPEM)) << 16);
		/* Round to integer ppem */
		xPixelsPerEm += 0x00008000; xPixelsPerEm >>= 16;
		yPixelsPerEm += 0x00008000; yPixelsPerEm >>= 16;

		/* setup dropout flags */
#ifdef AllowDropoutControl
		t->masterDropOutControlFlags=FindDropoutControlFlags(xPixelsPerEm,yPixelsPerEm);
#else
		t->masterDropOutControlFlags=0;
#endif
		/* Now remove the point size from the matrix */
#ifdef ENABLE_TT_HINTING
		/* Save resolution ( code below to set transformation MTE) */
		t->ttd.xRes=xRes; t->ttd.yRes=yRes;  
 		t->ttd.pointSize=xPointSize>yPointSize?xPointSize:yPointSize;
#endif

		if ( xPixelsPerEm > 0 && yPixelsPerEm > 0 ) {
#ifdef ENABLE_TT_HINTING
		   /*   save   version with all point sizes equal: */
 		   t->ttd.trans=*trans;
		   t->ttd.trans.t00 = util_FixDiv( trans->t00, t->ttd.pointSize );
		   t->ttd.trans.t10 = util_FixDiv( trans->t10, t->ttd.pointSize );
		   t->ttd.trans.t11 = util_FixDiv( trans->t11, t->ttd.pointSize );
		   t->ttd.trans.t01 = util_FixDiv( trans->t01, t->ttd.pointSize );
#endif
			trans->t00 = util_FixDiv( trans->t00, xPointSize );
			trans->t10 = util_FixDiv( trans->t10, xPointSize );
			trans->t11 = util_FixDiv( trans->t11, yPointSize );
			trans->t01 = util_FixDiv( trans->t01, yPointSize );
		} else {
			trans->t00 = 0;
			trans->t10 = 0;
			trans->t11 = 0;
			trans->t01 = 0;
#ifdef ENABLE_TT_HINTING
		       	t->ttd.trans=*trans;
#endif
		}
#ifdef ENABLE_TT_HINTING
		NewTTHintTranForT2K(t);
#endif

		{
			VectorSet unitVectors; 
			ExtractUnitVectors( &unitVectors,
					trans->t00, trans->t01, 
					trans->t10, trans->t11);
 			T2K_NewTransformationInternal( t, doSetUpNow, xPixelsPerEm, yPixelsPerEm, trans,&unitVectors );
		}

		assert( t != NULL && t->font != NULL );
		t->numGlyphs = GetNumGlyphs_sfntClass( t->font );
		
		t->horizontalFontMetricsAreValid 	= false;
		t->verticalFontMetricsAreValid		= false;
#ifdef ENABLE_SBIT
		t->enableSbits = enableSbits && T2K_FontSbitsExists(t) && t->is_Identity;
#else		
		t->enableSbits = enableSbits;
		t->enableSbits = false;
#endif /* ENABLE_SBIT */
		{
			T2K_FontWideMetrics hori, vert;
			int usedOutlines = false;
#if 0
			/* You do not really want sbit metrics for fontwide info */	
#ifdef ENABLE_SBIT
			if ( t->enableSbits ) {
				GetFontWideSbitMetrics( t->font->bloc, t->font->ebsc, (tt_uint16)xPixelsPerEm, (tt_uint16)yPixelsPerEm, &hori, &vert );
				t->horizontalFontMetricsAreValid	= hori.isValid;
				t->verticalFontMetricsAreValid		= vert.isValid;
			}
#endif /* ENABLE_SBIT */
#endif
			if ( !t->horizontalFontMetricsAreValid && !t->verticalFontMetricsAreValid ){
				GetFontWideOutlineMetrics( t->font, &hori, &vert );
				usedOutlines = true;
			}
			if ( hori.isValid ) {
				t->yAscender 	= hori.Ascender;	t->yAscender	<<= 16;
				t->xAscender 	= 0;
				t->yDescender	= hori.Descender;	t->yDescender	<<= 16;
				t->xDescender	= 0;
				t->yLineGap		= hori.LineGap;		t->yLineGap		<<= 16;
				t->xLineGap		= 0;
				t->xMaxLinearAdvanceWidth	= hori.maxAW;	t->xMaxLinearAdvanceWidth		<<= 16;
				t->yMaxLinearAdvanceWidth	= 0;
				t->caretDx		= hori.caretDx;
				t->caretDy		= hori.caretDy;
				
				t->horizontalFontMetricsAreValid 	= true;
				if ( usedOutlines ) {
					/* We need to scale */
					if ( !t->is_Identity ) {
						F16Dot16 tmpX = t->caretDx;
						F16Dot16 tmpY = t->caretDy;
						t->caretDx = util_FixMul( t->t00, tmpX ) + util_FixMul( t->t01, tmpY );
						t->caretDy = util_FixMul( t->t10, tmpX ) + util_FixMul( t->t11, tmpY );
					}
					T2K_TransformYFunits( t, hori.Ascender,		&t->xAscender,				&t->yAscender);
					T2K_TransformYFunits( t, hori.Descender,	&t->xDescender,				&t->yDescender);
					T2K_TransformYFunits( t, hori.LineGap,		&t->xLineGap,				&t->yLineGap);
					T2K_TransformXFunits( t, hori.maxAW,		&t->xMaxLinearAdvanceWidth,	&t->yMaxLinearAdvanceWidth );
				}
			}
			if ( vert.isValid ) {
				t->vert_xAscender 	= vert.Ascender;	t->vert_xAscender	<<= 16;
				t->vert_yAscender 	= 0;
				t->vert_xDescender	= vert.Descender;	t->vert_xDescender	<<= 16;
				t->vert_yDescender	= 0;
				t->vert_xLineGap	= vert.LineGap;		t->vert_xLineGap	<<= 16;
				t->vert_yLineGap	= 0;
				t->vert_yMaxLinearAdvanceWidth	= vert.maxAW;	t->vert_yMaxLinearAdvanceWidth		<<= 16;
				t->vert_xMaxLinearAdvanceWidth	= 0;
				t->vert_caretDx		= vert.caretDx;
				t->vert_caretDy		= vert.caretDy;
				
				t->verticalFontMetricsAreValid 	= true;
				if ( usedOutlines ) {
					/* We need to scale */
					if ( !t->is_Identity ) {
						F16Dot16 tmpX = t->vert_caretDx;
						F16Dot16 tmpY = t->vert_caretDy;
						t->vert_caretDx = util_FixMul( t->t00, tmpX ) + util_FixMul( t->t01, tmpY );
						t->vert_caretDy = util_FixMul( t->t10, tmpX ) + util_FixMul( t->t11, tmpY );
					}
					T2K_TransformXFunits( t, vert.Ascender,	&t->vert_xAscender,					&t->vert_yAscender);
					T2K_TransformXFunits( t, vert.Descender,&t->vert_xDescender,				&t->vert_yDescender);
					T2K_TransformXFunits( t, vert.LineGap, 	&t->vert_xLineGap,					&t->vert_yLineGap);
					T2K_TransformYFunits( t, vert.maxAW, 	&t->vert_xMaxLinearAdvanceWidth,	&t->vert_yMaxLinearAdvanceWidth );
				}
			}
		}
	} else {
		/* catch */
		tsi_EmergencyShutDown( t->mem );
	}
}

	
static F26Dot6 scaleValue( T2K *t, F26Dot6 value, tt_int32 ppem )
{
	register short UPEM = GetUPEM( t->font );
	
	value *= ppem*64;
	value += UPEM>>1;
	value /= UPEM;
	return value;
}

/* Not for external use */
static void T2K_PurgeMemoryInternal( register T2K *t, int level )
{
	int err;

	Delete_GlyphClass( t->glyph ); t->glyph = NULL;
	/* tsi_DeAllocMem( t->mem, t->x ); t->x = NULL; t->y = NULL; */
	
	if ( level > 0 ) {
		tsi_DeAllocMem( t->mem, t->baseAddr ); t->baseAddr = NULL;
		#ifdef ENABLE_T2KE
			tsi_DeAllocMem( t->mem, t->baseARGB ); t->baseARGB = NULL;
		#endif /* ENABLE_T2KE */

		if ( level > 1 ) {
			err = ag_HintEnd( t->hintHandle ); t->hintHandle = NULL;
			t->ag_xPixelsPerEm = t->ag_yPixelsPerEm = -1;
			tsi_Assert( t->mem, err == 0, err  );
		}
	}
}

void T2K_PurgeMemory( register T2K *t, int level, int *errCode )
{
	assert( errCode != NULL );
	if ( (*errCode = setjmp(t->mem->env)) == 0 ) {
		/* try */
		T2K_PurgeMemoryInternal( t, level );
	} else {
		/* catch */
		tsi_EmergencyShutDown( t->mem );
	}
}


/* The following Matrix routines are for handling composite glyph transforms */

/* ** Matrix routines to support full affine transforms for compound glyphs. */
	/* Multiply 2 fixed point matrices */
	static void MatrixTimesMatrix16x16(T2K_TRANS_MATRIX *matA, 
		T2K_TRANS_MATRIX *matB, 
		T2K_TRANS_MATRIX *resultX)			
	{
    T2K_TRANS_MATRIX m,*mptr;	/* create a copy in case the result
												is the same as one of the inputs. */
		mptr=&m;
 		
    mptr->t00=  util_FixMul(matA->t00, matB->t00) + util_FixMul(matA->t01, matB->t10);
    mptr->t01=  util_FixMul(matA->t00, matB->t01) + util_FixMul(matA->t01, matB->t11);
    mptr->t10=  util_FixMul(matA->t10, matB->t00) + util_FixMul(matA->t11, matB->t10);
    mptr->t11=  util_FixMul(matA->t10, matB->t01) +  util_FixMul(matA->t11, matB->t11);

		*resultX= *mptr;				/* return 2x2 result */
	} 
	 
	/* multiply  x,y vectors by transform */
static void  MatrixTimesVector( T2K_TRANS_MATRIX *xfrm, F26Dot6 *xPtr, 
				F26Dot6  *yPtr, tt_uint32	 count )
	{
 		tt_uint32 i;
 		F26Dot6 tmpX, tmpY;
	 	F16Dot16 t00 = xfrm->t00;
		F16Dot16 t01 = xfrm->t01;
		F16Dot16 t10 = xfrm->t10;
		F16Dot16 t11 = xfrm->t11;
    if ( t01 == 0 && t10 == 0 )  {
        for ( i = 0; i < count; i++ ) {
					*xPtr++ = util_FixMul( * xPtr, t00);
					*yPtr++ = util_FixMul( * yPtr, t11);
				}
			} 
    else {
      for ( i = 0; i < count; i++ ) {
						tmpX = *xPtr; 
						tmpY = *yPtr;
		 				*xPtr++ = util_FixMul( t00, tmpX ) + util_FixMul( t01, tmpY );
						*yPtr++ = util_FixMul( t10, tmpX ) + util_FixMul( t11, tmpY );
					}
			}
	}
 /* Does the matrix require styling? Return */
tt_int32  IsUnhintableMatrix( T2K_TRANS_MATRIX *xfrm )
{
    return( (xfrm->t00 <= 0) ||(xfrm->t11<=0) || (xfrm->t01!=0) || (xfrm->t10!=0));
} 

tt_int32  IsQuadrantRotationMatrix( T2K_TRANS_MATRIX *xfrm ) {
    return xfrm->t00 == 0 && xfrm->t11 == 0 || xfrm->t01 == 0 && xfrm->t10 == 0;
}

static void  MatrixTimesPoint( T2K_TRANS_MATRIX *xfrm, F26Dot6 	*xPtr,
			       F26Dot6  	*yPtr)
	{
 		MatrixTimesVector(xfrm,xPtr,yPtr,1);
	}

	
 	static tt_int32 DivideRoundUnits( tt_int32 value, tt_int32 rounder)
	{
		if(value>=0)
 			return( (value+(rounder>>1))/rounder );
		else
        return(-(  ((-value)+(rounder>>1))/rounder ));
} 
		
static void  MatrixTimesOffset( short UPEM, T2K_TRANS_MATRIX *xfrm,
				F26Dot6  *xPtr, tt_int32 xPixelsPerEm,
				F26Dot6  *yPtr, tt_int32 yPixelsPerEm )
	{
		F26Dot6 x, y;
		/* Convert to pixel resolution * UPEM */
 		x  = (*xPtr)*xPixelsPerEm*64;
		y  = (*yPtr)*yPixelsPerEm*64;
		MatrixTimesPoint(xfrm,&x,&y);
		x=DivideRoundUnits(  x, UPEM);
		y=DivideRoundUnits(  y, UPEM);
		*xPtr=x;
		*yPtr=y;
	}

static void SetOneUnitAxis( F16Dot16 t00, F16Dot16 t10, ShortFracVector *sfv)
{
 	F16Dot16 aMagnitude,anInvert;
	if (t10==0) {
 		sfv->x= (t00>0) ? ONE2Dot14: -ONE2Dot14;
 		sfv->y=0;
 	}
 	else if (t00==0) {
 		sfv->x=0;
 		sfv->y=(t10 >0) ? ONE2Dot14: -ONE2Dot14;
 	}
 	else {
		aMagnitude=util_EuclidianDistance( t00, t10 );				
 	 	anInvert=util_FixDiv(0x10000, aMagnitude );
		sfv->x= Frac2Dot14FromFixed( util_FixMul( anInvert, t00) );
		sfv->y= Frac2Dot14FromFixed( util_FixMul( anInvert, t10) );
 	}
}
 	
 	
 	
void ExtractUnitVectors( VectorSet *vs,F16Dot16 t00, F16Dot16 t01, F16Dot16 t10, F16Dot16 t11)
{
  SetOneUnitAxis(  t00, t10, &vs->xVec);
  SetOneUnitAxis(  t01, t11, &vs->yVec);
}
	

#ifdef UIDebug					
 extern int isForceDropoutFlag;
 extern 		int isReverseContourFlag ;
#endif

tt_int32 crossproduct(tt_int32 dx1, tt_int32 dy1, tt_int32 dx2, tt_int32 dy2)
{	
  tt_int32 temp;
  temp =  util_FixMul( dx1 , dy2) - util_FixMul( dx2 , dy1);
  return(temp);

}


#ifdef ENABLE_TT_HINTING
		/* Normally, the StyleFuncPost routine is called after the hinting code. */
		/*		However, when hinting is skipped, it must still be applied.		 */
			void ApplyPostStyle(GlyphClass *glyph,	T2K *t) 		
				{
 					StyleFuncPostPtr sfp= ((t->font)->StyleFuncPost);
					tt_int16 pointCount=glyph->pointCount;
					if (sfp!=0)
					{
						F26Dot6 *xAux;		 /* the x auxilliary */
						F26Dot6 *yAux;		 /* the y auxilliary */
						StyleFuncPostPtr sfp;
						/* allocate memory for the secondary array. */
						xAux = (F26Dot6 *)tsi_AllocMem( t->mem, (pointCount+pointCount) * sizeof(F26Dot6) );
						yAux = &xAux[pointCount];

			 			sfp=   (t->font)->StyleFuncPost;
						/* typically, this will be: 	tsi_SHAPET_BoldItalic_GLYPH_Hinted  */
						(*sfp)( 
							glyph->contourCount,	/* number of contours in the character */
							pointCount,		/* number of points in the characters + 0 for the sidebearing points */
							glyph->sp,		/* sp[contourCount] Start points */
							glyph->ep,  		/* ep[contourCount] End points */
						 	glyph->x,
						 	glyph->y,
						 	xAux,			/* Temporary space. */
						 	yAux,
							t->mem, 
							t->xPixelsPerEm16Dot16,  t->yPixelsPerEm16Dot16, 
							/*  ORIENTBOLD_STYLES */
		 	 				glyph->curveType, 
							glyph->onCurve,	
							&t->theContourData,
							( (t->font)->params)  );
						/* relinquish the extra memory. */
						tsi_DeAllocMem( t->mem,  xAux);
					}
 			}
#endif  



#ifdef UIDebug
		int compooundCount=0;
		int compoundEntries=0;
		int compoundEntriesLocal=0;
		int compoundFlipLocal=0;
		int compoundAllFlipOfOne=0;
		int compoundAllFlip=0;
		int compoundPartialeFlip=0;
		void SysBeepOnce(void);
		int arg1OrgX, arg2OrgX;
		int superTemp=0;
#endif

#ifdef UIDebug

					void 	MarkRangeOut(void);
					void CheckRange(int32 value);
					void UIDebugCheckGlyph( int32  n,int32 *xPtr,int32 *yPtr);
					int32 outOfRange=0;
					int32 tooLow= -64*50;
					int32 tooHigh= 64*50;

					void 	MarkRangeOut(void)
					{
						outOfRange++;
					}

					void CheckRange(int32 value)
					{
						if ((value<tooLow)|| (value>tooHigh))
							MarkRangeOut();
					}
					void UIDebugCheckGlyph( int32  n,int32 *xPtr,int32 *yPtr)
					 {
							/* Check for out-of-bounds */
					 		int32 i;
					 		int32 xVal, yVal;
					  		for (i=0;i<n;i++)
							{
								xVal= xPtr[i];
								yVal= yPtr[i];
								CheckRange(xVal);
								CheckRange(yVal);
							}
						
						
					}
#endif	

static F16Dot16	ShortFractWithFixedMultiply(Frac2Dot14  s, F16Dot16 m)
{
		return( util_FixMul(   (((F16Dot16) s) << 2), m) );
}

static void T2K_RenderGlyphInternal( 

	T2K *t, 			/* the scaler data */
	tt_int32 code, 			/* the glyph index or character code */
	tt_int8 xFracPenDelta, 
	tt_int8 yFracPenDelta, 
	tt_uint8 greyScaleLevel, tt_uint8 cmd, 
	tt_int32 hintingComposite)				/* set to depth of recursion: should never exceed 1 */

{
	F26Dot6 *xPtr, *yPtr;
	int err, pointCount;
	tt_int32 i, n;
	register GlyphClass *glyph;
	int isFigure = false;
	short kern;
	tt_uint16 advanceWidth, 			/* Advanced width, including style */
			advanceWidthPure, styleDeltaAdjust;		/* Advanced width, without style. 	*/
	tt_int32 errorCode;
	int isOptimizeOutline=0;
	register short UPEM;
	T2K_TRANS_MATRIX  save;
	VectorSet saveVectorSet;
	tt_int32 save_XPPEM, save_YPPEM;
	tt_int32 isUnhintableMatrixFlag = 0;
	int gotComponentMatrix=0;
	int isCompound;
	tt_int32 rawArg1, rawArg2;
 	tsiScanConv  scHintData;			 /* allocate structure on stack */
	tsiScanConv *scHint = 0;		/*   Use this structure when creating hinted images. */
									/* if scHint is non-zero, it contains the image data, (but
										other data is deallocated. */
									
	/* MTE: the isCompound value is a flag to indicate, at this level
			of recursion, whether or not the glyph is compound. If compound, then
			no scaling is applied at the end, because the scaling is done for each
			glyph component. But why can't we just look at the glyph contour count?
			because the glyph changes when a recursive call is made.
	*/
	/* Should we read the hints? Do we hint if color is used? */
#ifdef ENABLE_TT_HINTING 
#define isTTReadHints true
#else
#define isTTReadHints false
#endif
 	tsi_Assert( t->mem, /* greyScaleLevel >= BLACK_AND_WHITE_BITMAP && */ 
	greyScaleLevel <= GREY_SCALE_BITMAP_EXTREME_QUALITY, T2K_ERR_BAD_GRAY_CMD  );
/*	tsi_Assert( t->mem, xFracPenDelta >= 0 && xFracPenDelta < 64 && yFracPenDelta >= 0 && yFracPenDelta < 64, T2K_ERR_BAD_FRAC_PEN  ); */

	
	UPEM = GetUPEM( t->font );
	T2K_PurgeMemoryInternal( t, 1 );
	
	kern = 0;
	if ( cmd & T2K_CODE_IS_GINDEX ) {
		glyph = GetGlyphByIndex( t->font, code, isTTReadHints, &advanceWidth );

		isFigure = IsFigure( t->font, (unsigned short)code );
	} else {
		glyph = GetGlyphByCharCode( t->font, code, isTTReadHints, &advanceWidth );
		if ( code >= '0' && code <= '9' ) isFigure = true;
	}
 	/* ORIENTBOLD_STYLES */
 	/* Now that we have the glyph, we can check the contours. 				*/
	/* The contour data is checked on the original data (or for compound	*/
	/* glyphs, it includes flipping the data points.						*/
	/* Therefore, whether hinting or not, the final transform matrix 		*/
	/* may apply another transform flip, in which case, the contour data	*/
	/* may need to be flipped. See routine FlipContourDataList.				*/
	/* Rotation and stretching have no affect on the contour data, except	*/
	/* that attempts to use it after hinting may fail because the hinting	*/
	/* can leave the outlines in bad shape, and change the oncurve flages.	*/
 	AccumulateGlyphContours(&t->theContourData, glyph );
 	/* the contour information includes the T2K_FLIP_GLYPH effect. */
 

 	styleDeltaAdjust=t->font->hmtxLinearAdjustment;
	advanceWidthPure=advanceWidth - styleDeltaAdjust;
#ifdef ENABLE_TT_HINTING
		/* When hinting, stylistic changes must be added after hinting takes place.  */
		/* Therefore, we must go back and remove the adjustment to phantom points.   */
		/* Note: this adjustment is in addition to stylistic changes. 				 */
		if (t->font->hmtxLinearAdjustment)	{
			tt_uint32 rightPhantomIndex= glyph->pointCount+1;
			glyph->oox[rightPhantomIndex]-=styleDeltaAdjust;
			}
#endif	
	
	/* For all code, the right side phantom point has been adjusted by the style. 				*/
	/* This is incorrect for hinting, where the final style adjusts the phantom automatically. */
	tsi_Assert( t->mem, glyph != NULL, T2K_ERR_GOT_NULL_GLYPH  );
	t->glyph = glyph;
	isCompound=  glyph->contourCount < 0;
	save.t00 = t->t00;
	save.t01 = t->t01;
	save.t10 = t->t10;
	save.t11 = t->t11;
	save_XPPEM = t->xPixelsPerEm;
	save_YPPEM = t->yPixelsPerEm;
#ifdef ENABLE_TT_HINTING
	/* We determine if styling is done before or after */
	/* We also assumet that simple glyph, of a compound glyph, does
		not contain a transform which flips, rotates, or skews */
	isUnhintableMatrixFlag= IsUnhintableMatrix( &save );
#endif
	saveVectorSet= t->ttd.unitVectors;
	if ( isCompound ) {
		GlyphClass *base = NULL;
		tt_uint8 comp_cmd;
		tt_uint16 flags, oredFlags = 0;
		T2K_TRANS_MATRIX  newbie,totalCompositeMatrix;
 		int newT;
		short *componentData = glyph->componentData; /* Grab it! */
		tt_uint8 *hintFragmentSave;
		tt_int32 hintLengthSave;
		tt_int32 hasComponentFlip=0, startContourValue, endContourValue;
 		
		
		glyph->componentData = NULL;
		
		/* Save the compound glyph's hint information. We wont need it till later */
		hintFragmentSave=glyph->hintFragment;
		hintLengthSave=glyph->hintLength;

		/* Set the value to zero so that it is not de-allocated. */
		glyph->hintFragment=0L;
		glyph->hintLength=0;
		
 		
		comp_cmd = (unsigned char)(cmd & ( T2K_GRID_FIT | T2K_USE_FRAC_PEN));
		comp_cmd |= T2K_RETURN_OUTLINES | T2K_CODE_IS_GINDEX;
 
 		i = 0;
		do {
			/* Use in32 to ensure correctness in LP64 */
			tt_int32 gIndex;
			tt_int32 arg1, arg2;
			
			flags  = componentData[i++]; oredFlags |= flags;
			gIndex = (tt_uint16) componentData[i++];
 			if ( (flags & ARG_1_AND_2_ARE_WORDS) != 0 ) {
				arg1 = componentData[i++];
				arg2 = componentData[i++];
 			} else {
				arg1 = componentData[i++];
				/* Signed/unsigned bug fixed Dec 8, 1998 ---Sampo */
 				if ( flags & ARGS_ARE_XY_VALUES ) {
	 				arg2 = (tt_int32)((tt_int8)(arg1 & 0xff));
	 				arg1 >>= 8;
 				} else {
	 				arg2 = arg1 & 0xff;
	 				arg1 >>= 8;
 					arg1 &= 0xff;
 				}
 			}

 			newT = false;
 			if ( flags & ARGS_ARE_XY_VALUES ) 
 			{
#if 0
 			MatrixTimesOffset(GetUPEM( t->font ),&save,&arg1, t->xPixelsPerEm, &arg2,t->yPixelsPerEm);
#else
				
 				arg1 = scaleValue( t, rawArg1=arg1, t->xPixelsPerEm );
 				arg2 = scaleValue( t, rawArg2=arg2, t->yPixelsPerEm );
#endif

 			}
 			if ( (flags & WE_HAVE_A_SCALE) != 0 ) {
		        newbie.t00 = componentData[i] << 2; /* format f2Dot14 */
				newbie.t01 = 0;
				newbie.t10 = 0;
				newbie.t11 = componentData[i] << 2;
				i++;
				newT = true;
 			} else if ( (flags & WE_HAVE_AN_X_AND_Y_SCALE) != 0 ) {
				newbie.t00 = componentData[i++] << 2;
				newbie.t01 = 0;
				newbie.t10 = 0;
				newbie.t11 = componentData[i++] << 2;
				newT = true;
 			} else if ( (flags & WE_HAVE_A_TWO_BY_TWO) != 0 ) {
				/* NB: According to Apple (http://developer.apple.com/fonts/TTRefMan/RM06/Chap6glyf.html)
				       matrix has following form
				       (t00 t10)
				       (t01 t11)
				*/
				newbie.t00 = componentData[i++] << 2;
				newbie.t10 = componentData[i++] << 2;
				newbie.t01 = componentData[i++] << 2;
				newbie.t11 = componentData[i++] << 2;
				newT = true;
 			} else {
			  newbie.t00 = ONE16Dot16;
			  newbie.t11 = ONE16Dot16;
			  newbie.t01 = 0;
			  newbie.t10 = 0;
            }

 		
  		if ( newT ) 
 			{ 
			  /* check whether orientation will change due to transform */
 				if ( crossproduct(newbie.t00, newbie.t01, newbie.t10, newbie.t11) < 0)
 				{
 					startContourValue=t->theContourData.current;
 					hasComponentFlip=1;
 				}
			
 				/* Create the total transform: local * global */
 				MatrixTimesMatrix16x16(&save, &newbie, &totalCompositeMatrix);
			{
				VectorSet unitVectors; 
				ExtractUnitVectors( &unitVectors,
					totalCompositeMatrix.t00, totalCompositeMatrix.t01, 
					totalCompositeMatrix.t10, totalCompositeMatrix.t11);
 				T2K_NewTransformationInternal( t, false, t->xPixelsPerEm, t->yPixelsPerEm, &totalCompositeMatrix,
 							&unitVectors);
 			
 			}
  			}
			/* T2K_RenderGlyph( t, gIndex, xFracPenDelta, yFracPenDelta, 0, comp_cmd, NULL ); */
			T2K_RenderGlyphInternal( t, gIndex, xFracPenDelta, yFracPenDelta, 0, comp_cmd,hintingComposite+1);

				
 			if ( newT ) {
 			/* Restore the transform. */
 				T2K_NewTransformationInternal( t, false, save_XPPEM, save_YPPEM, &save, &saveVectorSet );
  			}
			Add_GlyphClass( &base, t->glyph, flags, arg1, arg2, rawArg1, rawArg2, newbie);

			/* we need to flip countours if orientation has changed (see also 4355226) */
 	 		if (t->theContourData.initializedContour &&
 	 			t->theContourData.active && hasComponentFlip) {
 	 			endContourValue=t->theContourData.current-1;
 	 			FlipContourDataList(&t->theContourData,
 	 								startContourValue, endContourValue );
 	 		}
postGlyph:		
			if ( base != t->glyph ) {
				Delete_GlyphClass( t->glyph );
			}
			t->glyph = NULL;
		} while ( flags & MORE_COMPONENTS );
		
		/*******************************************************************************************/
		/**************** We have now gathered all the components of compound glyph ****************/
		/*******************************************************************************************/
		t->glyph = glyph = base;
		tsi_DeAllocMem( t->mem, componentData );
		xPtr = glyph->x;
		yPtr = glyph->y;
		pointCount = glyph->pointCount; n = pointCount + 2;

		if (  !(oredFlags & USE_MY_METRICS) ) {
			glyph->oox[glyph->pointCount + 0] = 0;
			glyph->oox[glyph->pointCount + 1] = (short)(glyph->oox[t->glyph->pointCount + 0] + advanceWidthPure);
			glyph->x[glyph->pointCount + 1]   = scaleValue( t, glyph->oox[glyph->pointCount + 1], t->xPixelsPerEm );
 			if (cmd & T2K_GRID_FIT) {
				glyph->x[glyph->pointCount + 1] += 32;
				glyph->x[glyph->pointCount + 1] &= ~63;

				glyph->y[glyph->pointCount + 1] += 32;
				glyph->y[glyph->pointCount + 1] &= ~63;
			}

			glyph->x[glyph->pointCount + 0]   = 0; /* == scaleValue(0) */	
			glyph->y[glyph->pointCount + 0]   = 0; /* == scaleValue(0) */	
								
 		}
		
		

 		{
			/* Composite Hinting:
				At this point each simple glyph has already been hinted and combined
					into a single composite glyph.
				Before proceeding we must see if hinting is required.
				There are 3 conditions:
					1. T2K_GRID_FIT flag must be set n the cmd. 
					2. T2K_USE_FRAC_PEN flag must NOT be set n the cmd. Why? Because Frac Pen causes funny adjustment.
							FracPen is a higher level function which should not be on for the actual hinting.
							The LinearAdvance values can be used to determine how far to advance the drawing pen,
									or use the hmtx table for linear advance.
					3. There must be some outline points.
			*/

		  /*for those composite glyphs which have their "scale" info totally depend
			on hinting instructions, we have to turn on the hinting engine even
			when "isUnhintableMatrixFalg" is on or it's not a "T2K_GRID_FIT operation".
			Font "mingliu" on Microsoft Traditional Chinese version Windows is an example. 
            This variable indicates that we don't have any flaged "scale" info, 
			so have to apply hinting to scale component glyph correctly
		  */
		  tt_int32 haveScaleInfo = oredFlags & WE_HAVE_A_SCALE |
			                       oredFlags & WE_HAVE_AN_X_AND_Y_SCALE |
			                       oredFlags & WE_HAVE_A_TWO_BY_TWO;
			isOptimizeOutline= 
#ifdef ENABLE_TT_HINTING		
					(!haveScaleInfo || !isUnhintableMatrixFlag && (cmd & T2K_GRID_FIT))  &&	
#else
					(cmd & T2K_GRID_FIT) && 		/* If not grid-fitting, then nothing more. 	*/
#endif
					(!(cmd & T2K_USE_FRAC_PEN)) && 	/*	Fractional pen => no grid fitting. 		*/
					(pointCount > 1) ;				/* Nothing to do, if there areno points. 	*/

			/* restrore the specified hint information, by first de-allocating the current glyph. */
			if ( glyph->hintLength > 0 ) 
			{
				assert( glyph->hintFragment != NULL );
				/* The hintFragment belongs to the last simple glyph, and NOT what we need. */
				/* Therefore de-allocate it, and initialize to empty. */
				tsi_DeAllocMem( t->mem, (char *)glyph->hintFragment );
				glyph->hintFragment = NULL;
				glyph->hintLength = 0;
			}
			
			/* We can now restore the original hinting information which was saved before processing */
			/*   all of the simple glyphs. */
			glyph->hintFragment= hintFragmentSave;
			glyph->hintLength=hintLengthSave;
#ifdef ENABLE_TT_HINTING		
			/* If all 3 conditions are meet, we can proceed to hint the data. */
  			if (isOptimizeOutline)
	 			{
	 				int wantEarlyHintedImage;
	 				tt_int32 applyStyleTrue ;
	 				/* Originally, within T2K, massaging the outline and building a bit map 	*/
	 				/* are kept clearly separate. But the long setup time for hinting mandates 	*/
	 				/* that, where possible, both are done at the same time. 					*/
	 				/* if wantEarlyHintedImage is True, then we might as well do the bitmap		*/
	 				/*      creation right now. Otherwise, we won't need it, ever. 				*/

  					wantEarlyHintedImage=
			 				( cmd & T2K_SCAN_CONVERT && glyph->pointCount > 1 )
#ifdef UIDebug
  	&& (gridMagnify==0)
#endif
							 &&
			 				 ( !  ( glyph->colorPlaneCount > 0 )
			 				 &&(greyScaleLevel == BLACK_AND_WHITE_BITMAP)  );
			 		applyStyleTrue=1; /* for all cases we apply the style the final result */
					if (isUnhintableMatrixFlag) {
					    /*came here because "!haveScaleInfo", so need to setup 
						  new transMatrix for hinting program
						  */
						T2K_TRANS_MATRIX  trans;
						trans.t00 = trans.t11 = ONE16Dot16;
						trans.t01 = trans.t10 = 0;
						t->ttd.trans = trans;
						/*setup a new trans for hinting*/
						NewTTHintTranForT2K(t);
						/*and we don't need hinted image*/
						wantEarlyHintedImage = 0;
					}
					if (!(cmd & T2K_GRID_FIT)) {
					    wantEarlyHintedImage = 0;
					}
                          
 					if (wantEarlyHintedImage)
						{
		 					scHint =&scHintData;
		 					scHint->mem=t->mem;
							scHint->baseAddr=0;
							scHint->xEdge=0;
		     				errorCode=TTScalerHintGlyph((GlyphClass *)glyph,( T2K *)t,(void **)&scHint,applyStyleTrue);
		     				/* If error code, then no image is ever created. */
		     				/* If there is an error in imaging, then scHint is already set to zero. */
		     				/* if scHint !=0 then the address of the image is available. */
		   				}
					    else 
		    				errorCode=TTScalerHintGlyph((GlyphClass *)glyph,( T2K *)t,(void **) 0L,applyStyleTrue);
					if (isUnhintableMatrixFlag) {
					    /*reset the ttd trans info, it might not be necessary though*/
                        t->ttd.trans = save;
                        NewTTHintTranForT2K(t);
                    }
	    		}
	    		else
 						/* Always apply styling after all else. */
 	    			 	 ApplyPostStyle( glyph, t);
#endif
	/* end ENABLE_TT_HINTING */
	
	    		 glyph->hintFragment= hintFragmentSave;
 				 glyph->hintLength=hintLengthSave;

 			}

	} else {     	/* ************** ------------ END COMPOUND ---------- START SIMPLE GLYPH *********************** */
 				 	/* ************** ------------ END COMPOUND ---------- START SIMPLE GLYPH *********************** */
 				 	
 		/* This code is used to process a simple glyph. It may occur in two ways. First, it may truly be a request */
 		/*     to render a simple glyph. But it might also be part of a compound glyph. 							*/
 		/*	The routine parameter, hintingComposite, is non-zero when hinting a simple glyph within a composite. 	*/
 		/*  It is zero for a simple glyph. It is assumed that the value never exceeds 1! ?							*/
 		/*		that it will be other than 0 or 1.																	*/
 		 
 		
 		
 		pointCount = glyph->pointCount;
		n    = pointCount + 2;
		xPtr = (F26Dot6*) tsi_AllocMem( t->mem, (n) * 2 * sizeof( F26Dot6 ) );
		yPtr = &xPtr[ n ];
		glyph->x = xPtr;
		glyph->y = yPtr;

#ifdef ENABLE_T2KE
		if ( t->font != NULL && t->font->T2KE != NULL && t->font->T2KE->properties[T2KE_PROP_HINTS_DISABLED] != 0 ) {
			cmd &= ~T2K_GRID_FIT;
		}
#endif		
			isOptimizeOutline=
#ifdef ENABLE_TT_HINTING		
               /*if we have quadrant rotation matrix (bug #4445525 or #4896786 for example) 
                 then even if the "isUnhintableMatrixFlag" is true, we still 
                 want to apply hinting on it. We setup an identity matrix first, 
                 apply the hinting on glyph with this "identity" matrix, and then 
                 apply the real matrix later. 
                            */
               (IsQuadrantRotationMatrix(&save) || 
               !isUnhintableMatrixFlag) &&  /* If we have pre-styled, then cannot hint. */
#endif
			  (cmd & T2K_GRID_FIT) && 		/* If not grid-fitting, then nothing more. 	*/
					(!(cmd & T2K_USE_FRAC_PEN)) && 	/*	Fractional pen => no grid fitting. 		*/
					(pointCount > 1) ;				/* Nothing to do, if there are no points/contours.*/


#ifndef ENABLE_TT_HINTING
		if (isOptimizeOutline ) {
			ag_ElementType elem;
			short curveType = 2;
			
			T2KDoGriddingSetUp( t );
			elem.contourCount   = glyph->contourCount;
			tsi_Assert( t->mem, pointCount <= 32000, T2K_ERR_TOO_MANY_POINTS  );

			elem.pointCount 	= (short)pointCount;
			elem.sp 			= glyph->sp;
			elem.ep 			= glyph->ep;
			elem.oox 			= glyph->oox;
			elem.ooy 			= glyph->ooy;
			elem.onCurve 		= glyph->onCurve;
			elem.x 				= xPtr;
			elem.y 				= yPtr;
			err = ag_AutoGridOutline( t->hintHandle, &elem, (short)isFigure, curveType, 
				(short)(greyScaleLevel > BLACK_AND_WHITE_BITMAP) );
			tsi_Assert( t->mem, err == 0, err  );
		} else 
#endif
 {
			/* scale glyph */
			register tt_int32 xMultiplier = t->xPixelsPerEm*64;
			register tt_int32 yMultiplier = t->yPixelsPerEm*64;
			register F26Dot6 tmpX, tmpY;
			
			int halfUPEM= (UPEM>>1);
		 	for ( i = 0; i < n; i++ ) 
				{
					tmpX = glyph->oox[i];
					tmpY = glyph->ooy[i];
					tmpX *= xMultiplier;
					tmpY *= yMultiplier;

  					/* guarantee symmetric around zero */
  					if ((tmpX)>=0) 				/* Correct Negative Rounding to compensate for divisor. */
						tmpX = (tmpX+halfUPEM)/UPEM;
					else 
						tmpX = -((-tmpX+halfUPEM)/UPEM);
  					if ((tmpY)>=0) 				/* Correct Negative Rounding to compensate for divisor. */
						tmpY = (tmpY+halfUPEM)/UPEM;
					else 
						tmpY = -((-tmpY+halfUPEM)/UPEM);
  					xPtr[i] = tmpX + xFracPenDelta;
					yPtr[i] = tmpY - yFracPenDelta;
				}
			/* Undo for the side bearing points */ 
			xPtr[--i] -= xFracPenDelta;
			yPtr[  i] += yFracPenDelta;
			xPtr[--i] -= xFracPenDelta;
			yPtr[  i] += yFracPenDelta;
#ifdef ENABLE_TT_HINTING
	 			if (isOptimizeOutline)
	 			{
	 				int wantEarlyHintedImage;
	 				tt_int32 applyStyle=0;

					wantEarlyHintedImage= (greyScaleLevel==0) &&
			 				( cmd & T2K_SCAN_CONVERT && glyph->pointCount > 1 )
#ifdef UIDebug
  										&& (gridMagnify==0)
#endif
							 &&
			 				 ( !  ( glyph->colorPlaneCount > 0 ) &&
			 				  (greyScaleLevel == BLACK_AND_WHITE_BITMAP));


                   if (isUnhintableMatrixFlag) { 
                       /*though it's an "UnhintableMatrix", we reset the ttd 
						 transform (for hinting) to an identity matrix and
						 apply the hinting on it, we will then apply the real
						 matrix later (see "if its a final compound glyph...")
						 
						 In this case, we should not call MatrixTimesVector()
						 to apply the matrix on glyph outline here.
						 */
					   T2K_TRANS_MATRIX  trans;
					   trans.t00 = trans.t11 = ONE16Dot16;
					   trans.t01 = trans.t10 = 0;
					   t->ttd.trans = trans;
					   NewTTHintTranForT2K(t);
					   wantEarlyHintedImage = 0;
				   }
				   else {
					   MatrixTimesVector(&save, xPtr,  yPtr,n);
				   }

			 		/* For sub-components of a compound glyph, we don't want to add style */
			 		/*   The style changes will be applied after the full glyph is hinted. */
		   /* though QuadrantRotationMatrix is an unhintable matrix, we let it come down here to */
		   /* solve bug#4445525, so set applyStyle to true. It will be ideal if the t2k hinting  */
		   /* can finally handle the quadrant rotation matrix, then we will be able to merge     */
		   /* IsQuadrantRotationMatrix() into isUnhintableMatrix()                               */
			 		applyStyle=(hintingComposite==0)&&(!isUnhintableMatrixFlag ||IsQuadrantRotationMatrix(&save));
					if (wantEarlyHintedImage)
					{
	 					scHint =&scHintData;
	 					scHint->mem=	t->mem;
						scHint->baseAddr=0;
						scHint->xEdge=0;	
	     				errorCode=TTScalerHintGlyph((GlyphClass *)glyph,( T2K *)t,(void **)&scHint,applyStyle);
	     				/* If error code, then no image is ever created. */
	     				/* If there is an error in imaging, then scHint is already set to zero. */
	     				/* if scHint !=0 then the address of the image is available. */
	   				}
					else 
	    				errorCode=TTScalerHintGlyph((GlyphClass *)glyph,( T2K *)t,(void **) 0L,applyStyle);

                   if (isUnhintableMatrixFlag) { 
					   /*reset the ttd transform, might not be necessary */
                       t->ttd.trans = save;
                       NewTTHintTranForT2K(t);                    
                   }

	    		}
	    		else{
						/*  ORIENTBOLD_STYLES 	  */
 						if  (hintingComposite==0)
 		    			 	 ApplyPostStyle( glyph, t);
		    		}
#endif
 		 }

		if ( !(cmd & T2K_USE_FRAC_PEN) ) 
				{
					/* To ensure integer spacing for no gridfitting, and something like */
					/* the space character with zero points */
					F26Dot6 A,W;
					F26Dot6 x1Old, x1New, wOld;
								
					
					A = (x1Old = xPtr[pointCount]);
					W = wOld = xPtr[pointCount+1] - A;
					A +=  32; A &= ~63; 		/* integralize */
					W +=  32; W &= ~63;
					
					xPtr[pointCount] 	= x1New = A; 	/* write out */
					xPtr[pointCount+1]  = A+W; 	/* write out */
				}
	}
	t->xLinearAdvanceWidth16Dot16 = advanceWidth; t->xLinearAdvanceWidth16Dot16 <<= 16;
	t->yLinearAdvanceWidth16Dot16 = 0;
	
#ifdef ENABLE_T1
										if ( t->font->T1 != NULL && t->font->T1->m01 != 0 ) {
											F16Dot16 skew = t->font->T1->m01;
											for ( i = 0; i < n; i++ ) {
												xPtr[i] += util_FixMul( skew, yPtr[i] );
											}
										} 
#endif
#ifdef ENABLE_CFF
										if ( t->font->T2 != NULL && t->font->T2->topDictData.m01 != 0 ) {
											F16Dot16 skew = t->font->T2->topDictData.m01;
											for ( i = 0; i < n; i++ ) {
												xPtr[i] += util_FixMul( skew, yPtr[i] );
											}
										} 
#endif
	{
 		t->xLinearAdvanceWidth16Dot16	= util_FixMul( t->xLinearAdvanceWidth16Dot16, t->xMul );
		
		/* go to 16.16 for the side bearing points */
		xPtr[pointCount] <<= 10;
		yPtr[pointCount] <<= 10;
		xPtr[pointCount+1] <<= 10;
		yPtr[pointCount+1] <<= 10;
	}
	/*  if its a final compound glyph,  or a single glyph 
			then apply the matrix.
	*/
 	if (  ( 
				(t->t00 != 0x10000) ||
				(t->t11 != 0x10000) ||
				(t->t10 !=  0) ||
				(t->t01 !=  0) 
		  )
			   &&  ( hintingComposite == 0 ) 
	)
 	{
		F16Dot16 t00 = t->t00;
		F16Dot16 t01 = t->t01;
		F16Dot16 t10 = t->t10;
		F16Dot16 t11 = t->t11;
		register F26Dot6 tmpX, tmpY;
		if ( t01 == 0 && t10 == 0 ) 
			{
						for ( i = 0; i < n; i++ ) 
						{
							tmpX = xPtr[i]; tmpY = yPtr[i];
							xPtr[i] = util_FixMul( t00, tmpX ) + 0;
							yPtr[i] = 0 + util_FixMul( t11, tmpY );
						}
		  	} 
	  else 
	  		{
						for ( i = 0; i < n; i++ )
						{
									tmpX = xPtr[i]; tmpY = yPtr[i];
									xPtr[i] = util_FixMul( t00, tmpX ) + util_FixMul( t01, tmpY );
									yPtr[i] = util_FixMul( t10, tmpX ) + util_FixMul( t11, tmpY );
						}
			}
 	}
 	
 	{	
		register F26Dot6 tmpX, tmpY;		
		F16Dot16 t00 = t->t00;
		F16Dot16 t01 = t->t01;
		F16Dot16 t10 = t->t10;
		F16Dot16 t11 = t->t11;
		tmpX = t->xLinearAdvanceWidth16Dot16; tmpY = 0;
		t->xLinearAdvanceWidth16Dot16 = util_FixMul( t00, tmpX ) /* + util_FixMul( t01, tmpY ) */;
		t->yLinearAdvanceWidth16Dot16 = util_FixMul( t10, tmpX ) /* + util_FixMul( t11, tmpY ) */;
	}
	
	
	
	t->xAdvanceWidth16Dot16	= xPtr[pointCount+1] - xPtr[pointCount];
	t->yAdvanceWidth16Dot16	= yPtr[pointCount+1] - yPtr[pointCount];
	
	/* Translate back to 26.6 from 16.16 */
	xPtr[pointCount] >>= 10;
	yPtr[pointCount] >>= 10;
	xPtr[pointCount+1] >>= 10;
	yPtr[pointCount+1] >>= 10;
	
		/* ******************************************************************************************************************** */
	/* BEGIN SCAN CONVERSION, IF NOT ALREADY DONE. */
	/* ******************************************************************************************************************** */
#ifdef ENABLE_T2KE
	t->baseARGB = NULL;
#endif /* ENABLE_T2KE */
	
	t->baseAddr = NULL;	t->rowBytes = 0;
	t->width = 0; 		t->height = 0;
	t->fTop26Dot6 = 0;	t->fLeft26Dot6 = 0;

	if ( cmd & T2K_SCAN_CONVERT && glyph->pointCount > 1 ) {
	tsiScanConv *sc = NULL; 
#ifdef ENABLE_T2KE
		tsiColorScanConv *scc = NULL;
#endif

		char xWeightIsOne = (char)(t->xWeightIsOne && (cmd & T2K_GRID_FIT) && t->ag_xPixelsPerEm <= 24);
	 
		if ( glyph->colorPlaneCount > 0 ) {
#ifdef ENABLE_T2KE
			assert( t->font->T2KE != NULL );
			scc = tsi_NewColorScanConv( t->mem, (void *)t->font->T2KE, (void*)t, glyph->contourCount, glyph->sp, glyph->ep,
								 	glyph->colors, glyph->colorPlaneCount,
				                  	xPtr, yPtr, (char *)glyph->onCurve, greyScaleLevel, glyph->oox, glyph->ooy, GetUPEM( t->font ) );
			MakeColorBits( scc, greyScaleLevel, xWeightIsOne, (char)(cmd & T2K_SKIP_SCAN_BM) );
			t->width  		= scc->right  - scc->left;
			t->height 		= scc->bottom - scc->top;
			t->fTop26Dot6	= scc->fTop26Dot6;
			t->fLeft26Dot6  = scc->fLeft26Dot6;
			t->rowBytes 	= scc->rowBytes;
#else
			assert( false );
#endif
		} 
		else  
		{
 	 		if (scHint)
	 		{
	 			/* If we have a hinted image, we can use it, instead of
	 			   our own. */
	 			t->width  		= scHint->right  - scHint->left;
				t->height 		= scHint->bottom - scHint->top;
				t->fTop26Dot6	= scHint->fTop26Dot6;
				t->fLeft26Dot6  = scHint->fLeft26Dot6;
				t->rowBytes 	= scHint->rowBytes;
	 		}
	 		else
	 		{
	 			short isDropout=1;  /* Always assume dropout control. */
	 			
#ifdef AllowDropoutControl
	 			DropoutControlFlags masterDropOutControlFlags= t->masterDropOutControlFlags;

	 			if (BLACK_AND_WHITE_BITMAP==greyScaleLevel)
	 				masterDropOutControlFlags >>=  AntiAliasDropoutShift;
	 			else
	 				masterDropOutControlFlags >>=  T2KDropoutShift;
	 			if ( isDropout & DisableScannerDropoutFlag)
	 				isDropout=0;
#endif	 				

#ifdef UIDebug
				if (isForceDropoutFlag )
						isDropout=1; /* force it on, even if not needed. */
				{
					int i;
					if (gridMagnify)
						for (i=0;i<(glyph->pointCount +2);i++)
						{
							 glyph->x[i] *= gridMagnify;  
							 glyph->y[i] *= gridMagnify;
						}
				}
#endif
	 			sc = tsi_NewScanConv( t->mem, glyph->contourCount, glyph->sp, glyph->ep,
					                  xPtr, yPtr, (char *)glyph->onCurve, greyScaleLevel, 
					                  (char)glyph->curveType );
				MakeBits( sc, greyScaleLevel, xWeightIsOne, (char)(cmd & T2K_SKIP_SCAN_BM),true,isDropout);
 	 			t->width  		= sc->right  - sc->left;
				t->height 		= sc->bottom - sc->top;
				t->fTop26Dot6	= sc->fTop26Dot6;
				t->fLeft26Dot6  = sc->fLeft26Dot6;
				t->rowBytes 	= sc->rowBytes;
			}
		}


		
		if ( glyph->colorPlaneCount > 0 ) {
#ifdef ENABLE_T2KE
			t->baseARGB = scc->baseARGB; scc->baseARGB = NULL; /* We take over and deallocate t->baseARGB */
			t->baseAddr = NULL;
#else
			assert( false );
#endif
		} else 
		{
			if (scHint)
			{
				t->baseAddr = scHint->baseAddr; 
				scHint->baseAddr=0;			
			}
			else
			{
				 t->baseAddr  = sc->baseAddr;
				 sc->baseAddr = NULL; /* We take over and deallocate t->baseAddr */
#ifdef ENABLE_T2KE
				t->baseARGB = NULL;
#endif
			}
		}
		
		/* Compensate if the lsb_point != 0 */
		if ( (i=xPtr[pointCount]) != 0 ) {
			t->fLeft26Dot6 -= i;
		}
		if ( (i=yPtr[pointCount]) != 0 ) {
			t->fTop26Dot6 -= i;
		}
		
		t->fLeft26Dot6 &= ~63;
		t->fTop26Dot6  &= ~63;
		
#ifdef ENABLE_T2KE
		tsi_DeleteColorScanConv( scc );
#endif
		tsi_DeleteScanConv( sc );  
	}
	
	if ( cmd & T2K_RETURN_OUTLINES ) {
		;
	} else {
		T2K_PurgeMemoryInternal( t, 0 );
	}
	
	if ((scHint!=0) && (scHint->baseAddr!=0))
	{
		/* Make sure the image is gone. */
		tsi_DeAllocMem( scHint->mem, scHint->baseAddr );
		scHint->baseAddr=0L;
	}
}
 

tt_int32 T2K_GetNumAxes(T2K *t)
{
	sfntClass *font;
	tt_int32 numAxes = 0;
	
	font = t->font;
	assert( font != NULL );
	
#ifdef ENABLE_T1
	if ( font->T1 != NULL ) {
		numAxes = font->T1->numAxes;
	}
#endif
#ifdef ENABLE_CFF
	if ( font->T2 != NULL ) {
		numAxes = font->T2->topDictData.numAxes;
	}
#endif
#ifdef ENABLE_T2KE
	if ( font->T2KE != NULL ) {
		numAxes = font->T2KE->numAxes;
	}
#endif
	/* granularity;  location; */
	return numAxes; /*****/
}

F16Dot16 T2K_GetAxisGranularity(T2K *t, tt_int32 n)
{
	F16Dot16 granularity = ONE16Dot16;
	sfntClass *font = t->font;

	assert( font != NULL );
	assert( n >= 0 && n < T2K_GetNumAxes( t ) );
	
#ifdef ENABLE_T2KE
	if ( font->T2KE != NULL ) {
		granularity = font->T2KE->granularity[n];
	}
#endif
	return granularity; /*****/
}

F16Dot16 T2K_GetCoordinate(T2K *t, tt_int32 n )
{
	F16Dot16 coordinate = 0;
	sfntClass *font = t->font;

	assert( font != NULL );
	assert( n >= 0 && n < T2K_GetNumAxes( t ) );
	
#ifdef ENABLE_T2KE
	if ( font->T2KE != NULL ) {
		coordinate = font->T2KE->currentCoordinate[n];
	}
#endif
	return coordinate; /*****/
}

void T2K_SetCoordinate(T2K *t, tt_int32 n, F16Dot16 value )
{
	sfntClass *font = t->font;

	assert( font != NULL );
	assert( n >= 0 && n < T2K_GetNumAxes( t ) );
	
#ifdef ENABLE_T2KE
	if ( font->T2KE != NULL ) {
		font->T2KE->currentCoordinate[n] = value;
	}
#endif
}


#ifdef ENABLE_SBIT
/*
 * Query method to see if a particular glyph exists in sbit format for the current size.
 * If you need to use characterCode then map it to glyphIndex by using T2K_GetGlyphIndex() first.
 */
int T2K_GlyphSbitsExists( T2K *t, tt_uint16 glyphIndex, int *errCode  )
{
	/* no volatile and gcc warns: 'result' might be clobbered by longjmp */
	volatile int result = false; /* Initialize */
	
	tt_uint16 ppemX = (tt_uint16)t->xPixelsPerEm;
	tt_uint16 ppemY = (tt_uint16)t->xPixelsPerEm;
	assert( errCode != NULL );
	if ( (*errCode = setjmp(t->mem->env)) == 0 ) {
		/* try */
		tsi_Assert( t->mem, t->mem->state == T2K_STATE_ALIVE, T2K_ERR_USE_PAST_DEATH );
		/* See if we have an indentity transformation and if the bloc and bdat tables exist */
		if ( t->is_Identity && t->font->bloc != NULL && t->font->bdatOffset != 0 ) {
			result = FindGlyph_blocClass( t->font->bloc, t->font->ebsc, t->font->in, glyphIndex, ppemX, ppemY, &(t->font->bloc->gInfo) );
		}
	} else {
		/* catch */
		tsi_EmergencyShutDown( t->mem );
	}	
	return result; /*****/
}



/*
 * Gets the embeded bitmap
 */
static int T2K_GetSbits(T2K *scaler, tt_int32 code, tt_uint8 greyScaleLevel, tt_uint8 cmd)
{
	int result = false;	/* Initialize */
	blocClass *bloc = scaler->font->bloc;
	ebscClass *ebsc = scaler->font->ebsc;
	
	/* See if we have an indentity transformation and if the bloc and bdat tables exist */
	if ( scaler->is_Identity && bloc != NULL && scaler->font->bdatOffset != 0 ) {
		tt_uint16 ppemX = (tt_uint16)scaler->xPixelsPerEm;
		tt_uint16 ppemY = (tt_uint16)scaler->yPixelsPerEm;
		sbitGlypInfoData *gInfo = &(bloc->gInfo);
		tt_uint16 glyphIndex;
		
		glyphIndex = (tt_uint16)((cmd & T2K_CODE_IS_GINDEX) ? code : T2K_GetGlyphIndex(scaler, (tt_uint16)code ));
		
		result = gInfo->glyphIndex == glyphIndex && ppemX == gInfo->ppemX && ppemY == gInfo->ppemY && gInfo->offsetA != 0;
		if ( !result ) {
			FindGlyph_blocClass( bloc, ebsc, scaler->font->in, glyphIndex, ppemX, ppemY, gInfo );
			result = gInfo->glyphIndex == glyphIndex && ppemX == gInfo->ppemX && ppemY == gInfo->ppemY && gInfo->offsetA != 0;
		}
		if ( result ) {
			ExtractBitMap_blocClass( bloc, ebsc, gInfo, scaler->font->in, scaler->font->bdatOffset, greyScaleLevel, 0 );
		
			scaler->baseAddr	= gInfo->baseAddr;	gInfo->baseAddr	= NULL; /* Hand over the pointer. */
			if ( scaler->baseAddr != NULL ) {
				scaler->xAdvanceWidth16Dot16 = gInfo->bigM.horiAdvance;
				/* scaler->yAdvanceWidth16Dot16 = gInfo->bigM.vertAdvance; */
				scaler->yAdvanceWidth16Dot16 = 0;
				
				scaler->rowBytes		= gInfo->rowBytes;	gInfo->rowBytes	= 0;
				scaler->width			= gInfo->bigM.width;	
				scaler->height			= gInfo->bigM.height;
				
				/* Set horizontal metrics. */
				scaler->horizontalMetricsAreValid = true;
				scaler->fLeft26Dot6		= gInfo->bigM.horiBearingX;	scaler->fLeft26Dot6		<<= 6;
				scaler->fTop26Dot6		= gInfo->bigM.horiBearingY; scaler->fTop26Dot6		<<= 6;
				scaler->xAdvanceWidth16Dot16		= gInfo->bigM.horiAdvance;
				scaler->xAdvanceWidth16Dot16 		<<= 16;
				scaler->xLinearAdvanceWidth16Dot16	= scaler->xAdvanceWidth16Dot16;
				scaler->yAdvanceWidth16Dot16 		= scaler->yLinearAdvanceWidth16Dot16 = 0;
				/* Set vertical metrics. */
				scaler->verticalMetricsAreValid = true;
				scaler->vert_fLeft26Dot6	= gInfo->bigM.vertBearingX;	scaler->vert_fLeft26Dot6	<<= 6;
				scaler->vert_fTop26Dot6		= gInfo->bigM.vertBearingY; scaler->vert_fTop26Dot6		<<= 6;
				scaler->vert_yAdvanceWidth16Dot16		= gInfo->bigM.vertAdvance;
				scaler->vert_yAdvanceWidth16Dot16 		<<= 16;
				scaler->vert_yLinearAdvanceWidth16Dot16	= scaler->vert_yAdvanceWidth16Dot16;
				scaler->vert_xAdvanceWidth16Dot16 		= scaler->vert_xLinearAdvanceWidth16Dot16 = 0;
				
				if ( gInfo->smallMetricsUsed ) {
					if ( !(gInfo->flags & SBIT_SMALL_METRIC_DIRECTION_IS_HORIZONTAL) ) {
						scaler->horizontalMetricsAreValid = false;
					}
					if ( !(gInfo->flags & SBIT_SMALL_METRIC_DIRECTION_IS_VERTICAL) ) {
						scaler->verticalMetricsAreValid = false;
					}
				}
			} else {
				result = false;
			}
		}
	}
	return result; /*****/
}
#endif /* ENABLE_SBIT */

/* this is debug fuction that output bitmap to stdout */
static void dump_bitmap(unsigned char *baseAddr, tt_uint16 width, tt_uint16 height, tt_uint16 rowBytes) {
  int xi, yi, xd;

  if (baseAddr == NULL) {
    printf("Bitmap is NULL\n"); 
    return;
  }

  printf("=========================\n");

  /* 0,0 - is top-left */
  for ( yi = 0; yi < height; yi++ ) {
	for ( xi = 0; xi < width; xi++ ) {
	  xd = xi;

	  /* Paint pixel xi, yi */
	  if ( baseAddr[ xd>>3] & (0x80 >> (xd&7)) ) {
		printf("*");
	  } else {
        printf(" ");
      }
	}
	/* Advance to the next row */
	baseAddr += rowBytes;
    printf("\n");
  }
}

/* check whether point x,y in given bitmap is black */
#define IS_POINT_SET(x, y, rowBytes, baseAddr) (baseAddr[rowBytes*(y)+((x)>>3)] & (0x80 >> ((x)&7)) )
/* mark point x,y as black */ 
#define SET_POINT(x, y, rowBytes, baseAddr) {baseAddr[rowBytes*(y)+((x)>>3)] |= (0x80 >> ((x)&7));}

/******* config data *****/
#define HEIGHT_THRESHOLD_FOR_BITMAP_BOLDING  26  /* below this threshold we use bitmap bolding,
                                    otherwise traditional algorithmic bolding is used 
                                    (number was obtained empirically to have backward compatible 
                                      visual metrics) */
#define HEIGHT_THRESHOLD_NO_WIDENING          9  /* below this threshold we bold bitmap in place
                                    (number was obtained empirically to have backward compatible 
                                      visual metrics) */
 
#define MAX_WIDTH 50 /* maximum width when this algorithm is applicable  
						(used to allocate space for temporary storage only */ 

#define T2K_ABS(v) (v > 0 ? v : -v)

static void bold_bitmap(T2K *t) {
  int xi, yi, xii;
  tt_uint8 columnScore[MAX_WIDTH];      /* loss factors for each of bitmap columns 
                                           characterizing how many pixels in this column 
                                           could not be bolded without widening */
  tt_int32 newRowBytes;
  unsigned char *newBaseAddr; /* placeholder for new bitmap */
  char cand = -1;       /* candidate column to be replicated */ 
  unsigned char offset; /* compensation factor used to map columns from original to 
						   new bitmap (count of inserted column replicas) */
  int widen_by_pixels = 1; /* at the moment we support only 1 and 0 */
  int deltaY = 0, deltaX = 0;

  /* do not bold big bitmaps */
  if (t->width >= MAX_WIDTH) 
	return;

  /* we do not widen very small glyphs to be compatible with earlier releases of jdk 
     NB: for larger sizes we may want to widen by 2 or more pixels */
  if (t->yPixelsPerEm < HEIGHT_THRESHOLD_NO_WIDENING)
     widen_by_pixels = 0; 
   
  /* if we widen bitmap we may need to update advances to prevent glyph overlapping.
     However, we must keep in mind that we want to preserve direction of baseline
     while widening is always performed horizontally.
     To workaround this we use following approach:
     1) if our xAdvance is bigger than yAdvance 
     (i.e. for angle between -PI/4 and PI/4 degree and between 3*PI/4 to 5*PI/4) 
     we stretch xAdvance by 1 pixel and update yAdvance to preserve angle of baseline 
     (using rule xAdvance/yAdvance = deltaX/deltaY)
     2) otherwise we stretch yAdvance by 1 pixel and update xAdvance to keep direction 
     of baseline unchanged
  */    

  if (widen_by_pixels == 1) {
      if (T2K_ABS(t->xAdvanceWidth16Dot16) > T2K_ABS(t->yAdvanceWidth16Dot16)) {
	  deltaY = util_FixDiv(t->yAdvanceWidth16Dot16, t->xAdvanceWidth16Dot16);
	  deltaX = ONE16Dot16;
	  if (t->xAdvanceWidth16Dot16 < 0) { /* pay attention to direction of deltaX */
	      deltaY = -deltaY;
	      deltaX = -deltaX;
	  }        
      } else if (t->yAdvanceWidth16Dot16 != 0) {
	  deltaX = util_FixDiv(t->xAdvanceWidth16Dot16, t->yAdvanceWidth16Dot16);
	  deltaY = ONE16Dot16;
	  if (t->yAdvanceWidth16Dot16 < 0) { /* pay attention to direction of deltaY */
	      deltaY = -deltaY;
	      deltaX = -deltaX;
	  }  
      }
      t->xAdvanceWidth16Dot16 += deltaX;
      t->yAdvanceWidth16Dot16 += deltaY;      
  }

  /* for black&white rasterization NULL bitmap means that we process space
   * glyph. There are no points to bold but we did need to adjust advance. */
  if (t->baseAddr == NULL) 
      return; /* this is space character - nothing to be done */
 
  /* debug output */
  /*  dump_bitmap(t->baseAddr, t->width, t->height, t->rowBytes); 
   */
 
  memset(columnScore, 0, MAX_WIDTH); 
 
  if (widen_by_pixels > 0) {
	/* look for column to be replicated (current implementation  we can replicate at most one column) */
	/* calculate loss factors for every column */
	for(yi=0; yi<t->height; yi++) {
	  for(xi=0; xi<t->width; xi++) {
		/* if we can not perform naive bolding (see below for full description)
		   because addition of black pixel introduce vridge between two other pixels
		   then we increase loss factor for this column by 1 */
		if (IS_POINT_SET(xi, yi, t->rowBytes, t->baseAddr) && 
			((xi == 0 ) 
			 || (xi >= 2 && !IS_POINT_SET(xi-1, yi, t->rowBytes, t->baseAddr)
				 && IS_POINT_SET(xi-2, yi, t->rowBytes, t->baseAddr)))) {
		  columnScore[xi]++;
		}       
	  }
	}
  
	/* dump column scores */ 
	/*  for(xi=0; xi<t->width; xi++) {
		printf("score[%d]=%d\n", xi, columnScore[xi]);
		}
	*/

    /* give column 0 a little higher priority if it's not zero 
       (to prevent losing single pixel on the left edge - 
       see \u4e0a->\u4f55 with "MingLin/12/bold" for example) */
	if (columnScore[0] != 0) {
		columnScore[0]++;
	}

	/* select best column replicate */
	/* giving preference to the left columns -
	   in particular to increase the probability we replicate column 0 */
	for(cand = 0, xi=1; xi<t->width; xi++) {
	  if (columnScore[xi] > columnScore[cand]) {
		  cand = xi;
	  }
	}
    /* printf("Selected column %d(%d)\n", cand, columnScore[cand]); */

    /* if we widen we need to create new bitmap */
	newRowBytes =   (t->width + widen_by_pixels + 7) >> 3; 
	newBaseAddr = (unsigned char*) tsi_AllocMem( t->mem,  newRowBytes*t->height);
	memset(newBaseAddr, 0, t->height*newRowBytes);
  } else { /* i.e. widen_by_pixels == 0 */
	/* if we do not widen bitmap then we can do bolding in place */
	newRowBytes = t->rowBytes;
	newBaseAddr = t->baseAddr;
	cand = -1;
  }

  /* Main bolding logic.
     We work with 2 bitmaps here (they could be the same in we bold in place).
     We scan bitmap from top-left corner and process pixel by pixel
       (xi, yi) is the current point in the original bitmap
	  (xii, yi) is the current pixel in the new bitmap */   
  for(yi=0; yi<t->height; yi++) {
    offset = 0;
    for(xi=0, xii=0; xi<t->width; xi++, xii++) {
	  /* first replicate column if necessary */  
      if (xi == cand) {
 	    /* we want to duplicate the previous column. If cand is 0 we skip as in
		   this case its just the insertion of a new empty column */
        if (xi != 0 && IS_POINT_SET(xi-1, yi, t->rowBytes, t->baseAddr)) {
          SET_POINT(xi, yi, newRowBytes, newBaseAddr);
        }
        offset++;
        xii++;
      } 

      /* start bolding by trying to add black pixel on the left 
         for every black pixel in original bitmap ...  */
      if (IS_POINT_SET(xi, yi, t->rowBytes, t->baseAddr)) {
        if (widen_by_pixels > 0) {
        /* If we widen image then we copy all black pixels 
           from the original image to new one.
           Otherwise we do bolding in place and can avoid copying pixels. */
           SET_POINT(xii, yi, newRowBytes, newBaseAddr);
        }
      /* Bitmap bolding:
         --------------------
         We bold black pixels by seting pixels on previous position to black color.
         How we decide whether we want to bold current black pixel at position xi
           (or xii in the new bitmap)? 
         We do this iff:
          1) we have possibility to bold
              (pixel at position xi-1 is white)
          2) addition of new black pixel will not merge 2 black pixels
              in the same row or in the subsequent rows
             (i.e. pixel at position (xi-2) is not black in this row, one above it and one below
                or xi-2 is out of range)

          Notes:
            We can try to make check for more complicated things in condition 2).
            At the moment we only prohibit effects like 
               (X is the current pixel, square brackets depict the focus of examined neigborhood)
              a)  [* X]  ->   [**X]
                  except the special case of (e.g. see mingliu/14 \u4ebb and mingliu/12 \u4e6a)
                   [*]         [*]   
                  [* X]  ->   [**X]
                   [ *]        [ *]
              b)  [*]    ->   [*]
                  [  X]       [ *X]
              c)  [  X]  ->   [ *X]
                  [*]         [*]
            b) and c) help us to protect from things as  
                    [**]        [**]
                   [* *]  ->   [* *]
                  [*  X]      [* *X]
               but may not help in more complicated cases
            (in above examples we show only effect of bolding last column
    	    		- rest is not shown for simplification)

            Note, that we set pixels on the left only and therefore may break symmetry, e.g.
			    ***    ->    **** 
			   * * *         * * *
	  */
       if (
		   /* if we are replicating this column then we should not bold it */
           (xi+1 != cand) &&
           /* check condition 1 */
            (xii > 0) && 
		   !IS_POINT_SET(xii-1, yi, newRowBytes, newBaseAddr)
           /* check condition 2 */
		   && (xii < 2 
                 /* check for [* X] 
                    with exception of  [*] 
                                      [* X]
                                       [ *]                    */
                 || !((IS_POINT_SET(xii-2, yi, newRowBytes, newBaseAddr) 
                    && (yi < 1 || yi+1 >= t->height
                        || !IS_POINT_SET(xii-1, yi-1, newRowBytes, newBaseAddr)
                        ||  IS_POINT_SET( xi-1, yi+1, t->rowBytes, t->baseAddr)
                        || !IS_POINT_SET( xi,   yi+1, t->rowBytes, t->baseAddr)))     
                 /* check for [*]
                            [  X] */
                 || (yi > 0 
                       &&  IS_POINT_SET(xii-2, yi-1, newRowBytes, newBaseAddr)
                       && !IS_POINT_SET(xii-1, yi-1, newRowBytes, newBaseAddr)) 
                 /* check for [  X]
                              [*] 
                    Important: we use original matrix and therefore must  
                        process situation when previous column was replicated 
                        in the special way 
                        (because both (xii-1) and (xii-2) will correspond to (xi-1)!) */
                 /* NB: if we DO BOLD replicated columns this condition must be reworked 
                       to take into account new bitmap too. */ 
                 || (xi != cand && yi+1 < t->height 
                       &&  IS_POINT_SET(xi-2, yi+1, t->rowBytes, t->baseAddr)
                       && !IS_POINT_SET(xi-1, yi+1, t->rowBytes, t->baseAddr)))
           )) {
		  SET_POINT(xii-1, yi, newRowBytes, newBaseAddr);
        }
      }
    }
  }

  if (widen_by_pixels > 0) {
	t->width += widen_by_pixels;
	t->rowBytes = newRowBytes;
	tsi_DeAllocMem(t->mem, t->baseAddr);
	t->baseAddr = newBaseAddr;
  }

/* original naive bitmap bolding without widening */
/*  for(yi=0; yi<t->height; yi++) {
      for(xi=1; xi<t->width; xi++) {
       if (IS_POINT_SET(xi, yi, t->rowBytes) && 
           !IS_POINT_SET(xi-1, yi, t->rowBytes)
          && (xi<2 || !IS_POINT_SET(xi-2, yi, t->rowBytes))) {
	       SET_POINT(xi-1, yi, t->rowBytes);
      }       
    }
  }
*/
  
  /*   dump_bitmap(t->baseAddr, t->width, t->height, t->rowBytes); */
}


/*
 *
 */
void T2K_RenderGlyph( T2K *t, tt_int32 code, tt_int8 xFracPenDelta, tt_int8 yFracPenDelta, tt_uint8 greyScaleLevel, tt_uint8 cmd, int *errCode )
{
	int doBitmapBolding = 0;
	F16Dot16 savedStyleFactor;
	assert( errCode != NULL );
	if ( (*errCode = setjmp(t->mem->env)) == 0 ) {
		/* try */
		tsi_Assert( t->mem, t->mem->state == T2K_STATE_ALIVE, T2K_ERR_USE_PAST_DEATH );
		assert( !( (cmd & T2K_GRID_FIT) && (cmd & T2K_TV_MODE)) ); /* If you turn on T2K_TV_MODE, then please turn off T2K_GRID_FIT */

		if(t->font->StyleFuncPost != NULL && greyScaleLevel == BLACK_AND_WHITE_BITMAP 
		   && t->yPixelsPerEm < HEIGHT_THRESHOLD_FOR_BITMAP_BOLDING  && t->font->params[0] != ONE16Dot16) {
		      doBitmapBolding = 1;
		      savedStyleFactor  = t->font->params[0];
		      t->font->params[0] = ONE16Dot16;
		}

#ifdef ENABLE_SBIT
		if ((cmd & T2K_RETURN_OUTLINES) == 0 && t->enableSbits && T2K_GetSbits( t, code, greyScaleLevel, cmd )  ) {
			t->embeddedBitmapWasUsed 		= true;
		} else
#endif /* ENABLE_SBIT */
		{
			t->embeddedBitmapWasUsed		= false;
				/*  ORIENTBOLD_STYLES */
 				t->theContourData.active=0;	/* normally its off. */
				if ( 
						(t->theContourData.initializedContour) 			/* If the data structure is in place. */
						&&
						(t->font->StyleFuncPost)
					)
				{
					/* make it active and start collecting contour data. */
					t->theContourData.active=1;
					t->theContourData.current=0;
					InitializeDefaultContourData(&t->theContourData);
				}

			T2K_RenderGlyphInternal( t, code, xFracPenDelta, yFracPenDelta, greyScaleLevel, cmd,0  );
			/*  ORIENTBOLD_STYLES */			
		if (t->theContourData.active)
				VerifyContourUsage(&t->theContourData);
 			/*  ORIENTBOLD_STYLES */			
			t->horizontalMetricsAreValid	= true;
			t->verticalMetricsAreValid		= false;
		}
        if (doBitmapBolding) {
            t->font->params[0] = savedStyleFactor;
            bold_bitmap(t);
            /* we need to apply bolding to outline to get correct advances */
            if (cmd & T2K_RETURN_OUTLINES) {
               /* be sure to not apply italic twice! */
               savedStyleFactor  = t->font->params[1];
               t->font->params[1] = 0x0;
               ApplyPostStyle(t->glyph, t);
               t->font->params[1] = savedStyleFactor;
            }
        }
	} else {
		/* catch */
		tsi_EmergencyShutDown( t->mem );
	}
}


/*
 *
 */
void DeleteT2K( T2K *t, int *errCode )
{
	assert( errCode != NULL );
	if ( (*errCode = setjmp(t->mem->env)) == 0 ) {
		/* try */
		tsi_Assert( t->mem, t->stamp1 == T2K_MAGIC1 && t->stamp2 == T2K_MAGIC2, T2K_ERR_BAD_T2K_STAMP  );
		/* Release all true type hinting. (reverse of allocation ) */
#ifdef ENABLE_TT_HINTING
		ReleaseTTHintTranForT2K(t);
		ReleaseTTHintFontForT2K(t);
#endif  
		/*  ORIENTBOLD_STYLES */
		if(t->theContourData.initializedContour)
			ReleaseContourData( t->mem, &t->theContourData);
 		T2K_PurgeMemoryInternal( t, 2 );

		tsi_DeAllocMem( t->mem, t->font->globalHintsCache );
		t->font->globalHintsCache = NULL;
		tsi_DeAllocMem( t->mem, t );
	} else {
		/* catch */
		tsi_EmergencyShutDown( t->mem );
	}
}

#ifdef ENABLE_LINE_LAYOUT

tt_uint16 T2K_GetGlyphIndex( T2K *t, tt_uint16 charCode )
{
    if (t == NULL) {
        return 0;
    }
    return GetSfntClassGlyphIndex( t->font, charCode ); /*****/
}



tt_uint32 T2K_MeasureTextInX(T2K *t, const tt_uint16 *text, tt_int16 *xKernValuesInFUnits, tt_uint32 numChars )
{
	tt_uint32 i, totalWidth, thisWidth;
	tt_uint16 glyphIndex, prevGlyphIndex;
	tt_uint16 charCode, prevCharCode;
	tt_uint16 *awArr;
#ifdef LAYOUT_CACHE_SIZE
	tt_uint32 cachePos, cacheTag;
#endif
#ifdef ENABLE_KERNING
	tt_int16 xKern, yKern;
#endif
	
	assert( t != NULL );
	assert( t->font != NULL );
	assert( t->font->hmtx != NULL );
	assert( xKernValuesInFUnits != NULL );
	totalWidth		= 0;
	awArr			= t->font->hmtx->aw;
	glyphIndex 		= 0xffff;
	prevCharCode	= 32; /* space character */
	prevGlyphIndex	= 0xffff;
	for ( i = 0; i < numChars; i++ ) {
		charCode = text[i];
		
#ifdef LAYOUT_CACHE_SIZE
		cachePos   = prevCharCode;
		cachePos <<= 4;
		cachePos  ^= charCode;
		cachePos  %= LAYOUT_CACHE_SIZE;
		
		cacheTag   = prevCharCode;
		cacheTag <<= 16;
		cacheTag  |= charCode;
		if ( t->tag[cachePos] == cacheTag ) {
			thisWidth	= t->kernAndAdvanceWidth[ cachePos ];
#ifdef ENABLE_KERNING
			xKern		= t->kern[ cachePos ];
#endif
			glyphIndex	= 0xffff;
		} else {
#endif	/* LAYOUT_CACHE_SIZE */	
			glyphIndex	= GetSfntClassGlyphIndex( t->font, charCode );
			thisWidth	= awArr[ glyphIndex ];
			#ifdef ENABLE_KERNING
			{
				if ( prevGlyphIndex == 0xffff ) {
					prevGlyphIndex = GetSfntClassGlyphIndex( t->font, prevCharCode );
				}
				GetSfntClassKernValue( t->font, prevGlyphIndex, glyphIndex, &xKern, &yKern );
				thisWidth += xKern;
			}
			#endif /* ENABLE_KERNING */	
#ifdef LAYOUT_CACHE_SIZE
			/* cache the results */
			t->tag[cachePos]					= cacheTag;
			t->kernAndAdvanceWidth[ cachePos ]	= (short)thisWidth;
#ifdef ENABLE_KERNING
			t->kern[ cachePos ]					= xKern;
#endif
		}
#endif	/* LAYOUT_CACHE_SIZE */
#ifdef ENABLE_KERNING
		xKernValuesInFUnits[i]	= xKern; /* in fUnits */
#else
		xKernValuesInFUnits[i]	= 0; 
#endif
		totalWidth		+= thisWidth;
		prevGlyphIndex	 = glyphIndex;
		prevCharCode	 = charCode;
	}
	/* now scale to pixels */
	totalWidth = util_FixMul( totalWidth, t->xMul );
	return totalWidth; /*****/
}


#ifdef ENABLE_KERNING
void T2K_GetIdealLineWidth( T2K *t,         const T2KCharInfo cArr[], tt_int32 lineWidth[], T2KLayout out[] )
#else
void T2K_GetIdealLineWidth( T2K *UNUSED(t), const T2KCharInfo cArr[], tt_int32 lineWidth[], T2KLayout out[] )
#endif
{
	int i;
	const T2KCharInfo *cd;
	tt_int32 totalIntWidthX, totalIntWidthY;
	F16Dot16 totSumX, totSumY;
	tt_uint16 prevIndex = 0;

	totalIntWidthX = totalIntWidthY = 0;
	totSumX = totSumY = 0;
	for ( i = 0; (cd = &cArr[ i ])->charCode != 0 ; i++ ) {
		totSumX += cd->LinearAdvanceWidth16Dot16[ T2K_X_INDEX ];
		totSumY += cd->LinearAdvanceWidth16Dot16[ T2K_Y_INDEX ];
		/* Initialize to the non-linear metrics */
		out[i].BestAdvanceWidth16Dot16[ T2K_X_INDEX ] = cd->AdvanceWidth16Dot16[ T2K_X_INDEX ];
		out[i].BestAdvanceWidth16Dot16[ T2K_Y_INDEX ] = cd->AdvanceWidth16Dot16[ T2K_Y_INDEX ];
#ifdef ENABLE_KERNING
		if ( i != 0 ) {
			tt_int16 xKern, yKern;
			F16Dot16 delta;
			GetSfntClassKernValue( t->font, prevIndex, cd->glyphIndex, &xKern, &yKern );
			if ( xKern != 0 ) {
				delta = xKern; delta <<= 16;
				delta = util_FixMul( delta, t->xMul );
				totSumX += delta;
				/* truncate */
				delta &= 0xffff0000;
				out[i].BestAdvanceWidth16Dot16[ T2K_X_INDEX ] += delta;
			}
			if ( yKern != 0 ) {
				delta = yKern; delta <<= 16;
				delta = util_FixMul( delta, t->yMul );
				totSumY += delta;
				/* truncate */
				delta &= 0xffff0000;
				out[i].BestAdvanceWidth16Dot16[ T2K_Y_INDEX ] += delta;
			}
		}
#endif /* ENABLE_KERNING */		
		totalIntWidthX += totSumX >> 16; totSumX &= 0x0000ffff;
		totalIntWidthY += totSumY >> 16; totSumY &= 0x0000ffff;
		prevIndex = cd->glyphIndex;
	}
	lineWidth[ T2K_X_INDEX ] = totalIntWidthX;
	lineWidth[ T2K_Y_INDEX ] = totalIntWidthY;
}

#ifdef ENABLE_KERNING
/*
 * Return value is a pointer to T2K_KernPair with *pairCountPtr entries.
 * The entries consist of all kern pairs between the the character with
 * the charCode character code combined with itself and all the members
 * of baseSet. (A character should only appear once in baseSet)
 * The caller *has* to deallocate the pointer, if != NULL, with
 * tsi_DeAllocMem( t->mem, pointer );
 */
T2K_KernPair *T2K_FindKernPairs( T2K *t, tt_uint16 *baseSet, int baseLength, tt_uint16 charCode, int *pairCountPtr )
{
	register T2K_KernPair *pairs = NULL;
	register sfntClass *font = t->font;
	register int i, pairCount = 0;
	tt_int16 xKern, yKern;
	tt_uint16 glyphIndexA, glyphIndexB;
	
	glyphIndexA	= GetSfntClassGlyphIndex( font, charCode );
	GetSfntClassKernValue( font, glyphIndexA, glyphIndexA, &xKern, &yKern ); /* Check charCode-charCode */
	if ( xKern != 0 || yKern != 0 ) {
		pairCount++;
	}
	/* Allocate memory for worst (largest) case */
	pairs = (T2K_KernPair *)tsi_AllocMem( t->mem, sizeof(T2K_KernPair) * (baseLength+baseLength+pairCount) );
	
	if ( pairCount != 0 ) {
		pairs[0].left  = charCode;
		pairs[0].right = charCode;
		pairs[0].xKern = xKern;
		pairs[0].yKern = yKern;
	}
	
	for ( i = 0; i < baseLength; i++ ) {
		glyphIndexB = GetSfntClassGlyphIndex( font, baseSet[i] );
		GetSfntClassKernValue( font, glyphIndexA, glyphIndexB, &xKern, &yKern );
		if ( xKern != 0 || yKern != 0 ) {
			pairs[pairCount].left  = charCode;
			pairs[pairCount].right = baseSet[i] ;
			pairs[pairCount].xKern = xKern;
			pairs[pairCount].yKern = yKern;
			pairCount++;
		}
		GetSfntClassKernValue( font, glyphIndexB, glyphIndexA, &xKern, &yKern );
		if ( xKern != 0 || yKern != 0 ) {
			pairs[pairCount].left  = baseSet[i];
			pairs[pairCount].right = charCode;
			pairs[pairCount].xKern = xKern;
			pairs[pairCount].yKern = yKern;
			pairCount++;
		}
	}
	if ( pairCount != 0 ) {
		pairs = (T2K_KernPair *)tsi_ReAllocMem( t->mem, pairs, sizeof(T2K_KernPair) * pairCount );
	} else {
		tsi_DeAllocMem( t->mem, pairs );
		pairs = NULL;
	}
	
	*pairCountPtr = pairCount;
	return pairs; /*****/
}
#endif /* ENABLE_KERNING */		

void T2K_LayoutString( const T2KCharInfo cArr[], const tt_int32 LineWidthGoal[], T2KLayout out[] )
{
	int i, j, MY_INDEX;
	const T2KCharInfo *cd;
	tt_int32 totalIntWidth;
	F16Dot16 fracSum, spaceAdvance = 0;
	tt_int32 error, goal, delta, deltaI, spaceCount, strLen;

	
	
	/* Set to the dimension that we will tweak, the other we will just scale */
	if ( LineWidthGoal[ T2K_X_INDEX ] >= LineWidthGoal[ T2K_Y_INDEX ] ) {
		goal = LineWidthGoal[ T2K_X_INDEX ];
		MY_INDEX = T2K_X_INDEX;
	} else {
		goal = LineWidthGoal[ T2K_Y_INDEX ];
		MY_INDEX = T2K_Y_INDEX;
	}
	totalIntWidth = 0; fracSum = 0;
	for ( spaceCount = 0, strLen = 0, i = 0; (cd = &cArr[ i ])->charCode != 0; i++ ) {
		/*
		 already done
		out[i].BestAdvanceWidth16Dot16[ T2K_X_INDEX ] = cd->AdvanceWidth16Dot16[ T2K_X_INDEX ];
		out[i].BestAdvanceWidth16Dot16[ T2K_Y_INDEX ] = cd->AdvanceWidth16Dot16[ T2K_Y_INDEX ];
		*/
		fracSum += out[i].BestAdvanceWidth16Dot16[ MY_INDEX ];
		totalIntWidth += fracSum >> 16; fracSum &= 0x0000ffff;
		if ( cd->charCode == 32 ) {
			spaceCount++;
			spaceAdvance = cd->LinearAdvanceWidth16Dot16[ MY_INDEX ];
		}
		strLen++;
	}
	error = totalIntWidth - goal;
	if ( strLen == 0 ) return; /*****/
	
	
	if ( error > 0 ) {
		deltaI = -1;
		delta  = -ONE16Dot16;
	} else {
		deltaI = 1;
		delta  = ONE16Dot16;
	}
	if ( spaceCount > 0 ) {
		tt_int32 tmp 		= spaceAdvance;
		tt_int32 minSpace	= spaceAdvance/2 + 1;
		tt_int32 maxSpace	= spaceAdvance*4;
		while ( error != 0 && tmp >= minSpace && tmp <= maxSpace) {
			for ( i = 0; i < strLen; i++ ) {
				cd = &cArr[ i ];
				if ( cd->charCode == 32 ) {
					tmp = out[i].BestAdvanceWidth16Dot16[ MY_INDEX ] + delta;
					if ( tmp < minSpace || tmp > maxSpace ) break; /*****/
					out[i].BestAdvanceWidth16Dot16[ MY_INDEX ] = tmp;
					error += deltaI;
					if ( error == 0 ) break; /*****/
				}
			}
		}
	}
	if ( error >= strLen || error <= -strLen ) {
		int mul = error/strLen;
		if ( mul < 0 ) mul = -mul;
		for ( i = 0; i < strLen; i++ ) {
			cd = &cArr[ i ];
			out[i].BestAdvanceWidth16Dot16[ MY_INDEX ] += delta*mul;
			error += deltaI*mul;
		}
	}
	/* assert( error < strLen && error > -strLen ); */

	if ( error != 0 ) {
		int absError = error > 0 ? error : -error;
		int inc = strLen / ( absError + 1 );
		
		
		for ( i = inc>>1; error != 0; i = i % strLen ) {
			cd = &cArr[ i ];
			if ( out[i].BestAdvanceWidth16Dot16[ MY_INDEX ] > 0 ) {
				out[i].BestAdvanceWidth16Dot16[ MY_INDEX ] += delta;
				error += deltaI;
				i += inc;
			} else {
				i++;
			}
		}
	}
	
	/* Scale the other dimension(s) */
	for ( j = 0; j < T2K_NUM_INDECES; j++ ) {
		if ( j == MY_INDEX ) continue; /*****/
		for ( i = 0; i < strLen; i++ ) {
			F16Dot16 scaleFactor;
			scaleFactor = cd->AdvanceWidth16Dot16[ j ] > 0 ? util_FixDiv( out[i].BestAdvanceWidth16Dot16[ MY_INDEX ], cd->AdvanceWidth16Dot16[ MY_INDEX ] ) : 0;
			out[i].BestAdvanceWidth16Dot16[ j ] = util_FixMul( out[i].BestAdvanceWidth16Dot16[ j ], scaleFactor );
		}
	}
}


#endif /* ENABLE_LINE_LAYOUT */
#endif /* ENABLE_AUTO_GRIDDING */
