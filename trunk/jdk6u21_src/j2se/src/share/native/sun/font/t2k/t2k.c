/*
 * @(#)t2k.c	1.54 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

/* Handy macros to calculate result of transformation of vector (x, y) by matrix T (i.e. T*(x, y)) */
#define CALCULATE_TRANSFORMED_X(x, y, T) (util_FixMul(x, (T)->t00) + util_FixMul(y, (T)->t01))
#define CALCULATE_TRANSFORMED_Y(x, y, T) (util_FixMul(x, (T)->t10) + util_FixMul(y, (T)->t11))

/* does current transform flip axes? */
#define WILL_FLIP_AXES(t)  ((t)->compensation.t00 == 0)


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

        t->compensation.t00 = t->compensation.t11 = ONE16Dot16;
        t->compensation.t01 = t->compensation.t10 = 0;

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

/* Checks if given transformation matrix flips axes (or changes contour orientation) */
#define CHANGES_ORIENTATION(t) \
    ((util_FixMul((t)->t00, (t)->t11) - util_FixMul((t)->t01, (t)->t10)) < 0) 

/* Extract transform that later will be applied to result of rasterization
    (bitmap, outline and metrics) to ensure consistency with regard to 
    quadrant transforms/mirrors.
   I.e. 8 possible (quadrant) rotation/flip transform matrices are mapped to same "canonic"
    compensation matrix and some remainder matrix. Formally
         orig = compensation * remainder     
   Note that original transform is changed in place ro remainder transform, i.e.
      orig = remainder        
      
   See comment for T2K_NewTransformation() for more details.
 */
static void ExtractBitmapTransform(T2K_TRANS_MATRIX *orig, T2K_TRANS_MATRIX *compensation) {
    F16Dot16 tmp;

    /* To identify compensation required we use following approach: 

         - check direction of transformed (0, 1) vector 
           and determine quadrant angle needed to rotate it to first quadrant
           Rules to identify counter-clockwise rotation angle are following 
           (assuming (x, y) is transformed version of (0,1)):
              x >= 0, y >  0 =>   0 degrees
              x <  0, y >= 0 =>  90 degrees
              x <= 0, y <  0 => 180 degrees
              x >  0, y <= 0 => 270 degrees

         - check orientation of matrix of transformed basis vectors
           to see if axes were flipped
           (If transform includes flip then orientation of basis vectors will change
            and therefore determinant of transform matrix should be negative.)

       Note that result of applying transformation orig to vector (0, 1) 
         is (orig->t10, orig->t11) 
    */
    if (orig->t01 >= 0 && orig->t11 > 0) { /* 0 degree */
        /* No compensation is required. */
        compensation->t00 = compensation->t11 = ONE16Dot16;
        compensation->t01 = compensation->t10 = 0;
    } else if (orig->t01 < 0 && orig->t11 >= 0) { /* 90 degrees  */
        /* 
           rotate orig by -90 and set compensation to rotation by 90, i.e.
             [a b c d] = [0 -1 1 0]*([0 1 -1 0]*[a b c d]) = [0 -1 1 0]*[c d -a -b]) 
        */
        tmp = orig->t00;
        orig->t00 = orig->t10;
        orig->t10 = -tmp;
        tmp = orig->t01;
        orig->t01 = orig->t11;
        orig->t11 = -tmp;

        compensation->t00 = compensation->t11 = 0;
        compensation->t01 = -ONE16Dot16;
        compensation->t10 = ONE16Dot16;

    } else if (orig->t01 <= 0 && orig->t11 < 0) { /* 180 degrees  */
        /* 
           rotate orig by -180 and set compensation to rotation by 180 
             [a b c d] = [-1 0 0 -1]*([-1 0 0 -1]*[a b c d]) = [-1 0 0 -1]*[-a -b -c -d])
        */
        orig->t00 = -orig->t00;
        orig->t01 = -orig->t01;
        orig->t10 = -orig->t10;
        orig->t11 = -orig->t11;

        compensation->t00 = compensation->t11 = -ONE16Dot16;
        compensation->t01 = compensation->t10 = 0;
        
    } else if (orig->t01 > 0 && orig->t11 <= 0) { /* 270 degrees  */
        /*
           rotate orig by -270 (same as 90) and set compensation to rotation by 270
             [a b c d] = [0 1 -1 0]*([0 -1 1 0]*[a b c d]) = [0 1 -1 0]*[-c -d a b])
        */
        tmp = orig->t00;
        orig->t00 = -orig->t10;
        orig->t10 = tmp;
        tmp = orig->t01;
        orig->t01 = -orig->t11;
        orig->t11 = tmp;

        compensation->t00 = compensation->t11 = 0;
        compensation->t01 = ONE16Dot16;
        compensation->t10 = -ONE16Dot16;

    }


    /* 
       Now we check if orig is flipped.
       
       Note that if matrix flips axes we need to add flip over one of axes only.
       We do flip over y axis (i.e. (x,y) => (-x, y)) because at previous step 
        we were trying to coinside y unit vector with y axis.
        
       We want to flip axes in the new orig matrix but we want to keep 
         compensation*orig
       the same. Therefore, we invert sign of elements in first row of orig
       and to compensate this we need to invert sign in first column of compensation.
    */    
    if (CHANGES_ORIENTATION(orig)) {
        /* 
            flip both orig and compenstation matrices 
        */
        orig->t01 = -orig->t01;
        orig->t00 = -orig->t00;

        compensation->t10 = -compensation->t10;
        compensation->t00 = -compensation->t00;
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

/* Transforms xInFUnits into 16Dot16 x and y values
   by applying part of glyph transform (scaled by UPEM).
   I.e. compensation and (t->t00/t->t01/t->10/t->t11) are applied. */
void T2K_TransformXFunits(T2K *t, short xValueInFUnits, F16Dot16 *x, F16Dot16 *y) {
    F16Dot16 tmpX, tmpY;

    tmpX = util_FixMul(xValueInFUnits << 16, t->xMul);

    tmpY = util_FixMul(t->t10, tmpX); /* we are applying transform to (tmpX, 0) */
    tmpX = util_FixMul(t->t00, tmpX); 
    
    *x = CALCULATE_TRANSFORMED_X(tmpX, tmpY, &t->compensation);
    *y = CALCULATE_TRANSFORMED_Y(tmpX, tmpY, &t->compensation);
}

/* Transforms yInFUnits into 16Dot16 x and y values
   by applying part of glyph transform (scaled by UPEM).
   I.e. compensation and (t->t00/t->t01/t->10/t->t11) are applied. */
void T2K_TransformYFunits(T2K *t, short yValueInFUnits, F16Dot16 *x, F16Dot16 *y) {
    F16Dot16 tmpX, tmpY;

    tmpY = util_FixMul(yValueInFUnits << 16, t->yMul);

    tmpX = util_FixMul(t->t01, tmpY); /* we are applying transform to (0, tmpY) */
    tmpY = util_FixMul(t->t11, tmpY); 
    
    *x = CALCULATE_TRANSFORMED_X(tmpX, tmpY, &t->compensation);
    *y = CALCULATE_TRANSFORMED_Y(tmpX, tmpY, &t->compensation);
}


/*
 * Returns dropout mode - one of DOCONTROL_DISABLED, DOCONTROL_FULL,
 *  DOCONTROL_SMART, DOCONTROL_SMART_NOSTUBS or DOCONTROL_NOSTUBS.
 *
 * There are two factors contributing to decision which mode should be used.
 * 
 * 1) T2K has global preferences that used to enable or disable dropout
 *  control depending on font size. These preferences are the same for ALL
 *  fonts.  They are disabled at least since introduction of Truetype
 *  hinting support but they probably could be usefull for cases where
 *  explicit settings are missing (e.g. Type1 fonts).
 *
 * 2) Glyph-specific flags (glyphDropOutControlFlags) defining dropout mode
 *  for current glyph. These flags are native for Truetype where control is
 *  performed by SCANTYPE and SCANCTRL hinting instructions. For other fonts
 *  or if these instructions are missing default logic is used.
 * 
 * glyphDropOutControlFlags has following structure:
 *
 * Upper 16 bits specify dropout control mode (see SCANTYPE instruction in
 * the Truetype specification). Meaning of values is following:
 *   0  -  simple dropout control
 *   1  -  simple dropout control excluding stubs
 *   2  -  no dropout control
 *   4  -  smart dropout control
 *   5  -  smart dropout control excluding stubs
 *   3, 6 or 7 - same as 2
 *
 *   Note that modes 4 and 5 were introduced by Microsoft and are not described 
 *   in truetype specification from Apple. 
 *
 * Lower 16 bits define various conditions to be met to enable dropout control.
 * (see SCANCTRL instruction in the truetype specification for details).
 * Three base conditions are: Is the glyph rotated?, is the glyph stretched?,
 * is the current pixels per Em less than a specified threshold?
 * These conditions can be OR'd or ANDed together to determine whether the
 * dropout control mode ought to be used.
 * 
 * Six bits are used to specify the joint condition. Their meanings are:
 * 
 * BIT        Meaning if set
 * 8        Do dropout mode if other conditions don't block it AND
 *             pixels per em is less than or equal to bits 0-7
 * 9        Do dropout mode if other conditions don't block it AND
 *             glyph is rotated
 * 10        Do dropout mode if other conditions don't block it AND
 *             glyph is stretched
 * 11        Do not do dropout mode unless ppem is less than or equal to bits 0-7
 *             A value of FF in 0-7 means all sizes
 *             A value of 0 in 0-7 means no sizes
 * 12        Do not do dropout mode unless glyph is rotated    
 * 13        Do not do dropout mode unless glyph is stretched
 *             
 * In other words, we do not do dropout control if:
 * No bits are set,
 * Bit 8 is set, but ppem is greater than threshold
 * Bit 9 is set, but glyph is not rotated
 * Bit 10 is set, but glyph is not stretched
 * None of the conditions specified by bits 11-13 are true.
 * Dropout control mode in upper 16 bits is set to 2 (No dropout control)
 * 
 * For example, 0xA10 specifies turn dropout control on if the glyph is
 * rotated providing that it is also less or equal than 0x10 pixels per em.
 * (Note: MS version of TT spec states "<" in this example and this 
 *  contradicts with above rules.)	
 *
 * Note that glyph is considered stretched if the X and Y resolutions are
 * different either because of the device characteristics or because of the
 * transformation matrix.  If both X and Y are changed by the same factor
 * the glyph is not considered stretched.
 * 
 */

#define IS_ROTATED(t)     (t->t01 || t->t10)
#define IS_STRETCHED(t)   (t->t00 != t->t11 || IS_ROTATED(t))

static int getDropoutMode(T2K *t, int greyscale) {
    int dropoutMode = DOCONTROL_DISABLED;  /* disabled by default */

    if (greyscale > 0) { /* AA text - NB: dropout is not actually supported as it is */
#ifdef EnableGlobalDropoutPreferences
         /* if either dimension is too small or too big, then no dropout control */
         if ((t->xPixelsPerEm < MinAntiAliasDropoutControl) 
          || (t->yPixelsPerEm < MinAntiAliasDropoutControl) 
          || (t->xPixelsPerEm > MaxAntiAliasDropoutControl) 
          || (t->yPixelsPerEm > MaxAntiAliasDropoutControl)) {
             return DOCONTROL_DISABLED;
         }
         /* if one of dimensions is within the critical range, then do enable dropout */
         if ((t->xPixelsPerEm <= MaxCriticalAntiAliasDropoutControl)
          || (t->yPixelsPerEm <= MaxCriticalAntiAliasDropoutControl)) {
             return DOCONTROL_FULL; 
         }
#endif
        return dropoutMode; /* at the moment we ignore font instructions for AA text */
    } else { /* black & white */ 
#ifdef EnableGlobalDropoutPreferences
        /* if either dimension is too small or too big, then no dropout control */
         if ((t->xPixelsPerEm < MinT2KDropoutControl) 
          || (t->yPixelsPerEm < MinT2KDropoutControl) 
          || (t->xPixelsPerEm > MaxT2KDropoutControl) 
          || (t->yPixelsPerEm > MaxT2KDropoutControl)) {
             return DOCONTROL_DISABLED; 
         }
         /* if one of dimensions is within the critical range, then do enable dropout */
         if ((t->xPixelsPerEm <= MaxCriticalT2KDropoutControl)
          || (t->yPixelsPerEm <= MaxCriticalT2KDropoutControl)) {
             return DOCONTROL_FULL; 
         }
#endif     
    }

    /* Check dropout mode (set by SCANTYPE - see TrueType spec for details) */
    switch (t->glyphDropOutControlFlags >> 16) {
        case 0:
            dropoutMode = DOCONTROL_FULL;
            break;
        case 1:
            dropoutMode = DOCONTROL_NOSTUBS;
            break;
        case 4:
            dropoutMode = DOCONTROL_SMART;
            break;
        case 5: 
            dropoutMode = DOCONTROL_SMART_NOSTUBS;
            break;
        case 2: /* no dropout */
        default:
            return DOCONTROL_DISABLED;
    }

    /* Check blocking conditions first */

    /* No bits are set */
    if (!(t->glyphDropOutControlFlags & 0x3F00)) {
        return DOCONTROL_DISABLED;
    }

    /* Bit 11 is set => Do not do dropout mode unless ppem is less than or equal to bits 0-7 */
    if ((t->glyphDropOutControlFlags & 0x800) 
       && (t->yPixelsPerEm > (t->glyphDropOutControlFlags & 0xFF))) {
        return DOCONTROL_DISABLED;
    }
    /* Bit 12 is set => Do not do dropout mode unless glyph is rotated */
    if ((t->glyphDropOutControlFlags & 0x1000) && !IS_ROTATED(t)) {
        return DOCONTROL_DISABLED;
    }
    /* Bit 13 is set => Do not do dropout mode unless glyph is stretched */
    if ((t->glyphDropOutControlFlags & 0x2000) && !IS_STRETCHED(t)) {
        return DOCONTROL_DISABLED;
    }

        
    /* None of blocking conditions are true. Check if we can enable dropout. */
    
    /* Do dropout mode if other conditions don't block it AND ppem is not greater than threshold */   
    if ((t->glyphDropOutControlFlags & 0x100) 
       && (((t->glyphDropOutControlFlags & 0xFF) == 0xFF ) /* special case: ALL sizes */
           || t->yPixelsPerEm <= (t->glyphDropOutControlFlags & 0xFF))) {
        return dropoutMode;
    }
    /* Bit 9 is set and glyph is rotated */
    if ((t->glyphDropOutControlFlags & 0x200) && IS_ROTATED(t)) {
        return dropoutMode;
    }
    /* Bit 10 is set and glyph is stretched */
    if ((t->glyphDropOutControlFlags & 0x400) && IS_STRETCHED(t)) {
        return dropoutMode;
    }
    
    return DOCONTROL_DISABLED;
}

/*
  Handling transforms we want to:
    - benefit from hinting as much as possible
    - keep results consistent for quadrant rotations and mirrors
      (naturally people expect text flipped over axis X look
       the same as flipped bitmap of non-rotated text)
    - make sure hinting is disabled for cases when we can not handle
       even simplified transform matrix
      (e.g. for very small strecth factors)

  Our approach to archieve this is following:
    a. decompose original transform to bitmap transform (combination
       of quadrant rotation and mirror) and remainder transform
    b. further decompose remainder transform to safe hinting transform
       and outline compensation transform
    c. perform hinting with safe transform
    d. apply compensation transform to hinted outline
    e. perform scan convertion
    f. apply bitmap transform

  Design considerations:
    - both truetype and type1 hinting can only handle positive stretch transforms
      and may be confused by other types of transforms
    - result of scan conversion of symmetrical shapes may be different
      due to dropout control rules
      (that's why we need to transform bitmaps!)
    - rotations that swap x and y axes can not be handled perfectly in case
      of subpixel rendering because subpixels are available only for one axis
    - in some cases (e.g. very small stretch factor) execution of hinting
      instructions should be disabled but we may still need to alter outline
      (e.g. perform algorithmic styling)
    - advance width AND position of top-left point might change as a result of
      bitmap transform. Adjustments have to made even if we return outline only.
    - composite glyphs have embeded transforms but because scan conversion
      is performed AFTER grid fitting of all subglyphs these transforms
      should not affect bitmap transformation

  Note that hinting engines (neither truetype nor type1) do not scale outlines.
  They assume outline is scaled for them but they still need scaling transform 
  for reference - e.g. some hinting instructions depends on ppem, 
  proportions are important, etc.

  Hinting engines also do not support arbitrary transformations. Hinting 
  (it is not just this implementation, its assumption of hinting process)
  is designed to work with transforms they keep axis, i.e.
  they can only handle positive stretch transforms.

  At the moment outline scaling is done in the following way:
    a. Stretch unhinted outline in T2K_RenderGlyphInternal based on values of 
        xPixelsPerEm and yPixelsPerEm (before hinting)
    b. Apply remaining transformation to hinted outline in T2K_RenderGlyphInternal
        using t->t00/t->t01/t->t10/t->t11 (after hinting)
    c. Apply quadrant rotation/flip compensation transform in T2K_RenderGlyph()

  Embeded bitmaps can be used if following is true:
    - there are embeded bitmaps of required size
    - requested transform can be performed on bitmap without loss
  Due to nature of compensation matrix it can be safely applied to bitmap.
  So, if remaining transform is identity transform then bitmaps can be used
  (is_Identity flag used to signal this).
*/
void T2K_NewTransformation( T2K *t, int doSetUpNow, tt_int32 xRes, tt_int32 yRes, T2K_TRANS_MATRIX *trans, int enableSbits, int *errCode )
{
	F16Dot16 xPointSize, yPointSize;
	F16Dot16 xResRatio, yResRatio;
	tt_int32 xPixelsPerEm, yPixelsPerEm;
	register short UPEM;
        
	assert( errCode != NULL );
	if ( (*errCode = setjmp(t->mem->env)) == 0 ) {
        /* Note: trans is updated here! */
        /* Note: compensation is applied in T2K_RenderGlyph() and 
                 it is transparent to most of the code. 
                 There are 3 exceptions:
                   - fontwide metrics 
                   - glyph metrics 
                   - oversampling images for subpixel text. */
        ExtractBitmapTransform(trans, &t->compensation);
		
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

		/* Now remove the point size from the matrix */
#ifdef ENABLE_TT_HINTING
		/* Save resolution ( code below to set transformation MTE) */
		t->ttd.xRes=xRes; t->ttd.yRes=yRes;  
 		t->ttd.pointSize=xPointSize>yPointSize?xPointSize:yPointSize;
#endif

		if ( xPixelsPerEm > 0 && yPixelsPerEm > 0 ) {
#ifdef ENABLE_TT_HINTING
           /* for hinting we only need stretch matrix.
              Moreover scale factor will be extracted from t->ttd.pointSize, 
              we only need to preserve proportions here.
              Note: if trans is unrotated than this is the same as normalizing trans.
                However, if text is rotated this approach keep proportions of unrotated text. */
           t->ttd.trans.t00 = util_FixDiv(xPointSize, t->ttd.pointSize);
           t->ttd.trans.t10 = t->ttd.trans.t01 = 0;
           t->ttd.trans.t11 = util_FixDiv(yPointSize, t->ttd.pointSize);
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
                    /* We need to scale if !t->is_Identity or compensation is not identity. 
                       For simplicity just scale for now. */
                    F16Dot16 tmpX = t->caretDx;
                    F16Dot16 tmpY = t->caretDy;
                    t->caretDx = CALCULATE_TRANSFORMED_X(tmpX, tmpY, t); 
                    tmpY       = CALCULATE_TRANSFORMED_Y(tmpX, tmpY, t);
                    /* have to apply compensation transform 
                       note that (t->caretDx, tmpY) - are current values */
                    t->caretDy = CALCULATE_TRANSFORMED_Y(t->caretDx, tmpY, &t->compensation);
                    t->caretDx = CALCULATE_TRANSFORMED_X(t->caretDx, tmpY, &t->compensation);

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
                    /* We need to scale if !t->is_Identity or compensation is not identity. 
                       For simplicity just scale for now. */
                    F16Dot16 tmpX = t->vert_caretDx;
                    F16Dot16 tmpY = t->vert_caretDy;
                    t->vert_caretDx = CALCULATE_TRANSFORMED_X(tmpX, tmpY, t); 
                    tmpY            = CALCULATE_TRANSFORMED_Y(tmpX, tmpY, t);
                    /* have to apply compensation transform 
                       note (t->caretDx, tmpY) - are current values */
                    t->vert_caretDy = CALCULATE_TRANSFORMED_Y(t->vert_caretDx, tmpY, &t->compensation);
                    t->vert_caretDx = CALCULATE_TRANSFORMED_X(t->vert_caretDx, tmpY, &t->compensation);

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
	tt_uint8 greyScaleLevel,
	tt_uint32 cmd, 
	tt_int32 hintingComposite,	/* current level of recursion */
	F16Dot16 stretchBy)       	/* the amount by which we need to enlarge width of bitmap
	                          	   (used for bitmap bolding of AA text - see 
	                          	    bold_greyscale_bitmap()) */	
{
	F26Dot6 *xPtr, *yPtr, *backupOutline = NULL;
	int pointCount;
	tt_int32 i, n;
	register GlyphClass *glyph;
	int isFigure = false;
	tt_uint16 advanceWidth, 			/* Advanced width, including style */
			advanceWidthPure, styleDeltaAdjust;		/* Advanced width, without style. 	*/
	tt_int32 errorCode;
	int isOptimizeOutline=0;
	int isLCDMode = 0;
	int lcdScale = 3; /* if isLCDMode non-zero */
	register short UPEM;
	T2K_TRANS_MATRIX  save;
	VectorSet saveVectorSet;
	tt_int32 save_XPPEM, save_YPPEM;
	int gotComponentMatrix=0;
	int isCompound;
	tt_int32 rawArg1, rawArg2;
 
	/* MTE: the isCompound value is a flag to indicate, at this level
			of recursion, whether or not the glyph is compound. If compound, then
			no scaling is applied at the end, because the scaling is done for each
			glyph component. But why can't we just look at the glyph contour count?
			because the glyph changes when a recursive call is made.
	*/

	isLCDMode = (((cmd & T2K_LCD_MODE) || (cmd & T2K_LCD_MODE_V)) &&
		     ((cmd & T2K_RETURN_OUTLINES) == 0));
	if (isLCDMode) {
	    lcdScale = (cmd >> 24);
	}

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
	
        if (hintingComposite > 10) {
        	/* to protect from malformed fonts with circular dependencies in composites */
        	glyph = New_EmptyGlyph(t->mem, 0, 0);
	} else if ( cmd & T2K_CODE_IS_GINDEX ) {
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
 
	/* Note that advanceWidth might have been adjusted for style changes 
	   (see setHmtx() in truetype.c)
	   If so, t->font->hmtxLinearAdjustment stores the delta */
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
                if (CHANGES_ORIENTATION(&newbie)) {
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
			T2K_RenderGlyphInternal( t, gIndex, xFracPenDelta, yFracPenDelta, 0, comp_cmd, hintingComposite+1, stretchBy);

				
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
           There are 4 conditions:
             1. T2K_GRID_FIT flag must be set in the cmd or 
                it is composite glyph without scale info
                (see comment below on this special case). 
             2. T2K_USE_FRAC_PEN flag must NOT be set in the cmd. 
                Why? Because Frac Pen causes funny adjustment.
                FracPen is a higher level function which should not be on for 
                the actual hinting. The LinearAdvance values can be used 
                to determine how far to advance the drawing pen, or use 
                the hmtx table for linear advance.
             3. It should not be Type1/Type2 font.
             4. There must be some outline points.

           Clarification about "hinting & scaling":
           
           Some composite glyphs have their "scale" info totally depend
           on hinting instructions (e.g. \u5de5 of MingLiU). 
           For these glyphs we have to apply hinting to outlines to scale
           component glyph correctly.
           Note that we only do this iff none of components had scale info.
       */

            tt_int32 haveScaleInfo = oredFlags & WE_HAVE_A_SCALE |
                                     oredFlags & WE_HAVE_AN_X_AND_Y_SCALE |
                                     oredFlags & WE_HAVE_A_TWO_BY_TWO;

            isOptimizeOutline = 
                    (!haveScaleInfo || (cmd & T2K_GRID_FIT)) && 
                    (!(cmd & T2K_USE_FRAC_PEN)) &&
                    (t->font->T1 == NULL && t->font->T2 == NULL) &&
                    (pointCount > 1) ;

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
            if (isOptimizeOutline) {
                errorCode = TTScalerHintGlyph((GlyphClass *) glyph, (T2K *) t);
            }
            /* Always apply styling after all else. */
            ApplyPostStyle(glyph, t);
#endif
	/* end ENABLE_TT_HINTING */
	
	    		 glyph->hintFragment= hintFragmentSave;
 				 glyph->hintLength=hintLengthSave;

 			}

	} else {     	/* ************** ------------ END COMPOUND ---------- START SIMPLE GLYPH *********************** */
 				 	/* ************** ------------ END COMPOUND ---------- START SIMPLE GLYPH *********************** */
 				 	
        /* This code is used to process a simple glyph. It may occur in two ways. 
           First, it may truly be a request to render a simple glyph. 
           But it might also be part of a compound glyph. 
           
           The routine parameter, hintingComposite, is non-zero when hinting 
           a simple glyph within a composite. It is zero for a simple glyph. */
 		
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
        isOptimizeOutline = (cmd & T2K_GRID_FIT) && /* If not grid-fitting, then nothing more. 	*/
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
            if (isOptimizeOutline) {
                errorCode = TTScalerHintGlyph((GlyphClass *) glyph, (T2K *) t);
            }
            /* For sub-components of a compound glyph, we don't want to add style */
            if (hintingComposite == 0) {
                ApplyPostStyle(glyph, t);
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
	/* note that advanceWidth has been already adjusted for style changes
	   (see setHmtx in truetype.c) */
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
    if (isLCDMode) {
        int i;
        if (((cmd & T2K_LCD_MODE) && !WILL_FLIP_AXES(t)) ||
            ((cmd & T2K_LCD_MODE_V) && WILL_FLIP_AXES(t))) {
            for (i=0;i<(glyph->pointCount +2);i++) {
                glyph->x[i] *= lcdScale;
            }
        } else {   /* ((cmd & T2K_LCD_MODE_V && !WILL_FLIP_AXES(t)) ||
                      ((cmd & T2K_LCD_MODE) && !WILL_FLIP_AXES(t))) */ 
            for (i=0;i<(glyph->pointCount +2);i++) {
                glyph->y[i] *= lcdScale;
            }
        }
    }

#ifdef ENABLE_T2KE
	t->baseARGB = NULL;
#endif /* ENABLE_T2KE */
	
	t->baseAddr = NULL;	t->rowBytes = 0;
	t->width = 0; 		t->height = 0;
	t->fTop26Dot6 = 0;	t->fLeft26Dot6 = 0;

    /* We may need to alter outline by widening it.
       This happens if we bold AA text using new bitmap bolding approach
       (see bold_greyscale_bitmap for motivation and details) */
    if (stretchBy > 0 && (xPtr[glyph->pointCount+1] - xPtr[glyph->pointCount]) > 0) {
        tt_int32 i;
        F16Dot16 k;

        /* We are altering outline only to generate wider bitmap for further bolding.
           However, outline of B&W and AA text must stay the same.
           To preserve consistence we need to save original outline and restore it 
           after bitmap generation. It will be further processed in the ordinary way.
             
           NB: we are changing only X coordinates. So, we need to backup only first n numbers. */
             
           backupOutline = (F26Dot6*) tsi_AllocMem(t->mem, n*sizeof(F26Dot6));
           memcpy(backupOutline, xPtr, n*sizeof(F26Dot6));

         /* Widening is performed by horizontally scaling outline until distance 
            between phantom points is increased by stretchBy */

         /* NB: we probably could use FontMath.h primitives to work with F26Dot6 values here. */
         k = util_FixDiv(((xPtr[glyph->pointCount+1] - xPtr[glyph->pointCount]) << 10) + stretchBy, 
                           (xPtr[glyph->pointCount+1] - xPtr[glyph->pointCount]) << 10);
        
         for (i=0;i<(glyph->pointCount+2);i++) {
                xPtr[i] = xPtr[glyph->pointCount] 
                          + (util_FixMul((xPtr[i] - xPtr[glyph->pointCount]) << 10, k) >> 10);
         }
    }

	if ( cmd & T2K_SCAN_CONVERT && glyph->pointCount > 1 ) {
	tsiScanConv *sc = NULL; 
#ifdef ENABLE_T2KE
		tsiColorScanConv *scc = NULL;
#endif

		char xWeightIsOne = (char)(t->xWeightIsOne && (cmd & T2K_GRID_FIT) && t->ag_xPixelsPerEm <= 24);
	 
#ifdef ENABLE_T2KE
		if ( glyph->colorPlaneCount > 0 ) {
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
		} 
		else  
#endif
		{
	 			short dropoutMode = getDropoutMode(t, greyScaleLevel); 
#ifdef UIDebug
				if (isForceDropoutFlag )
						dropoutMode = DOCONTROL_FULL; /* force it on, even if not needed. */
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
                if (greyScaleLevel > 0) {
                    MakeGreyscaleBits(sc, greyScaleLevel, xWeightIsOne, 
                             (char)(cmd & T2K_SKIP_SCAN_BM), true, dropoutMode);
                } else {
                    MakeBWBits(sc, (char)(cmd & T2K_SKIP_SCAN_BM), true, dropoutMode);
                }
                t->width  = sc->right  - sc->left;
                t->height = sc->bottom - sc->top;
                t->fTop26Dot6  = sc->fTop26Dot6;
                t->fLeft26Dot6 = sc->fLeft26Dot6;
                t->rowBytes = sc->rowBytes;
      }


		
#ifdef ENABLE_T2KE
		if ( glyph->colorPlaneCount > 0 ) {
			t->baseARGB = scc->baseARGB; scc->baseARGB = NULL; /* We take over and deallocate t->baseARGB */
			t->baseAddr = NULL;
		} else 
#endif
		{
				 t->baseAddr  = sc->baseAddr;
				 sc->baseAddr = NULL; /* We take over and deallocate t->baseAddr */
#ifdef ENABLE_T2KE
				t->baseARGB = NULL;
#endif
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

        if (backupOutline != NULL) {
          memcpy(xPtr, backupOutline, n*sizeof(F26Dot6));
          tsi_DeAllocMem(t->mem, backupOutline);
        }

	if ( cmd & T2K_RETURN_OUTLINES ) {
		;
	} else {
		T2K_PurgeMemoryInternal( t, 0 );
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

#if 0
/* this is debug function that output bitmap to stdout */
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
#endif

/* number of bytes needed to store row of x b&w points */
#define BYTES_FOR_BWPIXELS(x) (((x) + 7) >> 3)

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

/* Do we need to widen plain version of glyph?
   We want to stay compatible with earlier releases of jdk. So we do not widen too small ones. */
#define IS_WIDENING(t) ((t)->yPixelsPerEm >= HEIGHT_THRESHOLD_NO_WIDENING)

static void bold_bitmap(T2K *t) {
  int xi, yi, xii;
  tt_uint8 columnScore[MAX_WIDTH];      /* loss factors for each of bitmap columns 
                                           characterising how many pixels in this column 
                                           could not be bolded without widening */
  tt_int32 newRowBytes;
  unsigned char *newBaseAddr; /* placeholder for new bitmap */
  char cand = -1;       /* candidate column to be replicated */ 
  unsigned char offset; /* compensation factor used to map columns from original to 
						   new bitmap (count of inserted column replicas) */
  int widen_by_pixels = 0; /* at the moment we support only 1 and 0 */
  int deltaY = 0, deltaX = 0;

  /* do not bold big bitmaps */
  if (t->width >= MAX_WIDTH) 
	return;

  if (IS_WIDENING(t))
     widen_by_pixels = 1; 
   
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
		   because addition of black pixel introduce bridge between two other pixels
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
    newRowBytes = BYTES_FOR_BWPIXELS(t->width + widen_by_pixels); 
    newBaseAddr = (unsigned char*) tsi_AllocArray(t->mem, newRowBytes, t->height);
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
         We bold black pixels by setting pixels on previous position to black color.
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



static void ApplyCompensationTransformToOutlines(T2K* t) {
    int i;
    F16Dot16 tmpX, tmpY, *xPtr, *yPtr;
    xPtr = t->glyph->x;
    yPtr = t->glyph->y;
    for (i = 0; i < t->glyph->pointCount+2; i++) {
        tmpX = xPtr[i]; 
        tmpY = yPtr[i];
        xPtr[i] = CALCULATE_TRANSFORMED_X(tmpX, tmpY, &t->compensation); 
        yPtr[i] = CALCULATE_TRANSFORMED_Y(tmpX, tmpY, &t->compensation);
    }
}

/* Update advances and position of leftTop coordinate.
   
   Implementation logic is in line with ApplyCompensationTransformToBitmap
   but it is decoupled because we need to update metrics even if we do not have outline. */
static void ApplyCompensationTransformToMetrics(T2K* t) {
    F16Dot16 tmp, tmp_x1, tmp_x2, tmp_y1, tmp_y2, dx, dy;

    /* this is essentially position of rightTop point after transform
       if it happens to become negative we'll shift coordinates to ensure it will become zero */
    dx = CALCULATE_TRANSFORMED_X((t->width<<16), (t->height<<16), &t->compensation);
    if (dx > 0) {
        dx = 0;
    }

    dy = CALCULATE_TRANSFORMED_Y((t->width<<16), (t->height<<16), &t->compensation);
    if (dy > 0) {
        dy = 0;
    }


    tmp = CALCULATE_TRANSFORMED_X(t->xAdvanceWidth16Dot16, 
                                  t->yAdvanceWidth16Dot16, 
                                  &t->compensation);
    t->yAdvanceWidth16Dot16 = CALCULATE_TRANSFORMED_Y(t->xAdvanceWidth16Dot16, 
                                  t->yAdvanceWidth16Dot16, 
                                  &t->compensation);
    t->xAdvanceWidth16Dot16 = tmp;

    tmp = CALCULATE_TRANSFORMED_X(t->xLinearAdvanceWidth16Dot16, 
                                  t->yLinearAdvanceWidth16Dot16, 
                                  &t->compensation);
    t->yLinearAdvanceWidth16Dot16 = CALCULATE_TRANSFORMED_Y(t->xLinearAdvanceWidth16Dot16, 
                                  t->yLinearAdvanceWidth16Dot16, 
                                  &t->compensation);
    t->xLinearAdvanceWidth16Dot16 = tmp;

    /* we calculate coordinates of transformed leftTop and rightBottom points
       then we select those that really become leftTop for transformed glyph */
    tmp_x1 = CALCULATE_TRANSFORMED_X((t->fLeft26Dot6<<10), 
                                     (t->fTop26Dot6<<10), 
                                     &t->compensation);
    tmp_y1 = CALCULATE_TRANSFORMED_Y((t->fLeft26Dot6<<10), 
                                     (t->fTop26Dot6<<10), 
                                     &t->compensation);
        
    tmp_x2 = CALCULATE_TRANSFORMED_X((t->fLeft26Dot6<<10) + (t->width<<16), 
                                     (t->fTop26Dot6<<10) - (t->height<<16), 
                                     &t->compensation);
    tmp_y2 = CALCULATE_TRANSFORMED_Y((t->fLeft26Dot6<<10) + (t->width<<16), 
                                     (t->fTop26Dot6<<10) - (t->height<<16), 
                                     &t->compensation);
        
    if (tmp_x1 > tmp_x2) 
        t->fLeft26Dot6 = tmp_x2 >> 10;
    else 
        t->fLeft26Dot6 = tmp_x1 >> 10;

    if (tmp_y1 > tmp_y2) 
        t->fTop26Dot6  = tmp_y1 >> 10;
    else 
        t->fTop26Dot6  = tmp_y2 >> 10;

}

static void ApplyCompensationTransformToBitmap(T2K *t, int isAA) {
    unsigned char *newBaseAddr;
    int x, y, sz, p, x1, y1, newRowBytes;
    F16Dot16 tmp, dx, dy;
    int t00, t01, t10, t11;

    if (t->baseAddr == NULL || (t->compensation.t00 > 0 && t->compensation.t11 > 0)) {
        return;
    }

    /* allocate space for transformed bitmap */
    if (t->compensation.t01 != 0) { /* dimensions were flipped! */
        if (isAA) {
            newRowBytes = t->height;
        } else {
            newRowBytes = BYTES_FOR_BWPIXELS(t->height);
        }
        sz = newRowBytes*t->width;
    } else {
        sz = t->rowBytes*t->height;
        newRowBytes = t->rowBytes;
    }

    t00 = t->compensation.t00 >> 16;
    t01 = -(t->compensation.t01 >> 16);
    t10 = -(t->compensation.t10 >> 16);
    t11 = t->compensation.t11 >> 16;

    dx = ((t->width-1)*t00 + (t->height-1)*t01);
    if (dx > 0) {
        dx = 0;
    }
    dy = ((t->width-1)*t10 + (t->height-1)*t11);
    if (dy > 0) {
        dy = 0;
    }

    newBaseAddr = (unsigned char*) tsi_AllocMem(t->mem, sz);
    memset(newBaseAddr, 0, sz);

    for(x=0; x<t->width; x++) {
        for (y=0; y<t->height; y++) {
            if (isAA) {
                p = t->baseAddr[t->rowBytes*y+x];
            } else {
                p = IS_POINT_SET(x, y, t->rowBytes, t->baseAddr);
            } 
            if (p) {
                x1 = x*t00 + y*t01 - dx; /* to preserve positive coordinates */
                y1 = x*t10 + y*t11 - dy;
                if (isAA) {
                    newBaseAddr[y1*newRowBytes+x1]=p;
                } else {
                    SET_POINT(x1, y1, newRowBytes, newBaseAddr);
                }
            }
        }
    }

    if (WILL_FLIP_AXES(t)) { /* axes were flipped! */
        t->rowBytes = newRowBytes;
        sz = t->width;
        t->width = t->height;
        t->height = sz;
    }

    tsi_DeAllocMem(t->mem, t->baseAddr);
    t->baseAddr = newBaseAddr;
}


#if 0
/* this is debug function that prints greyscale bitmap to stdout */
static void dump_greyscale_bitmap(T2K *t) {
  int x, y;
  unsigned char *baseAddr = t->baseAddr;
  
  if (baseAddr == NULL) return;

  printf("========= DUMP ======\n");
  for ( y = t->height; y > 0; y-- ) {
    for ( x = 0; x < t->rowBytes; x++ ) {
      printf("%2x ", baseAddr[x]);
    }
    printf("\n");
    baseAddr += t->rowBytes;
  }
  printf("============================\n");
}
#endif

/* maximal greyscale value (corresponds to black pixel) */
#define FULL_BLACK  120

/* below this threshold we consider pixels to be too white to be boosted (empirical) */
#define ALMOST_WHITE 5

/* get point value from original bitmap */
#define P(k)  t->baseAddr[k]

/* get point value from new (bold) bitmap */
#define PN(k) baseAddr[k]

/* safe version of P(i, j) - check indices for validity */
#define PVAL(i, j, k) (( (i) >= 0 && (i) < t->rowBytes && (j) >= 0 && (j) < t->height)? P(k) : 0) 

/* boost intensity of given original pixel by given factor. 
   Do not allow result go out of bounds */
#define BOOSTP(k, factor)  ((P(k) + (factor)) >  FULL_BLACK ? FULL_BLACK : (P(k) + (factor)))

/* rewind one line up/down */
#define UP(offset)   (offset - t->rowBytes)
#define DOWN(offset) (offset + t->rowBytes)

/*
  The idea of greyscale bitmap bolding is to add ink to some points 
    and spread extra ink to adjacent points. 
*/
static void bold_greyscale_bitmap(T2K *t) {
    int x, y, offset;
    unsigned char *baseAddr;
    unsigned char extra;
    int add_ink = FULL_BLACK;
    unsigned char p, p_l, p_r, p_u, p_d, p_ul, p_ur, p_dl, p_dr, p2;
    int pnew;
    int next_pixel = 0;
    
    if (t->baseAddr == NULL || HEIGHT_THRESHOLD_FOR_BITMAP_BOLDING < t->yPixelsPerEm) 
        return;

    /*  dump_greyscale_bitmap(t); */

    /* can we do it inplace? 
       This might be too complicated because sometime we need original values. */
    baseAddr = (unsigned char*) tsi_AllocArray(t->mem, t->rowBytes, t->height);
    memset(baseAddr, 0, t->rowBytes*t->height);                                           

    /* process bitmap line by line */
    offset = (t->height-1)*t->rowBytes;
    for (y = t->height-1; y >= 0; y--, offset = UP(UP(offset))) {
        for (x = 0; x < t->rowBytes; x++, offset++) {
            p    = PVAL(x  , y  , offset);
            p_l  = PVAL(x-1, y  , offset-1);
            p_r  = PVAL(x+1, y  , offset+1);
            p_d  = PVAL(x  , y+1, DOWN(offset));
            p_dl = PVAL(x-1, y+1, DOWN(offset-1));
            p_dr = PVAL(x+1, y+1, DOWN(offset+1));
            p_u  = PVAL(x  , y-1, UP(offset));
            p_ul = PVAL(x-1, y-1, UP(offset-1));
            p_ur = PVAL(x+1, y-1, UP(offset+1));
            p2 = p << 1;
            
            if (
               /* skip "too light grey" points.
                  motivation: even sequence of 3 white points has 
                    local minimum but we really do not want to boost it.
                    Extremely light grey points are often just minor side-effect
                      of contour that mostly fits into adjacent pixel. 
                      We do not want to exaggerate such cases. */
                 p > ALMOST_WHITE
               /* current point is local maximum along the horizontal axis 
                  or adjacent points have same value */
               && p >= p_l && (p> p_r || (p == p_r && x > 0))
                 /* current point is either darker than at least one of its vertical neighbours 
                    or at least it is not much lighter 
                    (Motivation: we want to bold vertical stems smoothly 
                                 but we do not want to merge something)) */
               && (p_u || p >= p_d || p >= (p_d + p_u)/3)
                 /* none of adjacent points should be much darker than current one */      
               && !(p2 < p_ul || p2 < p_dl || p2 < p_ur 
                    || p2 < p_dr || p2 < p_u || p2 < p_d)) {

                /* Ok, current point will be boosted. 
                
                   The idea of boosting is to add energy of single FULL_BLACK
                   pixel to the local maximum. This extra energy is spread proportionally 
                   between pixel in question and two adjacent pixels.
                   (Why spread? Because we are trying to keep smoothness of countours)

                   We may also want to propagate overflow energy from local maxima to 
                   one of adjacent pixels. 
                   Motivation: 1 pixel vertical line consisting of almost black pixels 
                      and surrounded by white pixels will not change without such propagation.
                   NB: hinting introduce black stems quite often. 
                   Caveat: Propagation to both pixels or choosing pixels to propagate 
                         dynamically results in blurry image where stems may look corrupted 
                */

                /* what will be new value of p? */
                pnew = (p*add_ink/(p +p_l+p_r)) + p;
                if (pnew > FULL_BLACK) {
                    PN(offset) = FULL_BLACK;
                    extra = pnew - FULL_BLACK;
                } else {
                    PN(offset) = (char) pnew;
                    extra = 0;
                }

                /* do not darken adjacent pixel if it is local minimum 
                   (to prevent merging of different stems) */

                if (x > 0 && PVAL(x-2, y, offset-2) <= p_l && PN(offset-1) == 0) {
                    PN(offset-1) = BOOSTP(offset-1, (char) (p_l*add_ink/(p+p_l+p_r)));
                }

                /* spread extra ink to the right to be more consistent with B&W */
                if (x+1 < t->rowBytes) {
                    if (PVAL(x+2, y, offset+2) <= p_r) {
                        /* spread half of energy to improve smoothing */
                        PN(offset+1) = BOOSTP(offset+1, (char) (p_r*add_ink/(p+p_l+p_r)) + (extra >> 1));
                    } else {
                        /* even if both adjacent points are darker we may want to 
                           boost this point a bit to improve smoothness of result
                          (this helps in cases like M or N  - otherwise vertical stems are 
                           too obviously non-uniformly bolded near connections with slanted stems) */
                        PN(offset+1) = BOOSTP(offset+1, (extra >> 2));
                    }
                    /* skip next pixel to avoid overwriting of boosted value */
                    offset++; x++;
                } 
            } else {
                /* just copy old value */
	        PN(offset) = P(offset);
            }
        }
    }

    /*  dump_greyscale_bitmap(t);  */
  
    tsi_DeAllocMem(t->mem, t->baseAddr);
    t->baseAddr = baseAddr;
}

/* 
   If we have widened bitmap during bitmap bolding
   we need to update advances to prevent glyph overlapping.
*/
static void adjust_advances_after_bitmap_bolding(T2K *t, F16Dot16 stretchBy) {
    F16Dot16 deltaY = 0, deltaX = 0;
 
    /* 
     We must keep in mind that we want to preserve direction of baseline
     while widening is always performed horizontally.

     To workaround this we use following approach:
       1) if our xAdvance is bigger than yAdvance 
          (i.e. for angle between -PI/4 and PI/4 degree and between 3*PI/4 to 5*PI/4) 
          we stretch xAdvance by stretchBy pixels and update yAdvance to preserve angle of baseline 
          (using rule xAdvance/yAdvance = deltaX/deltaY)
       2) otherwise we stretch yAdvance by stretchBy pixels and update xAdvance to keep direction 
          of baseline unchanged
          
     Note: adjustment is NOT necessary for linear advances (used when fractional metrics are on)
           In this case adjustments are done in special "styled" version 
           of hmtx table that is used to calculate linear advances.
  */    
    if (stretchBy > 0) {
        if (T2K_ABS(t->xAdvanceWidth16Dot16) > T2K_ABS(t->yAdvanceWidth16Dot16)) {
	    deltaY = util_FixMul(util_FixDiv(t->yAdvanceWidth16Dot16, t->xAdvanceWidth16Dot16), stretchBy);
	    deltaX = stretchBy;
	    if (t->xAdvanceWidth16Dot16 < 0) { /* pay attention to direction of deltaX */
	        deltaY = -deltaY;
	        deltaX = -deltaX;
	    }        
        } else if (t->yAdvanceWidth16Dot16 != 0) {
	    deltaX = util_FixMul(util_FixDiv(t->xAdvanceWidth16Dot16, t->yAdvanceWidth16Dot16), stretchBy);
	    deltaY = stretchBy;
	    if (t->yAdvanceWidth16Dot16 < 0) { /* pay attention to direction of deltaY */
	        deltaY = -deltaY;
	        deltaX = -deltaX;
	    }  
        }
        t->xAdvanceWidth16Dot16 += deltaX;
        t->yAdvanceWidth16Dot16 += deltaY;      
    }
}



/* is bolding required? */
#define IS_BOLDING(t)  ((t)->font->StyleFuncPost != NULL &&  (t)->font->params[0] != ONE16Dot16)

/* can we do bitmap bolding (only applicable to certain sizes) */
#define IS_BITMAP_BOLDING_APPLICABLE(t) ((t)->yPixelsPerEm < HEIGHT_THRESHOLD_FOR_BITMAP_BOLDING)

/* is it antialiasing or black&white rendering mode? */ 
#define IS_AA(greyScaleLevel)  (greyScaleLevel != BLACK_AND_WHITE_BITMAP)

/*
 *
 */
void T2K_RenderGlyph( T2K *t, tt_int32 code, tt_int8 xFracPenDelta, tt_int8 yFracPenDelta, tt_uint8 greyScaleLevel, tt_uint32 cmd, int *errCode )
{
	int doBitmapBolding = 0;
	F16Dot16 savedStyleFactor;
	F16Dot16 stretchBy = 0;
	assert( errCode != NULL );
	if ( (*errCode = setjmp(t->mem->env)) == 0 ) {
		/* try */
		tsi_Assert( t->mem, t->mem->state == T2K_STATE_ALIVE, T2K_ERR_USE_PAST_DEATH );
		assert( !( (cmd & T2K_GRID_FIT) && (cmd & T2K_TV_MODE)) ); /* If you turn on T2K_TV_MODE, then please turn off T2K_GRID_FIT */

		if (IS_BOLDING(t) && IS_BITMAP_BOLDING_APPLICABLE(t)) {
		    /* Bolding of small glyphs is performed on bitmap level for 2 reasons:
		          - we can still benefit from hints (if they are available)
		          - working with pixels we can better prevent introduction of black smudges

		       To do so, we request bitmap for non-bold version of glyph from rasterizer 
		          and bold it. 
		       Note that for certain sizes bold version of glyph must be wider than plain.
		       (at the moment we can widen only by 0 or 1 pixels)
		       We use two different ways to achieve that:
		          - for B&W we widen bitmap
		          - for AA text we temporary widen outline before generating plain bitmap
		    */
		    doBitmapBolding = 1;
		    savedStyleFactor  = t->font->params[0];
		    t->font->params[0] = ONE16Dot16;
		    if (IS_WIDENING(t) && IS_AA(greyScaleLevel)) {
		        stretchBy = ONE16Dot16;
		    }
		}

#ifdef ENABLE_SBIT
		if ((cmd & T2K_RETURN_OUTLINES) == 0 && t->enableSbits && T2K_GetSbits( t, code, greyScaleLevel, cmd )  ) {
			t->embeddedBitmapWasUsed 		= true;
		} else
#endif /* ENABLE_SBIT */
		{
		  /* In LCD mode we can't do bitmap bolding on the 3X scaled
		   * bitmap - at least not yet. But the extra resolution seems
		   * to help the image quality anyway.
		   * Also embedded bitmaps currently bypasses/deselects LCD 
		   * text, and we won't reach here, so those cases are 
		   * unaffected by LCD text.
		   */
		        if (doBitmapBolding &&
			    ((cmd & T2K_LCD_MODE) || (cmd & T2K_LCD_MODE_V))) {
			    t->font->params[0] = savedStyleFactor;
			    doBitmapBolding = 0;
			}
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

			/* set glyph dropout control mode */
			t->glyphDropOutControlFlags = DEFAULT_DROPOUT_MODE;

			T2K_RenderGlyphInternal( t, code, xFracPenDelta, yFracPenDelta, greyScaleLevel, cmd, 0, stretchBy);
			/*  ORIENTBOLD_STYLES */			
		if (t->theContourData.active)
				VerifyContourUsage(&t->theContourData);
 			/*  ORIENTBOLD_STYLES */			
			t->horizontalMetricsAreValid	= true;
			t->verticalMetricsAreValid		= false;
		}

		if (doBitmapBolding) {
		    t->font->params[0] = savedStyleFactor;
		    if (IS_AA(greyScaleLevel)) {
		        bold_greyscale_bitmap(t);
		    } else {
		        bold_bitmap(t);
		   }
		   /* outline is still plain. we may need to bold it */
		   if (cmd & T2K_RETURN_OUTLINES) {
		       /* be sure to not apply italic twice! */
		       savedStyleFactor  = t->font->params[1];
		       t->font->params[1] = 0x0;
               ApplyPostStyle(t->glyph, t);
		       t->font->params[1] = savedStyleFactor;
		   }
		   if (IS_WIDENING(t)) {
		       adjust_advances_after_bitmap_bolding(t, ONE16Dot16);
		   }
		}

        /* apply compensation tranform if it is not identity
           (we know that this matrix has only 2 non zero components) */
        if (t->compensation.t00 != ONE16Dot16 || t->compensation.t11 != ONE16Dot16) {
            ApplyCompensationTransformToMetrics(t);
            ApplyCompensationTransformToBitmap(t, IS_AA(greyScaleLevel));
            if (cmd & T2K_RETURN_OUTLINES) {
                /* Apply compensation transform to outline */
                MatrixTimesVector(&t->compensation, t->glyph->x,  t->glyph->y, t->glyph->pointCount+2);
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
