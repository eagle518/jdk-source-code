/* 
 *  @OSF_COPYRIGHT@
 *  COPYRIGHT NOTICE
 *  Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 *  ALL RIGHTS RESERVED (MOTIF). See the file named COPYRIGHT.MOTIF for
 *  the full copyright text.
*/ 
/* 
 * HISTORY
*/ 
/* $XConsortium: VaSimpleP.h /main/10 1996/10/16 16:57:41 drk $ */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmVaSimpleP_h
#define _XmVaSimpleP_h

#include <Xm/XmP.h>

# include <stdarg.h>
# define Va_start(a,b) va_start(a,b)


#ifdef __cplusplus
extern "C" {
#endif

#define StringToName(string) XrmStringToName(string)

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmVaSimpleP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
