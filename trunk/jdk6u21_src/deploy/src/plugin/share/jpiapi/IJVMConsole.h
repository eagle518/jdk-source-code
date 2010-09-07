/*
 * @(#)IJVMConsole.h	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=---------------------------------------------------------------------------=
//
// IJVMConsole.h  by X.Lu 
//
//=---------------------------------------------------------------------------=
//
// Contains interface for JVM Console action
//
#ifndef _IJVMCONSOLE_H
#define _IJVMCONSOLE_H

#include "ISupports.h"
//{CEA3596A-9DB0-11d6-9A7D-00B0D0A18D51}
#define IJVMCONSOLE_IID \
{0xCEA3596A, 0x9DB0, 0x11d6, {0x9A, 0x7D, 0x00, 0xB0, 0xD0, 0xA1, 0x8D, 0x51}}

class IJVMConsole : public ISupports
{
public:
    JD_DEFINE_STATIC_IID_ACCESSOR(IJVMCONSOLE_IID);
    
    /**
     * Show Java Console
     */
    JD_IMETHOD
    Show(void) = 0;

    /**
     * Hide Java Console
     */
    JD_IMETHOD
    Hide(void) = 0;

    /**
     * Test whether Java console is visible or not
     */
    JD_IMETHOD
    IsVisible(JDBool *result) = 0;
    
    /**
     * Prints a message to the Java console. 
     * @parms encodingName specifies the encoding of the message, and if NULL, 
     * specifies the default platform encoding.
     */
    JD_IMETHOD
    Print(const char* msg, const char* encodingName = NULL) = 0;
};

#endif //_IJVMCONSOLE_H
