/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(__IPluginVM__)
#define __IPluginVM__

typedef int JavaID;

#include "IEgo.h"
#include "IPluginVMCB.h"

class IPluginVM : public IEgo {
public:
    enum JavaType { aPPLET, bEAN };
    virtual void newx(JavaType, int argc, const char**, const char**, IPluginVMCB *, JavaID *)=0;
    virtual void start(JavaID)=0;
    virtual void stop(JavaID)=0;
    virtual void destroy(JavaID)=0;
    virtual void window(JavaID, int, int, int, int, int)=0; 
    virtual void print(JavaID)=0;
    virtual void docbase(JavaID, const char *)=0;
    virtual void proxyMapping(char *,char *)=0;
    virtual void cookie(JavaID, char *)=0;
    virtual void javaScriptReply(JavaID, const char *)=0;
    virtual void javaScriptEnd(JavaID)=0;
    virtual void attachThread()=0;
    virtual void getInstanceJavaObject()=0;
    virtual void consoleShow()=0;
    virtual void consoleHide()=0;

    virtual void invalidateCB(JavaID)=0;
};

#endif
