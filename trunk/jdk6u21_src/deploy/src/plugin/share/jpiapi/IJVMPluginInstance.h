/*
 * @(#)IJVMPluginInstance.h	1.1 02/11/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=---------------------------------------------------------------------------=
//
// IJVMPluginInstance.h  by X.Lu 
//
//=---------------------------------------------------------------------------=
//
// Contains interface for browser to get additional functionality using the plugin 
// instance
//
#ifndef _IJVMPLUGININSTANCE_H_
#define _IJVMPLUGININSTANCE_H_

#include "ISupports.h"
#include "jni.h"

#define IJVMPLUGININSTANCE_IID	\
{ /*{7A168FD8-A576-11d6-9A82-00B0D0A18D51} */			\
    0x7A168FD8,							\
    0xA576,							\
    0x11d6,							\
    {0x9A,0x82, 0x00, 0xB0, 0xD0, 0xA1, 0x8D, 0x51}		\
}

class IJVMPluginInstance : public ISupports
{
public:
    JD_DEFINE_STATIC_IID_ACCESSOR(IJVMPLUGININSTANCE_IID)
    
    /**
     * For browser to get the java/javabean object in order to 
     * do reflection for JS calls Java
     * @params result [out] the returned java object
     */
    JD_IMETHOD GetJavaObject(jobject *result) = 0;
};

#endif //_IJVMPLUGININSTANCE_H_
