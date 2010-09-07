/*
 * @(#)FixMulDiv.h	1.8 03/12/19 
 */

/*
 * Portions Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 
  	<1>		6/21/99		MTE		Created header file.
 
 
	Copyright:	© 1990 by Apple Computer, Inc., all rights reserved.

 */

#ifndef FixMulDivIncludes
#define FixMulDivIncludes
// #include "FontMath.h" 
tt_int32 MultiplyDivide(tt_int32 a, tt_int32 b, tt_int32 c);  
tt_int32 MultiplyFract(tt_int32 a, tt_int32 b);
 
#ifdef __cplusplus
}
#endif

#endif
