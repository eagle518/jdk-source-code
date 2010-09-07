/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "CJavaService.h"
#include "CPluginVM_OP.h"
#include "CJavaInstance.h"

#include <stdio.h>
#include <stdlib.h>

CJavaService * CJavaService::m_serviceStore = NULL;

void createJavaService(const char * userAgent, void ** ret) {

    if(CJavaService::m_serviceStore == NULL) {
        CJavaService::m_serviceStore = new CJavaService(userAgent);
    }
    *ret = (void *) CJavaService::m_serviceStore;
    reinterpret_cast<IEgo *>(*ret)->add();
}

JRESULT CJavaService::createJavaInstance(const char * version, 
                               int argc, const char** argn, 
                               const char** argv, IJavaInstanceCB * jicb, 
                               IJavaInstance ** ret) {

    JRESULT rv = J_NOERROR;    
    if (NULL == m_FDMonitor) {
        *ret = NULL;
        rv = J_UNEXPECTED;
    } else {
        if (NULL == m_vm) {
            CPluginVM_OP * vm = new CPluginVM_OP(m_FDMonitor, m_userAgent);
            m_vm = (IPluginVM *) vm;
            m_vm->add();
        }
    
        CJavaInstance * ji = new CJavaInstance(m_vm, argc, argn, argv, jicb);
    
        *ret = (IJavaInstance *) ji;
        (*ret)->add();
    }
    
    return rv;
}

void CJavaService::setFDMonitor(IFDMonitor * fdm) {
    m_FDMonitor = fdm;
    fprintf(stderr,"CJavaService::setFDMonitor\n");
}


void CJavaService::supportedVersions(const char *** sv) {

    static const char * temp[] = { 
                                  "1.4.2",
                                  "1.4.1",
                                  "1.4",
                                  "1.3.1",
                                  "1.3",
                                  "1.2.2",
                                  "1.2.1",
                                  "1.2",
                                  "1.1.3",
                                  "1.1.2",
                                  "1.1.1",
                                  "1.1",
                                     NULL };
    *sv = temp;
}

void CJavaService::containingVersions(const char *** cv) {

    static const char * temp[] = {
                                     "1.4.2",
                                        NULL };
    *cv = temp;
}

void CJavaService::showConsole() {
}

CJavaService::CJavaService(const char * ua) {
    m_vm = NULL;
    m_FDMonitor = NULL;
    m_userAgent = ua;
    m_refcount = 0;
}

CJavaService::~CJavaService() {
    if (m_vm != NULL) {
        m_vm->release();
    }
    m_serviceStore = NULL;
}

//  For ease of location, let these function always be last

JRESULT CJavaService::QI(const IID& iid, void ** ppv) {

    if (iid == IEgo_IID) { 
        *ppv = static_cast<ICreater *>(this);
    } else if (iid == ICreater_IID) {
        *ppv = static_cast<ICreater *>(this);
    } else if (iid == IVersion_IID) {
        *ppv = static_cast<IVersion *>(this);
    } else if (iid == IConsole_IID) {
        *ppv = static_cast<IConsole *>(this);
    } else {
        *ppv = NULL;
        return J_NOINTERFACE;
    }

    reinterpret_cast<IEgo *>(*ppv)->add();
    return J_NOERROR;
}

unsigned long CJavaService::add() {

    return ++m_refcount;
}

unsigned long CJavaService::release() {

    if (--m_refcount == 0) {
        delete this;
        return 0;
    }
    return m_refcount;
}
