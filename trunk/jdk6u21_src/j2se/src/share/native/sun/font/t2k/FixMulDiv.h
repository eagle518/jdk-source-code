/*
 * @(#)FixMulDiv.h	1.10 10/04/02 
 */

/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
