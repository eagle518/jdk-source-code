/*
 * @(#)img_scaleloop.h	1.18 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * This file contains the skeleton code for generating functions to
 * convert image data for the Java AWT.  Nearly everything below is
 * a call to a macro that is defined in one of the header files
 * included in this directory.  A description of the various macro
 * packages available for customizing this skeleton and how to use
 * this file to construct specific conversion functions is available
 * in the README file that should also be included in this directory.
 */

ImgConvertFcn NAME;

int NAME(struct Hjava_awt_image_ColorModel *colormodel,
	 int srcOX, int srcOY, int srcW, int srcH,
	 void *srcpix, int srcOff, int srcBPP, int srcScan,
	 int srcTotalWidth, int srcTotalHeight,
	 int dstTotalWidth, int dstTotalHeight,
	 ImgConvertData *cvdata, ImgColorData *clrdata)
{
    DeclareScaleVars
    DeclareInputVars
    DeclareDecodeVars
    DeclareAlphaVars
    DeclareDitherVars
    DeclareOutputVars
    unsigned int pixel;
    int red, green, blue;
    IfAlpha(int alpha;)

    InitInput(srcBPP);
    InitScale(srcpix, srcOff, srcScan,
	      srcOX, srcOY, srcW, srcH,
	      srcTotalWidth, srcTotalHeight,
	      dstTotalWidth, dstTotalHeight);
    InitOutput(cvdata, clrdata, DSTX1, DSTY1);
    InitAlpha(cvdata, DSTY1, DSTX1, DSTX2);

    InitPixelDecode(colormodel);
    InitDither(cvdata, clrdata, dstTotalWidth);

    RowLoop(srcOY) {
	RowSetup(srcTotalHeight, dstTotalHeight,
		 srcTotalWidth, dstTotalWidth,
		 srcOY, srcpix, srcOff, srcScan);
	StartDitherLine(cvdata, DSTX1, DSTY);
	StartAlphaRow(cvdata, DSTX1, DSTY);
	ColLoop(srcOX) {
	    ColSetup(srcTotalWidth, dstTotalWidth, pixel);
	    PixelDecode(colormodel, pixel, red, green, blue, alpha);
	    ApplyAlpha(cvdata, DSTX, DSTY, alpha);
	    DitherPixel(DSTX, DSTY, pixel, red, green, blue);
	    PutPixelInc(pixel, red, green, blue);
	}
	EndMaskLine();
	EndOutputRow(cvdata, DSTY, DSTX1, DSTX2);
	RowEnd(srcTotalHeight, dstTotalHeight, srcW, srcScan);
    }
    DitherBufComplete(cvdata, DSTX1);
    BufComplete(cvdata, DSTX1, DSTY1, DSTX2, DSTY2);
    return SCALESUCCESS;
}
