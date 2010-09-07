/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(__jpom__)
#define __jpom__

#define interface struct

#define DECL_IEGO \
    public: \
    JRESULT QI(const IID&, void **); \
    unsigned long add(); \
    unsigned long release(); \
    private: \
    unsigned long m_refcount; \
    public:


typedef int JRESULT;
typedef unsigned int IID;
typedef unsigned int CID;

#define J_NOERROR 0;
#define J_NOINTERFACE -1;
#define J_UNEXPECTED -2;


// Interface ID's

#define IEgo_IID		0x0001
#define IVersion_IID		0x0002
#define ICreater_IID		0x0003
#define IJavaInstance_IID	0x0004
#define IJavaInstanceCB_IID	0x0005
#define IPluginVM_IID		0x0006
#define IPluginVMCB_IID		0x0007
#define IConsole_IID		0x0008
#define IFDMonitor_IID		0x0009

#endif

