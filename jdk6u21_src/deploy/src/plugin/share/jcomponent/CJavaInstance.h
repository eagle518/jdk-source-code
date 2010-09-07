/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(__CJavaInstance__)
#define __CJavaInstance__

#include "IJavaInstance.h"
#include "IJavaInstanceCB.h"
#include "IPluginVM.h"
#include "IPluginVMCB.h"
#include "CJavaService.h"

class CJavaInstance : public IJavaInstance, private IPluginVMCB {
public:
    // IEgo functions
    DECL_IEGO

    // IJavaInstance functions
    void start();
    void stop();
    void destroy();
    void window(int, int, int, int, int);
    void javascriptReply(const char *);
    void docbase(const char *);

    virtual ~CJavaInstance();

    friend class CJavaService;

private:
    IPluginVM * m_vm;
    CJavaInstance(IPluginVM *, int, const char**, const char**, IJavaInstanceCB *);
    JavaID m_jid;
    IJavaInstanceCB * m_jicb;
      
    // IPluginVMCB methods
    void showStatus(char *);
    void showDocument(char *, char *);
    int findProxy(char *, char **);
    void findCookie(char *, char **);
    int javascriptRequest(char *);
    void setCookie(char *, char *);
};
#endif

