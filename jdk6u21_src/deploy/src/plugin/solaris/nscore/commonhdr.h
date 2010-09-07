/*
 * @(#)commonhdr.h	1.4 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// commonhdr.h  by X.Lu
//
///=--------------------------------------------------------------------------=

#ifndef _COMMONHDR_H_
#define _COMMONHDR_H

// Disable jni.h headers in Mozilla -- this causes conflict with the jni.h 
// in the JDK/JRE
#ifndef JNI_H
#define JNI_H
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <libintl.h>
#include "Debug.h"
#include "utils.h"
#include "plugin_defs.h"
#include "util5.h"
// This header includes the utility of Smart Pointer
#include "JDSmartPtr.h"
#include "JDSupportUtils.h"

#endif
