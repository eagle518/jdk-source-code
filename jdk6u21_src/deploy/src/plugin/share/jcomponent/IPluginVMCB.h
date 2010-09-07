/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(__IPluginVMCB__)
#define __IPluginVMCB__

class IPluginVMCB {
public:
    virtual void showStatus(char *)=0;
    virtual void showDocument(char *, char *)=0;
    virtual int findProxy(char *, char **)=0;
    virtual void findCookie(char *, char **)=0;
    virtual int javascriptRequest(char *)=0;
    virtual void setCookie(char *, char *)=0;

    virtual ~IPluginVMCB() { }
};

#endif
