/*
 * @(#)Configure.h	1.17 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


// USE THIS FILE TO SET UP PARAMS FOR DEBUG ETC

	    
// USE ERROR PRINTS (from Jnc.h)
#undef USE_ERROR 
//#ifndef USE_ERROR
//#define USE_ERROR
//#endif


// USE TRACE PRINTS (from Jnc.h)
#undef USE_TRACE 
//#ifndef USE_TRACE
//#define USE_TRACE
//#endif

// USE VERBOSE TRACE PRINTS
#undef USE_VERBOSE_TRACE 
//#ifndef USE_VERBOSE_TRACE
//#define USE_VERBOSE_TRACE
//#endif

#define  IMPLEMENTATION_PACKAGE_NAME "com/sun/media/sound"
#define  JAVA_PACKAGE_BASE            "javax/sound"
#define	 JAVA_SAMPLED_PACKAGE_NAME    JAVA_PACKAGE_BASE"/sampled"
#define	 JAVA_MIDI_PACKAGE_NAME       JAVA_PACKAGE_BASE"/midi"

