#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c2_init_amd64.cpp	1.2 03/12/23 16:35:43 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_c2_init_amd64.cpp.incl"

// processor dependent initialization for amd64

void Compile::pd_compiler2_init() 
{  
  guarantee(CodeEntryAlignment >= InteriorEntryAlignment, "" );
}
