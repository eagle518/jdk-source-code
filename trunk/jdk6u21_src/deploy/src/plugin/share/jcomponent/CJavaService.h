/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(__CJavaService__)
#define __CJavaService__

#include "ICreater.h"
#include "IVersion.h"
#include "IConsole.h"
#include "IPluginVM.h"

// This function will return a component that implements
// at least the ICreater, and IVersion interfaces
extern "C" void createJavaService(const char *, void **);

class CJavaService : public ICreater , public IVersion, public IConsole {
public:
    // IEgo functions
    DECL_IEGO

    // ICreater functions
    JRESULT createJavaInstance(const char *,
                               int, const char**, const char**, 
                               IJavaInstanceCB *, IJavaInstance **);
    virtual void setFDMonitor(IFDMonitor *);

    // IVersion functions
    void supportedVersions(const char ***  );
    void containingVersions(const char *** );

    // IConsole functions
    void showConsole();

    friend void createJavaService(const char *, void **);

    ~CJavaService();
private:
    CJavaService(const char *);
    IPluginVM *m_vm;
    IFDMonitor * m_FDMonitor;
    const char * m_userAgent;
    static CJavaService * m_serviceStore;
};
#endif

